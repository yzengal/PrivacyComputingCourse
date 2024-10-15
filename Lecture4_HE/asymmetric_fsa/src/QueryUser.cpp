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

#include <grpc/grpc.h>
#include <grpcpp/grpcpp.h>

#include "seal/seal.h"

#include "utils/BenchLogger.hpp"
#include "utils/DataType.hpp"
#include "FedSql.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using google::protobuf::Empty;
using FedSql::FedSqlService;
using FedSql::QueryObject;
using FedSql::EncryptDistance;
using FedSql::QueryAnswer;

// related to Microsoft SEAL
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


// #define LOCAL_DEBUG


class DataHolderReceiver {
public:
    DataHolderReceiver(std::shared_ptr<grpc::Channel> channel, const int silo_id, const std::string& silo_ipaddr, const std::string& silo_name) 
        : m_stub_(FedSqlService::NewStub(channel)), m_silo_id(silo_id), m_silo_ipaddr(silo_ipaddr), m_silo_name(silo_name) {
        
        m_logger.Init();
    }

    void GetEncryptDistance(const QueryObject& query_object) {
        ClientContext context;

        Status status = m_stub_->GetEncryptDistance(&context, query_object, &m_encrypt_dist); 
        if (!status.ok()) {
            std::cerr << "RPC failed: " << status.error_message() << std::endl;
            std::string error_message;
            error_message = std::string("Get encrypt distance from data silo #(") + std::to_string(m_silo_id) + std::string(") failed");
            throw std::invalid_argument(error_message);
        }

        float grpc_comm = query_object.ByteSizeLong() + m_encrypt_dist.ByteSizeLong();
        m_logger.LogAddComm(grpc_comm);
    }

    void GetEncryptPerturbDistance(const QueryObject& query_object, const std::string& other_ip_addr) {
        ClientContext context;

        query_object.set_ipaddr(other_ip_addr);
        Status status = m_stub_->GetEncryptPerturbDistance(&context, query_object, &m_encrypt_dist); 
        if (!status.ok()) {
            std::cerr << "RPC failed: " << status.error_message() << std::endl;
            std::string error_message;
            error_message = std::string("Get encrypt distance from data silo #(") + std::to_string(m_silo_id) + std::string(") failed");
            throw std::invalid_argument(error_message);
        }

        float grpc_comm = query_object.ByteSizeLong() + m_encrypt_dist.ByteSizeLong();
        m_logger.LogAddComm(grpc_comm);
    }

    VectorDataType GetQueryAnswer() {
        ClientContext context;
        Empty request;
        QueryAnswer response;

        Status status = m_stub_->GetQueryAnswer(&context, request, &response); 
        if (!status.ok()) {
            std::cerr << "RPC failed: " << status.error_message() << std::endl;
            std::string error_message;
            error_message = std::string("Get query answer from data silo #(") + std::to_string(m_silo_id) + std::string(") failed");
            throw std::invalid_argument(error_message);
        }

        float grpc_comm = request.ByteSizeLong() + response.ByteSizeLong();
        m_logger.LogAddComm(grpc_comm);

        const int dim = response.data_size();
        VectorDataType ret(dim, response.vid());
        for (int i=0; i<dim; ++i) {
            ret.data[i] = response.data(i);
        }
        return ret;
    } 

    void FinishQueryProcessing() {
        ClientContext context;
        Empty request;
        Empty response;

        Status status = m_stub_->FinishQueryProcessing(&context, request, &response); 
        if (!status.ok()) {
            std::cerr << "RPC failed: " << status.error_message() << std::endl;
            std::string error_message;
            error_message = std::string("Finish query processing from data silo #(") + std::to_string(m_silo_id) + std::string(") failed");
            throw std::invalid_argument(error_message);
        }

        float grpc_comm = request.ByteSizeLong() + response.ByteSizeLong();
        m_logger.LogAddComm(grpc_comm);
    } 

    EncryptDistance GetEncryptDistance() const {
        return m_encrypt_dist;
    }

    double GetQueryComm() const {
        return m_logger.GetQueryComm();
    }

    void InitBenchLogger() {
        m_logger.Init();
    }

    static void ThreadGetEncryptDistance(DataHolderReceiver* silo_receiver, const QueryObject& query_object) {  
        silo_receiver->GetEncryptDistance(query_object);
    }

    static void ThreadGetEncryptPerturbDistance(DataHolderReceiver* silo_receiver, const QueryObject& query_object, const std::string& other_ip_addr) {  
        silo_receiver->GetEncryptPerturbDistance(query_object, other_ip_addr);
    }

    static void ThreadGetDecryptDistance(DataHolderReceiver* silo_receiver, const std::string& edist_str, const EncryptionParameters& params, const SecretKey& secret_key, VectorDimensionType& dist) {  
        SEALContext context(params);

        Decryptor decryptor(context, secret_key);
        BatchEncoder batch_encoder(context);

        std::stringstream edist_sstream(edist_str);

        Ciphertext dist_encrypted;
        dist_encrypted.load(context, edist_sstream);

        Plaintext dist_decrypted;
        std::vector<int64_t> dist_matrix;
        decryptor.decrypt(dist_encrypted, dist_decrypted);
        batch_encoder.decode(dist_decrypted, dist_matrix);

        dist = dist_matrix[0];
    }    

    static void ThreadFinishQueryProcessing(DataHolderReceiver* silo_receiver) {
        silo_receiver->FinishQueryProcessing();
    }   

private:
    std::unique_ptr<FedSqlService::Stub> m_stub_;
    std::string m_silo_ipaddr;
    std::string m_silo_name;
    int m_silo_id;
    EncryptDistance m_encrypt_dist;
    BenchLogger m_logger;  
};

class FedSqlServer {
public:
    FedSqlServer(const std::string& silo_ip_filename, const std::string& user_name) : m_query_num(0), m_user_name(user_name) {

        m_ReadSiloIPaddr(silo_ip_filename, m_silo_ipaddr_list, m_silo_name_list);
        if (m_silo_ipaddr_list.empty()) {
            throw std::invalid_argument("There are no data holders' IP addresses and names");
        }

        std::cout << m_user_name << " is requesting asymmetric nearest neighbor query...\n";

        m_CreateSiloReceiver();
        m_InitSealParams();
        m_logger.Init();
    }

    void PsaAnnq(const int dim) {
        // Step 0: Initialize local variables
        m_InitBenchLogger();
        m_logger.SetStartTimer();

        // Step 1: Generator query object
        std::vector<VectorDimensionType> arr(dim);
        const int base = 10000;
        std::random_device rd;  // 用于获取随机数种子  
        std::default_random_engine eng(rd());  // 使用随机种子初始化引擎  
        // 创建均匀分布的整数随机数生成器，范围在 [1, 100]  
        std::uniform_int_distribution<> distribution(1, base); 
        for (int j=0; j<dim; ++j) {
            arr[j] = distribution(eng);
        }
        VectorDataType query_data(dim, m_query_num++, arr);
        std::cout << std::endl;
        std::cout << "Query object " << query_data.to_string() << std::endl;

        // Step 2: Get encrypt distance from all data holders
        m_GetEncryptDistance(query_data);

        // Step 3: Get decrypt distance from all data holders and determine the nearest one
        int nearest_silo_id = m_GetDecryptNearestDistance();

        // Step 4: Obtain query answer from specific data holder
        VectorDataType query_answer = m_GetQueryAnswer(nearest_silo_id);

        // Step 5: Finish query processing at each data holder
        m_FinishQueryProcessing();

        // Step 6: Print the log information
        m_logger.SetEndTimer();
        double query_comm = 0.0;
        for (int i=0; i<m_silo_num; ++i) {
            query_comm += m_silo_receiver_list[i]->GetQueryComm();
        }
        double query_time = m_logger.GetDurationTime();
        m_logger.LogOneQuery(query_comm);

        std::cout << std::fixed << std::setprecision(6) 
                    << "Query #(" << query_data.vid << "): runtime = " << query_time/1000.0 << " [s], communication = " << query_comm/1024.0 << " [KB]" << std::endl;
        std::cout << "Answer #(" << query_data.vid << "): data holder = " << m_silo_name_list[nearest_silo_id] << ", data = " << query_answer.to_string() << std::endl;
    }

    void FsaAnnq(const int dim=128) {
        // Step 0: Initialize local variables
        m_InitBenchLogger();
        m_logger.SetStartTimer();

        // Step 1: Generator query object
        std::vector<VectorDimensionType> arr(dim);
        const int base = 10000;
        std::random_device rd;  // 用于获取随机数种子  
        std::default_random_engine eng(rd());  // 使用随机种子初始化引擎  
        // 创建均匀分布的整数随机数生成器，范围在 [1, 100]  
        std::uniform_int_distribution<> distribution(1, base); 
        for (int j=0; j<dim; ++j) {
            arr[j] = distribution(eng);
        }
        VectorDataType query_data(dim, m_query_num++, arr);
        std::cout << std::endl;
        std::cout << "Query object " << query_data.to_string() << std::endl;

        // Step 2: Get perturb encrypt distance from all data holders
        m_GetEncryptPerturbDistance(query_data);

        // Step 3: Get decrypt distance from all data holders and determine the nearest one
        int nearest_silo_id = m_GetDecryptPerturbNearestDistance();

        // Step 4: Obtain query answer from specific data holder
        VectorDataType query_answer = m_GetQueryAnswer(nearest_silo_id);

        // Step 5: Finish query processing at each data holder
        m_FinishQueryProcessing();

        // Step 6: Print the log information
        m_logger.SetEndTimer();
        double query_comm = 0.0;
        for (int i=0; i<m_silo_num; ++i) {
            query_comm += m_silo_receiver_list[i]->GetQueryComm();
        }
        double query_time = m_logger.GetDurationTime();
        m_logger.LogOneQuery(query_comm);

        std::cout << std::fixed << std::setprecision(6) 
                    << "Query #(" << query_data.vid << "): runtime = " << query_time/1000.0 << " [s], communication = " << query_comm/1024.0 << " [KB]" << std::endl;
        std::cout << "Answer #(" << query_data.vid << "): data holder = " << m_silo_name_list[nearest_silo_id] << ", data = " << query_answer.to_string() << std::endl;
    }

    std::string to_string() const {
        std::stringstream ss;

        ss << "\n";
        ss << "-------------- Service Log --------------\n";
        ss << m_logger.to_string();

        return ss.str();
    }

private:
    void m_GetEncryptPerturbDistance(const VectorDataType& query_data) {
        QueryObject query_object;

        std::stringstream m_public_key_sstream;
        m_public_key.save(m_public_key_sstream);
        query_object.set_pk(m_public_key_sstream.str());
        const int dim = query_data.Dimension();
        for (int i=0; i<dim; ++i) {
            query_object.add_data(query_data.data[i]);
        }
        #ifdef LOCAL_DEBUG
        std::stringstream m_secret_key_sstream;
        m_secret_key.save(m_secret_key_sstream);
        query_object.set_sk(m_secret_key_sstream.str());
        #endif

        const int silo_num = m_silo_ipaddr_list.size();
        std::vector<std::thread> thread_list(silo_num);

        for (int i=0; i<silo_num; ++i) {
            std::string other_silo_ipaddr = m_silo_ipaddr_list[i^1];
            thread_list[i] = std::thread(DataHolderReceiver::ThreadGetEncryptPerturbDistance, m_silo_receiver_list[i].get(), query_object, other_silo_ipaddr);
        }
        for (int i=0; i<silo_num; ++i) {
            thread_list[i].join();
        }
    }

    void m_GetEncryptDistance(const VectorDataType& query_data) {
        QueryObject query_object;

        std::stringstream m_public_key_sstream;
        m_public_key.save(m_public_key_sstream);
        query_object.set_pk(m_public_key_sstream.str());
        const int dim = query_data.Dimension();
        for (int i=0; i<dim; ++i) {
            query_object.add_data(query_data.data[i]);
        }
        #ifdef LOCAL_DEBUG
        std::stringstream m_secret_key_sstream;
        m_secret_key.save(m_secret_key_sstream);
        query_object.set_sk(m_secret_key_sstream.str());
        #endif

        const int silo_num = m_silo_ipaddr_list.size();
        std::vector<std::thread> thread_list(silo_num);

        for (int i=0; i<silo_num; ++i) {
            thread_list[i] = std::thread(DataHolderReceiver::ThreadGetEncryptDistance, m_silo_receiver_list[i].get(), query_object);
        }
        for (int i=0; i<silo_num; ++i) {
            thread_list[i].join();
        }
    }

    int m_GetDecryptNearestDistance() {
        const int silo_num = m_silo_ipaddr_list.size();
        std::vector<std::thread> thread_list(silo_num);
        std::vector<VectorDimensionType> dist_list(silo_num);

        for (int i=0; i<silo_num; ++i) {
            EncryptDistance encrypt_dist = m_silo_receiver_list[i]->GetEncryptDistance();
            std::string edist_str = encrypt_dist.edist();
            thread_list[i] = std::thread(DataHolderReceiver::ThreadGetDecryptDistance, m_silo_receiver_list[i].get(), edist_str, m_parms, m_secret_key, std::ref(dist_list[i]));
        }
        for (int i=0; i<silo_num; ++i) {
            thread_list[i].join();
        }

        int nearest_silo_id = 0;
        for (int i=0; i<silo_num; ++i) {
            if (dist_list[i] < dist_list[nearest_silo_id]) {
                nearest_silo_id = i;
            }
            #ifdef LOCAL_DEBUG
            std::cout << "Data holder #(" << i << ") " << m_silo_name_list[i] << ": " << dist_list[i] << std::endl;
            #endif
        }
        return nearest_silo_id;
    }

    VectorDataType m_GetQueryAnswer(const int nearest_silo_id) {
        return m_silo_receiver_list[nearest_silo_id]->GetQueryAnswer();
    }

    void m_FinishQueryProcessing() {
        const int silo_num = m_silo_ipaddr_list.size();
        std::vector<std::thread> thread_list(silo_num);

        for (int i=0; i<silo_num; ++i) {
            thread_list[i] = std::thread(DataHolderReceiver::ThreadFinishQueryProcessing, m_silo_receiver_list[i].get());
        }
        for (int i=0; i<silo_num; ++i) {
            thread_list[i].join();
        }
    }

    void m_CreateSiloReceiver() {
        m_silo_num = m_silo_ipaddr_list.size();
        m_silo_receiver_list.resize(m_silo_num);
        grpc::ChannelArguments args;  
        args.SetInt(GRPC_ARG_MAX_SEND_MESSAGE_LENGTH, INT_MAX);  
        args.SetInt(GRPC_ARG_MAX_RECEIVE_MESSAGE_LENGTH, INT_MAX); 
        for (int silo_id=0; silo_id<m_silo_num; ++silo_id) {
            std::string silo_ipaddr = m_silo_ipaddr_list[silo_id];
            std::string silo_name = m_silo_name_list[silo_id];
            std::cout << "Query user " << m_user_name << " is connecting Data Holder #(" << std::to_string(silo_id+1) << ") " << silo_name << " on IP address " << silo_ipaddr << std::endl;

            std::shared_ptr<grpc::Channel> channel = grpc::CreateCustomChannel(silo_ipaddr, grpc::InsecureChannelCredentials(), args);
            m_silo_receiver_list[silo_id] = std::make_shared<DataHolderReceiver>(channel, silo_id+1, silo_ipaddr, silo_name);
        }
    }

    void m_ReadSiloIPaddr(const std::string& file_name, std::vector<std::string>& silo_ipaddr_list, std::vector<std::string>& silo_name_list) {
        std::ifstream file(file_name);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for reading data holders' IP addresses: " << file_name << std::endl;
            std::exit(EXIT_FAILURE);
        }
    
        size_t n;  
        if (!(file >> n)) {
            std::cerr << "Failed to read n from file: " << file_name << std::endl;
            std::exit(EXIT_FAILURE);
        }

        silo_ipaddr_list.resize(n);
        silo_name_list.resize(n);
        for (size_t i=0; i<n; ++i) {
            if (!(file >> silo_ipaddr_list[i] >> silo_name_list[i])) {
                std::cerr << "Failed to read data holders' IP addresses and names from line " << (i + 2) << " in file: " << file_name << std::endl;  
                std::exit(EXIT_FAILURE);
            }
        }
        file.close();
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

        KeyGenerator keygen(context);
        m_secret_key = keygen.secret_key();
        keygen.create_public_key(m_public_key);
        keygen.create_relin_keys(m_relin_keys);        
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

    void m_InitBenchLogger() {
        for (int silo_id=0; silo_id<m_silo_num; ++silo_id) {
            m_silo_receiver_list[silo_id]->InitBenchLogger();
        }
    }

    std::vector<std::shared_ptr<DataHolderReceiver>> m_silo_receiver_list;
    std::vector<std::string> m_silo_ipaddr_list;
    std::vector<std::string> m_silo_name_list;
    std::string m_user_name;
    VidType m_query_num;
    BenchLogger m_logger;
    int m_silo_num;

    // related to the BGV scheme in Microsoft SEAL
    EncryptionParameters m_parms;
    PublicKey m_public_key;
    SecretKey m_secret_key;
    RelinKeys m_relin_keys;
    static const size_t m_poly_modulus_degree = 4096;
    static const size_t m_batching_size = 40;
};

std::unique_ptr<FedSqlServer> fed_sqlserver_ptr = nullptr;

void RunService(const int n, const int dim, const std::string& silo_ip_filename, const std::string& user_name) {
    fed_sqlserver_ptr = std::make_unique<FedSqlServer>(silo_ip_filename, user_name);

    for (int i=0; i<n; ++i) {
        fed_sqlserver_ptr->PsaAnnq(dim);
    }

    std::string log_info = fed_sqlserver_ptr->to_string();
    std::cout << log_info;
    std::cout.flush();
}

// Ensure the log file is output, when the program is terminated.
void SignalHandler(int signal) {
    if (fed_sqlserver_ptr != nullptr) {
        std::string log_info = fed_sqlserver_ptr->to_string();
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
    int n, dim;
    std::string silo_ip_filename;
    std::string user_name("Tom");

    try { 
        bpo::options_description option_description("Required options");
        option_description.add_options()
            ("help", "produce help message")
            ("ip-file", bpo::value<std::string>(), "Data holder's IP address")
            ("name", bpo::value<std::string>(), "Query user's name")
            ("n", bpo::value<int>(&n)->default_value(1), "Number of nearest neighbor query")
            ("dim", bpo::value<int>(&dim)->default_value(128), "Dimension of query obeject")
        ;

        bpo::variables_map variable_map;
        bpo::store(bpo::parse_command_line(argc, argv, option_description), variable_map);
        bpo::notify(variable_map);    

        if (variable_map.count("help")) {
            std::cout << option_description << std::endl;
            return 0;
        }

        bool options_all_set = true;

        if (variable_map.count("ip-file")) {
            silo_ip_filename = variable_map["ip-file"].as<std::string>();
            std::cout << "Data holder's IP address file name was set to " << silo_ip_filename << "\n";
        } else {
            std::cout << "Data holder's IP address file name was not set" << "\n";
            options_all_set = false;
        }

        if (variable_map.count("name")) {
            user_name = variable_map["name"].as<std::string>();
            std::cout << "Query user's name was set to " << user_name << "\n";
        } else {
            std::cout << "Query user's name was not set" << "\n";
            options_all_set = false;
        }

        if (false == options_all_set) {
            throw std::invalid_argument("Some options were not properly set");
            std::cout.flush();
            std::exit(EXIT_FAILURE);
        }

    } catch (std::exception& e) {  
        std::cerr << "Error: " << e.what() << "\n";  
        std::exit(EXIT_FAILURE);
    }

    ResetSignalHandler();
    RunService(n, dim, silo_ip_filename, user_name);

    return 0;
}

