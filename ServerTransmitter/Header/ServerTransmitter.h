#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <cstring>
#include <chrono>
#include <memory>
#include <atomic>
#include <unordered_map>
#include <vector>
#include <thread>

#include "TransmitterIf.h"
#include "DTLSService.h"
#include "Blurring.h"
#include "Database.h"
#include "Frame.h"

class ServerTransmitter : public TransmitterIf {
    private:
        static constexpr const char* DB_FILE_PATH = "sdasd";
        static std::string s_serverIpAddress;
        static std::unordered_map<int, std::shared_ptr<ServerTransmitter>> s_socketCompilation;

        static sockaddr_in s_serverAddress;             
        int m_clientSocket = -1;

        std::unique_ptr<DTLSService> m_dtlsInst;                                                                                            
        std::shared_ptr<Blurring> m_blurringInst;
        static std::shared_ptr<Database> s_databaseInst;

        std::thread m_transmittingThread;
        std::atomic<bool> m_isWorking{false};


    public:
        explicit ServerTransmitter(const std::shared_ptr<Blurring>& p_blurringInst);
        ~ServerTransmitter();

        bool startTransmitFrame() override;
        bool stopTransmitFrame() override;
};