#pragma once

#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>

#include <optional>
#include <unordered_map>
#include <memory>
#include <string>
#include <cstring>
#include <vector>


#include "Database.h"
#include "Exception.h"

class Camera {
    private:
        static const std::string CAMERA_DB_PATH;
        static std::unordered_map<std::string, std::shared_ptr<Camera>> s_cameraCompilation;

        std::shared_ptr<Camera> m_cameraInstance;
        std::shared_ptr<Database> m_cameraDatabase;
        int m_cameraFileDescriptor;

        struct v4l2_format m_cameraFormat;
        struct v4l2_capability m_cameraCapability;
        

        struct m_Tag{};
        struct m_Buffer{
            void *start;
            size_t length;
        };

        struct v4l2_requestbuffers m_requestBuffer; 
        struct v4l2_buffer m_buffer;
        std::vector<struct m_Buffer> m_userBuffer;

        fd_set m_fileDescriptorSet;
    
    public:
        Camera(const std::string& p_cameraFD, struct m_Tag p_tag) {
            m_cameraFileDescriptor = open(p_cameraFD.c_str(), 'O_RDWR');

            if (m_cameraFileDescriptor == -1) {
                throw CameraOpenError();
            }

            m_cameraDatabase = Database::getInstance(CAMERA_DB_PATH);

            try {
                configCamera();
            } catch (CameraConfigError& exception) {

            }

            try {
                mappingBuffer();
            } catch () {

            }

            setUpFileDescriptorSet();
        }


        void configCamera() {
            memset(&m_cameraFormat, 0, sizeof(m_cameraFormat));
            m_cameraFormat.fmt.pix.width = std::get<int>(m_cameraDatabase->getValueFromKey("camera/pictureWidth"));
            m_cameraFormat.fmt.pix.height = std::get<int>(m_cameraDatabase->getValueFromKey("camera/pictureHeight"));
            m_cameraFormat.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
            m_cameraFormat.fmt.pix.field = V4L2_FIELD_NONE;    

            if (ioctl(m_cameraFileDescriptor, VIDIOC_S_FMT, &m_cameraFormat) < 0) {
                throw CameraConfigError("|   Camera Format");
            }

            memset(&m_requestBuffer, 0, sizeof(m_requestBuffer));
            m_requestBuffer.count = std::get<int>(m_cameraDatabase->getValueFromKey("camera/bufferSize"));
            m_requestBuffer.type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            m_requestBuffer.memory = V4L2_MEMORY_MMAP;

            if (ioctl(m_cameraFileDescriptor, VIDIOC_REQBUFS, &m_requestBuffer) < 0) {
                throw CameraConfigError("|   Camera Buffer");
            }
        }

        void mappingBuffer() {
            for (int i = 0; i < std::get<int>(m_cameraDatabase->getValueFromKey("camera/bufferSize")); i++) {
                memset(&m_buffer, 0, sizeof(m_buffer));
                m_buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                m_buffer.memory = V4L2_MEMORY_MMAP;
                m_buffer.index = i;

                if (ioctl(m_cameraFileDescriptor, VIDIOC_QUERYBUF, &m_buffer) < 0) {
                    // perror("VIDIOC_QUERYBUF");
                    // return 1;
                }

                m_userBuffer[i].length = m_buffer.length;

                m_userBuffer[i].start = mmap(
                    NULL,
                    m_buffer.length,
                    PROT_READ | PROT_WRITE,
                    MAP_SHARED,
                    m_cameraFileDescriptor,
                    m_buffer.m.offset
                );

                if (m_userBuffer[i].start == MAP_FAILED) {
                    // perror("mmap");
                    // return 1;
                }
            }

        }


        void setUpFileDescriptorSet() {
            FD_ZERO(&m_fileDescriptorSet);
            FD_SET(m_cameraFileDescriptor, &m_fileDescriptorSet);
        }


        static std::shared_ptr<Camera> getInstance(const std::string& p_cameraFD) {
            if(s_cameraCompilation.find(p_cameraFD) == s_cameraCompilation.end()) {
                try {
                    auto newAssigningInstance = std::make_shared<Camera>(p_cameraFD, m_Tag{});
                    s_cameraCompilation[p_cameraFD] = newAssigningInstance;

                    return newAssigningInstance;
                } catch (CameraOpenError& exception) {
                    // spdlog::error(exception.what() + p_cameraFD);
                }
                
            } 

            return s_cameraCompilation[p_cameraFD];
        }


        void imageCapture() {
            int result = select(m_cameraFileDescriptor + 1, &m_fileDescriptorSet, NULL, NULL, NULL);

            enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

            if (ioctl(m_cameraFileDescriptor, VIDIOC_STREAMON, &type) < 0) {
                // perror("VIDIOC_STREAMON");
                // return 1;
            }

            memset(&m_buffer, 0, sizeof(m_buffer));
            m_buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            m_buffer.memory = V4L2_MEMORY_MMAP;

            if (ioctl(m_cameraFileDescriptor, VIDIOC_DQBUF, &m_buffer) < 0) {
                // perror("VIDIOC_DQBUF");
                // return 1;
            }

            printf("Captured frame: %d bytes\n", m_buffer.bytesused);

            // Save image
            FILE *file = fopen("frame.jpg", "wb");
            fwrite(m_userBuffer[m_buffer.index].start, m_buffer.bytesused, 1, file);

            ioctl(m_cameraFileDescriptor, VIDIOC_QBUF, &m_buffer);
        }

};
