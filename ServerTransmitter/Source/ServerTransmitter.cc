#include "ServerTransmitter.h"

ServerTransmitter::ServerTransmitter(const std::shared_ptr<Blurring>& p_blurringInst) {
    try {
        if (m_clientSocket == -1) {
            auto databaseInst = Database::getInstance(DB_FILE_PATH);
            s_serverIpAddress = std::get<std::string>(databaseInst->getValueFromKey("server-transmitter/server-ip"));

            memset(&s_serverAddress, 0, sizeof(s_serverAddress));
            s_serverAddress.sin_family = AF_INET;
            s_serverAddress.sin_port = htons(s_serverIpAddress);
            s_serverAddress.sin_addr.s_addr = INADDR_ANY;

            m_blurringInst = p_blurringInst;
        }   

        m_clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
        m_dtlsInst = std::make_unique<DTLSService>(m_clientSocket, s_serverAddress);
        m_dtlsInst->handShake();

        startTransmitFrame();
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

            const char* reinterpretData = reinterpret_cast<const char*>(&frameHeader);
            try {
                int bytesSent = m_dtlsInst->sendPackage(reinterpretData, sizeof(FrameFormat));
            } catch (const std::exception& error) {
                std::cout << error.what();
                m_isWorking.store(false);
                break;
            }

            reinterpretData = reinterpret_cast<const char*>(imgBuffer.data());
            try {
                int bytesSent = m_dtlsInst->sendPackage(reinterpretData, imgBuffer.size());
            } catch (const std::exception& error) {
                std::cout << error.what();
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
    stopTransmitFrame();
    close(m_clientSocket);
}