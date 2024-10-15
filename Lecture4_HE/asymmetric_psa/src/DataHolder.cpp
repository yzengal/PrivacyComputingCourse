#include <algorithm>
#include <chrono>
#include <cmath>
#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <cctype>
#include <cstdlib>
#include <random>
#include <limits>
#include <utility>
#include <exception>
#include <signal.h>
#include <unistd.h>

#include <boost/program_options.hpp>
namespace bpo = boost::program_options;

#include <grpcpp/grpcpp.h>
#include "seal/seal.h"

#include "utils/BenchLogger.hpp"
#include "utils/DataType.hpp"
#include "FedSql.grpc.pb.h"


using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;
using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using google::protobuf::Empty;
using FedSql::FedSqlService;
using FedSql::QueryObject;
using FedSql::EncryptDistance;
using FedSql::QueryAnswer;


// #define LOCAL_DEBUG


class FedSqlImpl final : public FedSqlService::Service {
using PublicKey = seal::PublicKey;
using SecretKey = seal::SecretKey;
using RelinKeys = seal::RelinKeys;
using EncryptionParameters = seal::EncryptionParameters;
using SEALContext = seal::SEALContext;
using KeyGenerator = seal::KeyGenerator;
using Encryptor = seal::Encryptor;
using Evaluator = seal::Evaluator;
using Decryptor = seal::Decryptor;
using BatchEncoder = seal::BatchEncoder;
using scheme_type = seal::scheme_type;
using CoeffModulus = seal::CoeffModulus;
using PlainModulus = seal::PlainModulus;
using Plaintext = seal::Plaintext;
using Ciphertext = seal::Ciphertext;

public:
    explicit FedSqlImpl(const int silo_id, const std::string& silo_ipaddr, const std::string& silo_name)
                        : m_silo_id(silo_id), m_silo_ipaddr(silo_ipaddr), m_silo_name(silo_name) {

        m_logger.Init();
        m_InitSealParams();
    }

    void InitDataHolder(const int n, const int dim=128) {
        if (n <= 0) {
            throw std::invalid_argument("n must be a positive integer");
        }
        if (dim <= 1) {
            throw std::invalid_argument("dim must be larger than 1");
        }

        m_dim = dim;
        m_data_list.clear();
        std::vector<VectorDimensionType> arr(dim);
        const int base = 100;
        std::random_device rd;  // 用于获取随机数种子  
        std::default_random_engine eng(rd());  // 使用随机种子初始化引擎  
        // 创建均匀分布的整数随机数生成器，范围在 [1, 100]  
        std::uniform_int_distribution<> distribution(1, base);  

        for (int data_id=0; data_id<n; ++data_id) {
            for (int j=0; j<dim; ++j) {
                arr[j] = distribution(eng);
            }
            VectorDataType vector_data(dim, data_id, arr);
            m_data_list.emplace_back(vector_data);
            if (data_id < 10)
                std::cout << "Data " <<vector_data.to_string() << std::endl;
            else if (data_id == 10)
                std::cout << "Data ......" << std::endl;
        }

        m_local_nn.data.reserve(m_dim);
        m_local_nn.data.resize(m_dim);
    }

    Status GetEncryptDistance(ServerContext* context,
                                const QueryObject* request,
                                EncryptDistance* response) override {

        m_logger.SetStartTimer();

        // Obtain the query object
        const int dim = request->data_size();
        if (dim != m_dim) {
            throw std::invalid_argument("Dimension of query object should be equal to the dimension of data object");
        }
        VectorDataType query_data(dim, 0);
        for (int i=0; i<dim; ++i) {
            query_data.data[i] = request->data(i);
        }

        // Obtain the public key
        std::string pk_str = request->pk();
        m_LoadPublicKey(pk_str);

        #ifdef LOCAL_DEBUG
        std::string sk_str = request->sk();
        m_LoadSecretKey(sk_str);
        #endif
        
        // Compute the local nearest neighbor
        m_local_nn = m_GetLocalNearestNeighbor(query_data);
        std::cout << "Local NN: " << m_local_nn.to_string() << std::endl;

        // Compute the encrypt distance
        EncryptDistance encrypt_distance = m_GetEncryptDistance(m_local_nn, query_data);
        response->set_edist(encrypt_distance.edist());

        double grpc_comm = request->ByteSizeLong() + response->ByteSizeLong();
        m_logger.LogAddComm(grpc_comm);

        m_logger.SetEndTimer();
        m_logger.LogAddTime();

        return Status::OK;
    }

    Status GetQueryAnswer(ServerContext* context,
                            const Empty* request,
                            QueryAnswer* response) override {

        m_logger.SetStartTimer();

        response->set_vid(m_local_nn.vid);
        for (auto d : m_local_nn.data) {
            response->add_data(d);
        }
        double grpc_comm = request->ByteSizeLong() + response->ByteSizeLong();
        m_logger.LogAddComm(grpc_comm);

        m_logger.SetEndTimer();
        m_logger.LogAddTime();

        return Status::OK;
    }

    Status FinishQueryProcessing(ServerContext* context,
                            const Empty* request,
                            Empty* response) override {

        m_logger.LogOneQuery();

        return Status::OK;
    }

    std::string to_string() const {
        std::stringstream ss;

        ss << "-------------- Data Holder #(" << m_silo_id << ") " << m_silo_name << " Log --------------\n";
        ss << m_logger.to_string();

        return ss.str();
    }

private:
    VectorDataType m_GetLocalNearestNeighbor(const VectorDataType& query_data) {
        VectorDimensionType min_dist = std::numeric_limits<VectorDimensionType>::max();
        int min_id = -1;
        const int n = m_data_list.size();

        for (int i=0; i<n; ++i) {
            const VectorDataType& vector_data = m_data_list[i];
            VectorDimensionType dist = EuclideanSquareDistance(vector_data, query_data);
            if (dist < min_dist) {
                min_dist = dist;
                min_id = i;
            }
        }
        if (min_id < 0) {
            throw std::invalid_argument("database hasn't been initialized");
        }

        return m_data_list[min_id];
    }

    EncryptDistance m_GetEncryptDistance(const VectorDataType& vector_data, const VectorDataType& query_data) {
        VectorDimensionType dist = EuclideanSquareDistance(vector_data, query_data);
        EncryptDistance encrypt_dist;

        SEALContext context(m_parms);
        Encryptor encryptor(context, m_public_key);
        BatchEncoder batch_encoder(context);

        size_t slot_count = batch_encoder.slot_count();
        std::vector<int64_t> dist_matrix(slot_count, 0);
        dist_matrix[0] = dist;
        Plaintext dist_plain;
        batch_encoder.encode(dist_matrix, dist_plain);

        Ciphertext dist_encrypted;
        encryptor.encrypt(dist_plain, dist_encrypted);

        std::stringstream dist_encrypted_sstream;
        dist_encrypted.save(dist_encrypted_sstream);

        encrypt_dist.set_edist(dist_encrypted_sstream.str());

        #ifdef LOCAL_DEBUG
        Decryptor decryptor(context, m_secret_key);

        Plaintext dist_decrypted; 
        decryptor.decrypt(dist_encrypted, dist_decrypted);

        std::vector<int64_t> dist_matrix_tmp;
        batch_encoder.decode(dist_decrypted, dist_matrix_tmp);
        size_t row_size = slot_count / 2;
        PrintMatrix(dist_matrix_tmp, row_size);
        // std::cout << "Plaintext matrix row size: " << row_size << std::endl;
        std::cout << "encrypted distance is " << dist_matrix_tmp[0] << std::endl;
        #endif

        return encrypt_dist;
    }

    void m_LoadPublicKey(const std::string& pk_str) {
        // if pk_str is empty, 
        // we don't have to re-load the public key
        if (pk_str.empty()) return ;

        SEALContext context(m_parms);
        std::stringstream bytes_stream(pk_str);
        PublicKey public_key;
        public_key.load(context, bytes_stream);
        m_public_key = public_key;
    }

    void m_LoadSecretKey(const std::string& sk_str) {
        // if pk_str is empty, 
        // we don't have to re-load the public key
        if (sk_str.empty()) return ;

        SEALContext context(m_parms);
        std::stringstream bytes_stream(sk_str);
        SecretKey secret_key;
        secret_key.load(context, bytes_stream);
        m_secret_key = secret_key;
    }

    void m_InitSealParams() {
        /*
        Note that scheme_type is now "bgv".
        */
        m_parms = EncryptionParameters(scheme_type::bgv);
        m_parms.set_poly_modulus_degree(m_poly_modulus_degree);

        /*
        We can certainly use BFVDefault coeff_modulus. In later parts of this example,
        we will demonstrate how to choose coeff_modulus that is more useful in BGV.
        */
        m_parms.set_coeff_modulus(CoeffModulus::BFVDefault(m_poly_modulus_degree));
        m_parms.set_plain_modulus(PlainModulus::Batching(m_poly_modulus_degree, m_batching_size));

        SEALContext context(m_parms);
        PrintLine(__LINE__);
        std::cout << "Set encryption parameters and print" << std::endl;
        m_print_parameters(context);

        /*
        When parameters are used to create SEALContext, Microsoft SEAL will first
        validate those parameters. The parameters chosen here are valid.
        */
        std::cout << "Parameter validation (success): " << context.parameter_error_message() << std::endl;
    }

    /*
    Helper function: Prints the parameters in a SEALContext.
    */
    void m_print_parameters(const seal::SEALContext &context) {
        auto &context_data = *context.key_context_data();

        /*
        Which scheme are we using?
        */
        std::string scheme_name;
        switch (context_data.parms().scheme())
        {
        case seal::scheme_type::bfv:
            scheme_name = "BFV";
            break;
        case seal::scheme_type::ckks:
            scheme_name = "CKKS";
            break;
        case seal::scheme_type::bgv:
            scheme_name = "BGV";
            break;
        default:
            throw std::invalid_argument("unsupported scheme");
        }
        std::cout << "/" << std::endl;
        std::cout << "| Encryption parameters :" << std::endl;
        std::cout << "|   scheme: " << scheme_name << std::endl;
        std::cout << "|   poly_modulus_degree: " << context_data.parms().poly_modulus_degree() << std::endl;

        /*
        Print the size of the true (product) coefficient modulus.
        */
        std::cout << "|   coeff_modulus size: ";
        std::cout << context_data.total_coeff_modulus_bit_count() << " (";
        auto coeff_modulus = context_data.parms().coeff_modulus();
        std::size_t coeff_modulus_size = coeff_modulus.size();
        for (std::size_t i = 0; i < coeff_modulus_size - 1; i++)
        {
            std::cout << coeff_modulus[i].bit_count() << " + ";
        }
        std::cout << coeff_modulus.back().bit_count();
        std::cout << ") bits" << std::endl;

        /*
        For the BFV scheme print the plain_modulus parameter.
        */
        if (context_data.parms().scheme() == seal::scheme_type::bfv)
        {
            std::cout << "|   plain_modulus: " << context_data.parms().plain_modulus().value() << std::endl;
        }

        std::cout << "\\" << std::endl;
        fflush(stdout);
    }

    int m_silo_id;
    std::string m_silo_ipaddr;
    std::string m_silo_name;
    int m_dim;
    VectorDataType m_local_nn;
    std::vector<VectorDataType> m_data_list;
    BenchLogger m_logger;

    // private members that are related to the BGV scheme
    EncryptionParameters m_parms;
    PublicKey m_public_key;
    SecretKey m_secret_key;
    static const size_t m_poly_modulus_degree = 8192;
    static const size_t m_batching_size = 40;   
};
  
std::unique_ptr<FedSqlImpl> fed_db_ptr = nullptr;

void RunSilo(const int n, const int dim, const int silo_id, const std::string& silo_ipaddr, const std::string& silo_name) {
    fed_db_ptr = std::make_unique<FedSqlImpl>(silo_id, silo_ipaddr, silo_name);
    fed_db_ptr->InitDataHolder(n, dim);

    ServerBuilder builder;
    builder.AddListeningPort(silo_ipaddr, grpc::InsecureServerCredentials());
    builder.RegisterService(fed_db_ptr.get());
    builder.SetMaxSendMessageSize(INT_MAX);
    builder.SetMaxReceiveMessageSize(INT_MAX);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Data Holder #(" << silo_id << ") " << silo_name << " is listening on " << silo_ipaddr << std::endl;
    
    server->Wait();

    std::string log_info = fed_db_ptr->to_string();
    std::cout << log_info;
    std::cout.flush();
}

// Ensure the log file is output, when the program is terminated.
void SignalHandler(int signal) {
    if (fed_db_ptr != nullptr) {
        std::string log_info = fed_db_ptr->to_string();
        std::cout << log_info;
        std::cout.flush();
    }
    quick_exit(0);
}

void ResetSignalHandler() {
    signal(SIGINT, SignalHandler);
    signal(SIGQUIT, SignalHandler);
    signal(SIGTERM, SignalHandler);
    signal(SIGKILL, SignalHandler);
}

int main(int argc, char** argv) {
    // Expect the following args: --ip=0.0.0.0 --port=50051 --name=Alice --id=1 --n=500 --dim=128
    int n, dim;
    int silo_port, silo_id;
    std::string silo_ip, silo_ipaddr, silo_name;
    
    try { 
        bpo::options_description option_description("Required options");
        option_description.add_options()
            ("help", "produce help message")
            ("id", bpo::value<int>(&silo_id)->default_value(0), "Data holder's identifier")
            ("ip", bpo::value<std::string>(), "Data holder's IP address")
            ("port", bpo::value<int>(&silo_port), "Data holder's IP port")
            ("name", bpo::value<std::string>(), "Data holder's name")
            ("n", bpo::value<int>(&n)->default_value(500), "Data holder's data size")
            ("dim", bpo::value<int>(&dim)->default_value(128), "Data holder's dimension size")
        ;

        bpo::variables_map variable_map;
        bpo::store(bpo::parse_command_line(argc, argv, option_description), variable_map);
        bpo::notify(variable_map);    

        if (variable_map.count("help")) {
            std::cout << option_description << std::endl;
            return 0;
        }

        bool options_all_set = true;

        std::cout << "Data holder's ID is " << silo_id << "\n";

        if (variable_map.count("ip")) {
            silo_ip = variable_map["ip"].as<std::string>();
            std::cout << "Data holder's IP address was set to " << silo_ip << "\n";
        } else {
            std::cout << "Data holder's IP address was not set" << "\n";
            options_all_set = false;
        }

        if (variable_map.count("port")) {
            silo_port = variable_map["port"].as<int>();
            std::cout << "Data holder's IP port was set to " << silo_port << "\n";
        } else {
            std::cout << "Data holder's IP port was not set" << "\n";
            options_all_set = false;
        }

        if (variable_map.count("name")) {
            silo_name = variable_map["name"].as<std::string>();
            std::cout << "Data holder's name was set to " << silo_name << "\n";
        } else {
            std::cout << "Data holder's name was not set" << "\n";
            options_all_set = false;
        }

        if (false == options_all_set) {
            throw std::invalid_argument("Some options were not properly set");
            std::cout.flush();
            std::exit(EXIT_FAILURE);
        }

        silo_ipaddr = silo_ip + std::string(":") + std::to_string(silo_port);

    } catch (std::exception& e) {  
        std::cerr << "Error: " << e.what() << "\n";  
        std::exit(EXIT_FAILURE);
    }

    ResetSignalHandler();

    RunSilo(n, dim, silo_id, silo_ipaddr, silo_name);

    return 0;
}


