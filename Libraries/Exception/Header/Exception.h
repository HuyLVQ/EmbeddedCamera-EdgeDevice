#pragma once

#include <exception>
#include <string>

class DatabaseOpenError : public std::exception {
    public:
        const char* what() const noexcept override {
            return "Database opening failed ";
        }
};

class DatabaseLoadupError : public std::exception {
    public:
        const char* what() const noexcept override {
            return "Database loading up failed ";
        }
};

class CameraOpenError : public std::exception {
    public:
        const char* what() const noexcept override {
            return "Camera opening failed ";
        }
};

class CameraConfigError : public std::exception {
private:
    std::string m_message;

public:
    explicit CameraConfigError(const std::string& p_message) 
        : m_message("Camera configure failed: " + p_message) {}

    const char* what() const noexcept override {
        return m_message.c_str();
    }
};