#pragma once

#include <opencv2/core/core.hpp> 
#include <opencv2/objdetect.hpp>
#include <opencv2/core/types.hpp>


#include <stdio.h>
#include <iostream>
#include <thread>
#include <memory>
#include <vector>

#include "ThreadSafeQueue.h"
#include "Database.h"
#include "Camera.h"

class Blurring {
    private:
        static inline constexpr const char* BLURRING_DB_PATH = "dsad";
        static inline std::shared_ptr<Database> m_blurringDatabase;

        static inline float m_scaleFactor;
        static inline float m_minNeighbors;

        static inline int m_gaussianBlurKernelSize;
        static inline int m_sigmaX;

        static inline cv::CascadeClassifier m_faceDetector;

        std::thread m_processingThread;
        ThreadSafeQueue<cv::Mat> m_blurredImageQueue;
        std::shared_ptr<Camera> m_cameraInstance;

        cv::Mat faceBlurring(const cv::Mat& p_capturedImage);
    
    public:
        explicit Blurring(const std::shared_ptr<Camera>& p_cameraInstance);
        ~Blurring();

        cv::Mat retrieveProcessedImage();

        void startBlurringProcess();
};
