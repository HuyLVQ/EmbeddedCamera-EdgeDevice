#include "Camera.h"

Camera::Camera(const std::string& p_cameraFD, struct m_Tag p_tag) {
    (void)p_tag;
    m_cameraFileDescriptor = open(p_cameraFD.c_str(), O_RDWR | O_NONBLOCK);

    if (m_cameraFileDescriptor == -1) {
        throw CameraOpenError();
    }

    m_cameraDatabase = Database::getInstance(CAMERA_DB_PATH);

    configCamera();
    mappingBuffer();

    m_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (ioctl(m_cameraFileDescriptor, VIDIOC_STREAMON, &m_type) < 0) {
        throw CameraConfigError("| Stream On");
    }
}

void Camera::configCamera() {
    memset(&m_cameraFormat, 0, sizeof(m_cameraFormat));
    m_cameraFormat.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    m_cameraFormat.fmt.pix.width = std::get<int>(m_cameraDatabase->getValueFromKey("camera/pictureWidth"));
    m_cameraFormat.fmt.pix.height = std::get<int>(m_cameraDatabase->getValueFromKey("camera/pictureHeight"));
    m_cameraFormat.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    m_cameraFormat.fmt.pix.field = V4L2_FIELD_NONE;    

    if (ioctl(m_cameraFileDescriptor, VIDIOC_S_FMT, &m_cameraFormat) < 0) {
        throw CameraConfigError("|   Camera Format");
    }

    memset(&m_requestBuffer, 0, sizeof(m_requestBuffer));
    m_bufferSize = std::get<int>(m_cameraDatabase->getValueFromKey("camera/bufferSize"));
    m_requestBuffer.count = m_bufferSize;
    m_requestBuffer.type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    m_requestBuffer.memory = V4L2_MEMORY_MMAP;
    m_userBuffer.resize(m_bufferSize);

    if (ioctl(m_cameraFileDescriptor, VIDIOC_REQBUFS, &m_requestBuffer) < 0) {
        throw CameraConfigError("|   Camera Buffer");
    }
}

void Camera::mappingBuffer() {
    struct v4l2_buffer buffer;
    for (int i = 0; i < m_bufferSize; i++) {
        memset(&buffer, 0, sizeof(buffer));
        buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buffer.memory = V4L2_MEMORY_MMAP;
        buffer.index = i;

        if (ioctl(m_cameraFileDescriptor, VIDIOC_QUERYBUF, &buffer) < 0) {
            throw CameraConfigError("| Query Buffer");
        }

        m_userBuffer[i].length = buffer.length;

        m_userBuffer[i].start = mmap(
            NULL,
            buffer.length,
            PROT_READ | PROT_WRITE,
            MAP_SHARED,
            m_cameraFileDescriptor,
            buffer.m.offset
        );

        if (m_userBuffer[i].start == MAP_FAILED) {
            throw CameraConfigError("| Map Buffer");
        }
    }

    for (int i = 0; i < m_bufferSize; i++) {
        memset(&buffer, 0, sizeof(buffer));

        buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buffer.memory = V4L2_MEMORY_MMAP;
        buffer.index = i;

        if (ioctl(m_cameraFileDescriptor, VIDIOC_QBUF, &buffer) < 0) {
            throw CameraConfigError("| Queue Buffer");
        }
    }
}

bool Camera::awaitFileDescriptorSet() {
    fd_set fileDescriptorSet;
    struct timeval timeout;

    FD_ZERO(&fileDescriptorSet);
    FD_SET(m_cameraFileDescriptor, &fileDescriptorSet);
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    int result = select(m_cameraFileDescriptor + 1, &fileDescriptorSet, NULL, NULL, &timeout);
    return result > 0 && FD_ISSET(m_cameraFileDescriptor, &fileDescriptorSet);
}

std::shared_ptr<Camera> Camera::getInstance(const std::string& p_cameraFD) {
    std::lock_guard<std::mutex> lockGuard(s_initMutex);
    if(s_cameraCompilation.find(p_cameraFD) == s_cameraCompilation.end()) {
        try {
            auto newAssigningInstance = std::make_shared<Camera>(p_cameraFD, m_Tag{});
            s_cameraCompilation[p_cameraFD] = newAssigningInstance;

            return newAssigningInstance;
        } catch (CameraOpenError& exception) {
            // spdlog::error(exception.what() + p_cameraFD);
            return nullptr;
        }
        
    } 

    return s_cameraCompilation[p_cameraFD];
}

void Camera::imageCapture() {
    m_imageCaptureThread = std::thread([this](){
        struct v4l2_buffer buffer;
        while(!this->m_stopCapture.load()) {
            if (!awaitFileDescriptorSet()) {
                continue;
            }

            memset(&buffer, 0, sizeof(buffer));
            buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buffer.memory = V4L2_MEMORY_MMAP;

            if (ioctl(this->m_cameraFileDescriptor, VIDIOC_DQBUF, &buffer) < 0) {
                continue;
            }

            if (buffer.index >= this->m_userBuffer.size()) {
                break;
            }

            cv::Mat yuyvImage(this->m_cameraFormat.fmt.pix.height, this->m_cameraFormat.fmt.pix.width,
                                CV_8UC2, this->m_userBuffer[buffer.index].start);
            cv::Mat capturedImage;
            cv::cvtColor(yuyvImage, capturedImage, cv::COLOR_YUV2BGR_YUYV);

            this->m_imageQueue.push(capturedImage);

            if (ioctl(this->m_cameraFileDescriptor, VIDIOC_QBUF, &buffer) < 0) {
                break;
            }
        }
    });
}

cv::Mat Camera::retrieveCapturedImage() {
    auto capturedImage = m_imageQueue.pop();
    if (!capturedImage.has_value()) {
        return {};
    }

    return capturedImage.value();
}

Camera::~Camera() {
    m_stopCapture.store(true);
    ioctl(m_cameraFileDescriptor, VIDIOC_STREAMOFF, &m_type);
    m_imageQueue.shutdown();

    if (m_imageCaptureThread.joinable()) {
        m_imageCaptureThread.join();
    }

    for (int i = 0; i < m_bufferSize; i++) {
        if (m_userBuffer[i].start != MAP_FAILED && m_userBuffer[i].start != nullptr) {
            munmap(m_userBuffer[i].start, m_userBuffer[i].length);
        }
    }

    close(m_cameraFileDescriptor);
}
