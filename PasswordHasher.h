// PasswordHasher.h
#ifndef PASSWORD_HASHER_H
#define PASSWORD_HASHER_H

#include <string>
#include <mutex>

class PasswordHasher {
public:
    // 删除拷贝构造函数和赋值运算符
    PasswordHasher(const PasswordHasher&) = delete;
    PasswordHasher& operator=(const PasswordHasher&) = delete;

    // 获取单例实例
    static PasswordHasher& getInstance();

    // 初始化加密库（线程安全）
    bool initialize();

    // 哈希密码（线程安全）
    std::string hashPassword(const std::string& password);

    // 验证密码（线程安全）
    bool verifyPassword(const std::string& hashedPassword, const std::string& password);

    // 检查密码强度
    bool isPasswordStrong(const std::string& password);

private:
    // 私有构造函数
    PasswordHasher();
    ~PasswordHasher() = default;

    bool m_initialized;
    std::mutex m_mutex;
};

#endif // PASSWORD_HASHER_H
