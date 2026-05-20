#include "ServerTransmitter.h"

ServerTransmitter::ServerTransmitter(const std::shared_ptr<Blurring>& p_blurringInst) {
    try {
        if (m_clientSocket == -1) {
            s_databaseInst = Database::getInstance(DB_FILE_PATH);
            s_serverIpAddress = std::get<std::string>(s_databaseInst->getValueFromKey("server-transmitter/server-ip"));
            s_databaseInst->~Database();

            memset(&s_serverAddress, 0, sizeof(s_serverAddress));
            s_serverAddress.sin_family = AF_INET;
            s_serverAddress.sin_port = htons(s_serverIpAddress);
            s_serverAddress.sin_addr.s_addr = INADDR_ANY;
        }   

        m_clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
        m_dtlsInst = std::make_unique<DTLSService>(m_clientSocket, s_serverAddress);
        m_dtlsInst->handshake();
    } catch (std::exception& exceptionThrown) {
        ;
    }
}

bool ServerTransmitter::startTransmitFrame() {
    bool expected = false;
    if (!m_isWorking.compare_exchange_strong(expected, true)) {
        return false; 
    }

    m_transmittingThread = std::thread([this]() {
        std::vector<uint8_t> imgBuffer;
        FrameFormat frameHeader{
            .m_timeStampMs = 0, 
            .m_width = 1980,
            .m_height = 1200,
            .m_payloadSize = 0
        };

        while (m_isWorking.load()) {
            cv::Mat retrievedImg = m_blurringInst->retrieveProcessedImage();
            if (retrievedImg.empty()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }

            if (!cv::imencode(".jpg", retrievedImg, imgBuffer)) {
                continue; 
            }

            frameHeader.m_timeStampMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count();
            frameHeader.m_payloadSize = static_cast<int>(imgBuffer.size());

            int bytesSent = sendto(m_clientSocket, reinterpret_cast<const char*>(&frameHeader), sizeof(FrameFormat), 0, (struct sockaddr*)&s_serverAddress, sizeof(s_serverAddress));
            if (bytesSent <= 0) {
                m_isWorking.store(false);
                break;
            }

            bytesSent = sendto(m_clientSocket, reinterpret_cast<const char*>(imgBuffer.data()), imgBuffer.size(), 0, (struct sockaddr*)&s_serverAddress, sizeof(s_serverAddress));
            if (bytesSent <= 0) {
                m_isWorking.store(false);
                break;
            }
        }
    });

    return true;
}

bool ServerTransmitter::stopTransmitFrame() {
    bool expected = true;
    if (!m_isWorking.compare_exchange_strong(expected, false)) {
        return true; 
    }

    if (m_transmittingThread.joinable()) {
        m_transmittingThread.join();
        return true;
    } else {
        return false;
    }
}


ServerTransmitter::~ServerTransmitter() {
    close(m_clientSocket);
}