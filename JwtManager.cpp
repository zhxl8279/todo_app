#include "JwtManager.h"
#include <iostream>
#include <stdexcept>

JwtManager& JwtManager::getInstance() {
    static JwtManager instance;
    return instance;
}

JwtManager::JwtManager() {
    // 初始化代码（如果需要）
}

std::string JwtManager::generateToken(int userId, const std::string& username) {
    try {
        auto token = jwt::create()
            .set_issuer("todo-app")
            .set_type("JWT")
            .set_payload_claim("user_id", jwt::claim(std::to_string(userId)))
            .set_payload_claim("username", jwt::claim(username))
            .set_issued_at(std::chrono::system_clock::now())
            .set_expires_at(std::chrono::system_clock::now() + std::chrono::seconds(token_expiry_))
            .sign(jwt::algorithm::hs256{secret_key_});

        return token;
    } catch (const std::exception& e) {
        std::cerr << "Error generating token: " << e.what() << std::endl;
        return "";
    }
}

bool JwtManager::verifyToken(const std::string& token) {
    try {
        auto decoded = jwt::decode(token);
        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{secret_key_})
            .with_issuer("todo-app");

        verifier.verify(decoded);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Token verification failed: " << e.what() << std::endl;
        return false;
    }
}

int JwtManager::getUserIdFromToken(const std::string& token) {
    try {
        auto decoded = jwt::decode(token);
        auto claim = decoded.get_payload_claim("user_id");
        return std::stoi(claim.as_string());
    } catch (const std::exception& e) {
        std::cerr << "Error getting user ID from token: " << e.what() << std::endl;
        return -1;
    }
}

std::string JwtManager::getUsernameFromToken(const std::string& token) {
    try {
        auto decoded = jwt::decode(token);
        return decoded.get_payload_claim("username").as_string();
    } catch (const std::exception& e) {
        std::cerr << "Error getting username from token: " << e.what() << std::endl;
        return "";
    }
}