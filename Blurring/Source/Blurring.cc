#include "Blurring.h"

Blurring::Blurring(const std::shared_ptr<Camera>& p_cameraInstance) {
    if (m_blurringDatabase == nullptr) {
        m_blurringDatabase = Database::getInstance(BLURRING_DB_PATH);

        m_scaleFactor = std::get<float>(m_blurringDatabase->getValueFromKey("blurring/scaleFactor"));
        m_minNeighbors = std::get<float>(m_blurringDatabase->getValueFromKey("blurring/minNeighbors"));

        m_gaussianBlurKernelSize = std::get<float>(m_blurringDatabase->getValueFromKey("blurring/gaussianBlurKernelSize"));
        m_sigmaX = std::get<float>(m_blurringDatabase->getValueFromKey("blurring/sigmaX"));

        m_faceDetector.load(std::get<std::string>(m_blurringDatabase->getValueFromKey("blurring/faceDetectionXMLPath")));
    }

    m_cameraInstance = p_cameraInstance;
}

cv::Mat Blurring::faceBlurring(const cv::Mat& p_capturedImage) {
    cv::Mat processedImage = p_capturedImage.clone();   

    std::vector<cv::Rect> faceData;
    m_faceDetector.detectMultiScale(processedImage, faceData, m_scaleFactor, static_cast<int>(m_minNeighbors));

    for(const auto& faceDataMember:faceData) {
        cv::Mat roi = processedImage(faceDataMember);
        cv::GaussianBlur(roi, roi, cv::Size(m_gaussianBlurKernelSize, m_gaussianBlurKernelSize), m_sigmaX);
    }

    return processedImage;
}  

cv::Mat Blurring::retrieveProcessedImage() {
    auto blurredImage = m_blurredImageQueue.pop();
    if (!blurredImage.has_value()) {
        return {};
    }

    return blurredImage.value();
}

void Blurring::startBlurringProcess() {
    m_processingThread = std::thread([this]() {
        while(true) {
            m_blurredImageQueue.push(this->faceBlurring(m_cameraInstance->retrieveCapturedImage()));
        }
    });
} 