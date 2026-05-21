#pragma once

#include <iostream>
#include <vector>
#include <exception>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>

#include "SecurityServiceIf.h"

class DTLSService : public SecurityServiceIf {
    private:
        SSL_CTX* m_ctx = nullptr;
        SSL* m_ssl = nullptr;
        BIO* m_writeBIO = nullptr;

    public:
        DTLSService(int p_socket, const struct sockaddr_in& p_serverAddr);
        ~DTLSService();

        void handShake();
        int sendPackage(const char* p_encodedPackages, const int& p_encodedLen);
};
