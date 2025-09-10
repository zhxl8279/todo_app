// PasswordHasher.cpp
#include "PasswordHasher.h"
#include <sodium.h>
#include <stdexcept>
#include <regex>
#include <iostream>

// 静态成员初始化
PasswordHasher& PasswordHasher::getInstance() {
    static PasswordHasher instance;
    return instance;
}

// 私有构造函数
PasswordHasher::PasswordHasher() : m_initialized(false) {}

// 初始化加密库
bool PasswordHasher::initialize() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_initialized) {
        return true;
    }

    if (sodium_init() < 0) {
        std::cerr << "Failed to initialize libsodium" << std::endl;
        return false;
    }

    m_initialized = true;
    std::cout << "Libsodium initialized successfully" << std::endl;
    return true;
}

// 哈希密码
std::string PasswordHasher::hashPassword(const std::string& password) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_initialized) {
        throw std::runtime_error("PasswordHasher not initialized");
    }

    if (password.empty()) {
        throw std::invalid_argument("Password cannot be empty");
    }

    if (password.length() > 1024) {
        throw std::invalid_argument("Password too long");
    }

    char hashed_password[crypto_pwhash_STRBYTES];
    
    if (crypto_pwhash_str(hashed_password,
                         password.c_str(),
                         password.length(),
                         crypto_pwhash_OPSLIMIT_INTERACTIVE,   // 更高的安全级别
                         crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
        throw std::runtime_error("Password hashing failed: out of memory");
    }

    return std::string(hashed_password);
}

// 验证密码
bool PasswordHasher::verifyPassword(const std::string& hashedPassword, const std::string& password) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_initialized) {
        throw std::runtime_error("PasswordHasher not initialized");
    }

    if (hashedPassword.empty() || password.empty()) {
        return false;
    }

    return crypto_pwhash_str_verify(hashedPassword.c_str(),
                                   password.c_str(),
                                   password.length()) == 0;
}

// 检查密码强度
bool PasswordHasher::isPasswordStrong(const std::string& password) {
    // 至少8个字符
    if (password.length() < 8) {
        return false;
    }

    // 包含大写字母、小写字母、数字和特殊字符
    bool hasUpper = false;
    bool hasLower = false;
    bool hasDigit = false;
    bool hasSpecial = false;

    for (char c : password) {
        if (std::isupper(c)) hasUpper = true;
        if (std::islower(c)) hasLower = true;
        if (std::isdigit(c)) hasDigit = true;
        if (std::ispunct(c)) hasSpecial = true;
    }

    // 至少包含三种不同类型的字符
    int typeCount = (hasUpper ? 1 : 0) + (hasLower ? 1 : 0) + 
                   (hasDigit ? 1 : 0) + (hasSpecial ? 1 : 0);
    
    return typeCount >= 3;
}
