#include <iostream>
#include <memory>
#include <string>
#include <cstdlib>
#include <limits>
#include <grpcpp/grpcpp.h>
#include "seal/seal.h"

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

class FedSqlImpl final : public FedSqlService::Service {x
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
        const int base = 10000;

        for (int data_id=0; data_id<n; ++data_id) {
            for (int j=0; j<dim; ++j) {
                arr[j] = rand() % base;
            }
            VectorDataType vector_data(dim, data_id, arr);
            m_data_list.emplace_back(vector_data);
        }
    }

    Status GetEncryptDistance(ServerContext* context,
                        const QueryObject* request,
                        EncryptDistance* response) override {

        m_logger.SetStartTimer();

        // Obtain the query object
        const int dim = request->data_size();
        if (dim <= m_dim) {
            throw std::invalid_argument("Dimension of query object should be equal to the dimension of data object");
        }
        VectorDataType query_data(dim, 0);
        for (int i=0; i<dim; ++i) {
            query_data.data[i] = request->data(i);
        }

        // Obtain the public key
        std::string pk_str = request->pk();
        m_LoadPublicKey(pk_str);
        
        // Compute the local nearest neighbor
        m_local_nn = m_GetLocalNearestNeighbor(query_data);

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

    Status FinishOneQuery(ServerContext* context,
                        const Empty* request,
                        Empty* response) override {

        m_logger.LogOneQuery();

        return Status::OK;
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
        Encryptor encryptor(context, public_key);
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

        print_line(__LINE__);
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
    static const size_t m_poly_function_prime = 97;
    static const size_t m_poly_modulus_degree = 16384;
    static const size_t m_batching_size = 60;   
};
  
int main(int argc, char** argv) { 
    std::string ip_address("localhost");
    if (argc > 1) {
        ip_address = std::string(argv[1]);
    }
    ip_address += std::string(":");
    ip_address += std::to_string(PORT);
    std::cout << "Connect with server: IP " << ip_address << " at PORT " << PORT << std::endl; 
     
    DataHolder client(grpc::CreateChannel(ip_address, grpc::InsecureChannelCredentials()));
    
    // receive p and g
    client.GetParams();

    // perform AES encryption/decryption test
    std::string message("Talk is cheap, show me the code.");
    if (argc > 2) {
        message = std::string(argv[2]);
    }
    client.AesTest(message);


    return 0;
}
