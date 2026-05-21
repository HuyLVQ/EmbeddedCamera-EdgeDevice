#pragma once

#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <unistd.h>
#include <linux/videodev2.h>

#include <atomic>
#include <optional>
#include <unordered_map>
#include <memory>
#include <string>
#include <cstring>
#include <vector>
#include <mutex>
#include <thread>


#include "Database.h"
#include "Exception.h"
#include "ThreadSafeQueue.h"
#include <opencv2/opencv.hpp>


class Camera {
    private:
        static inline constexpr const char* CAMERA_DB_PATH = "dsad";
        static inline std::unordered_map<std::string, std::shared_ptr<Camera>> s_cameraCompilation;
        static inline std::mutex s_initMutex;

        std::shared_ptr<Database> m_cameraDatabase;
        int m_cameraFileDescriptor;

        struct v4l2_format m_cameraFormat;
        struct v4l2_capability m_cameraCapability;
        

        struct m_Tag{};
        struct m_Buffer{
            void *start;
            size_t length;
        };

        int m_bufferSize;
        struct v4l2_requestbuffers m_requestBuffer; 
        std::vector<struct m_Buffer> m_userBuffer;

        enum v4l2_buf_type m_type;

        std::thread m_imageCaptureThread;
        std::atomic_bool m_stopCapture{false};

        void configCamera();
        void mappingBuffer();
        bool awaitFileDescriptorSet();
        void imageCapture();
    
    public:
        ThreadSafeQueue<cv::Mat> m_imageQueue;

        Camera(const std::string& p_cameraFD, struct m_Tag p_tag);
        static std::shared_ptr<Camera> getInstance(const std::string& p_cameraFD);

        cv::Mat retrieveCapturedImage();
        ~Camera();
};
