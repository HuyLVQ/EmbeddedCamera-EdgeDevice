#include "DTLSService.h"

DTLSService::DTLSService(int p_socket, const struct sockaddr_in& p_serverAddr) {
    SSL_library_init();                                                 //< Initialize the library
    OPENSSL_init_ssl(OPENSSL_INIT_LOAD_SSL_STRINGS | OPENSSL_INIT_LOAD_CRYPTO_STRINGS, nullptr);
        
    m_ctx = SSL_CTX_new(DTLS_client_method());                          //< Create the client context
    if (!m_ctx) {
        throw std::runtime_error("Failed to create DTLS context");      //< Throw an error if could not initiate a Client context
    }

    SSL_CTX_set_min_proto_version(m_ctx, DTLS1_2_VERSION);              //< Configure the version of DTLS

    m_ssl = SSL_new(m_ctx);                                             //< Open new session
    
    m_writeBIO = BIO_new_dgram(p_socket, BIO_NOCLOSE);                  //< Setup a queue using DGRAM
    BIO_ctrl(m_writeBIO, BIO_CTRL_DGRAM_SET_CONNECTED, 0, (struct sockaddr*)&p_serverAddr);
    BIO_set_mem_eof_return(m_writeBIO, -1);             

    SSL_set_bio(m_ssl, nullptr, m_writeBIO);
    SSL_set_connect_state(m_ssl);                                       // Set to client mode
}

DTLSService::~DTLSService() {
    if (m_ssl) SSL_free(m_ssl);
    if (m_ctx) SSL_CTX_free(m_ctx);
}

void DTLSService::handShake() {
    int ret = 0;
    
    while ((ret = SSL_connect(m_ssl)) <= 0) {                           //< Wait for the session to be established
        int err = SSL_get_error(m_ssl, ret);
        
        if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {

            continue; 
        } else {
            throw std::runtime_error("DTLS Handshake failed");
        }
    }
}