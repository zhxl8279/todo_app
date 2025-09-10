#ifndef JWT_MANAGER_H
#define JWT_MANAGER_H

#include <string>
#include <jwt-cpp/jwt.h>  // 下载的 JWT 头文件

class JwtManager {
public:
    static JwtManager& getInstance();

    // 生成 JWT token
    std::string generateToken(int userId, const std::string& username);

    // 验证 JWT token
    bool verifyToken(const std::string& token);

    // 从 token 中获取用户ID
    int getUserIdFromToken(const std::string& token);

    // 从 token 中获取用户名
    std::string getUsernameFromToken(const std::string& token);

private:
    JwtManager();
    ~JwtManager() = default;

    const std::string secret_key_ = "your-super-secret-key-change-in-production";
    const int token_expiry_ = 3600; // 1小时过期
};

#endif // JWT_MANAGER_H