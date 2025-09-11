// httpserver.cpp
#include <iostream>
#include <string>
#include "httplib.h"
#include "nlohmann/json.hpp"
#include "PasswordHasher.h"
#include "Database.h"
#include "JwtManager.h"

#define PORT 8279

using json = nlohmann::json;

class Client {
public:
    Client() : id(-1), valid(false) {
        data.connect();
    }

    ~Client() {}

    int getIdByName(const std::string &username) {
        return data.getIdByName(username);
    }
    bool registerUser(const std::string &username, const std::string &email, const std::string &password) {
        bool res = data.registerUser(username, email, password);
        if (res == true) {
            id = getIdByName(username);
            valid = true;
            user = data.getUser(username);
        }
        return res;
    }

    bool loginUser(const std::string &username, const std::string &password) {
        std::cout << "loginUser: " << username << " :" << password << std::endl;
        bool res = data.loginUser(username, password);
        if (res == true) {
            id = getIdByName(username);
            valid = true;
        }
        return res;
    }

    bool addTask(int id, const Task &t) {
        std::cout << "addTask: " << id << std::endl;
        return data.addTask(id, t);
    }

    bool delTask(int taskId) {
        std::cout << "delTask: " << taskId << std::endl;
        return data.delTask(id);
    }

    bool updateTaskStatus(int taskId, bool completed) {
        std::cout << "updateTaskStatus: " << taskId << " :" << completed << std::endl;
        return data.updateTaskStatus(taskId, completed);
    }

    std::vector<Task> getUserTasks(int id) {
        if (valid == true) {
            return data.getUserTasks(id);
        }
        return std::vector<Task>();
    }

    User getUser(const std::string &username) {
        if (valid == true) {
            user = data.getUser(username);
            user.task = getUserTasks(user.id);
        }
        return user;
    }

    User getUser(int id) {
        if (valid == true) {
            user = data.getUser(id);
            user.task = getUserTasks(id);
        }
        return user;
    }
    User getUser() {
        return user;
    }

private:
    int id;
    bool valid;
    Database data;
    User user;
};

class TodoAppServer {
public:
    void start() {
        PasswordHasher::getInstance().initialize();
        // 用户注册接口
        server.Post("/api/register", [&](const httplib::Request& req, httplib::Response& res) {
            try {
                // 解析JSON请求体
                auto j = json::parse(req.body);
                std::string username = j["username"];
                std::string password = j["password"];
                std::string email = j["email"];
                std::cout << "新用户注册" << username << password << email << std::endl;

                // 保存到数据库（这里需要你实现）
                std::shared_ptr<Client> client = std::make_shared<Client>();
                if (client->registerUser(username, email, password)) {
                    int id = client->getIdByName(username);
                    std::cout << "新用户注册id: " << id << std::endl;
                    users[id] = client;
                    // 返回成功响应
                    res.set_content(R"({"status": "success", "message": "User registered"})", "application/json");
                } else {
                    res.set_content(R"({"status": "failed", "message": "User name or Email alread existed"})", "application/json");
                }

            } catch (const std::exception& e) {
                res.status = 400;
                res.set_content(R"({"status": "error", "message": "Registration failed"})", "application/json");
            }
        });

        // 用户登录接口
        server.Post("/api/login", [&](const httplib::Request& req, httplib::Response& res) {
            try {
                auto j = json::parse(req.body);
                std::string username = j["username"];
                std::string password = j["password"];

                std::shared_ptr<Client> client = std::make_shared<Client>();
                if (client->loginUser(username, password)) {
                    User user = client->getUser(username);
                    if (!users.count(user.id)) {
                        users[user.id] = client;
                    }
                    // 生成真实的 JWT token
                    std::string token = JwtManager::getInstance().generateToken(user.id, username);
                    std::cout << token << std::endl;

                    json response;
                    response["status"] = "success";
                    response["message"] = "Login successful";
                    response["token"] = token;
                    response["user"] = {
                        {"id", user.id},
                        {"username", user.username},
                        {"email", user.email}
                    };
                    res.set_content(response.dump(), "application/json");
                } else {
                    res.status = 401;
                    res.set_content(R"({"status": "error", "message": "Invalid credentials"})", "application/json");
                }

            } catch (const std::exception& e) {
                res.status = 400;
                res.set_content(R"({"status": "error", "message": "Login failed"})", "application/json");
            }
        });

        // 获取任务列表
        server.Get("/api/logout", [&](const httplib::Request& req, httplib::Response& res) {
            // 验证权限等
            res.set_content(R"({"status": "success", "message": "Logout"})", "application/json");
        });

        // 获取任务列表
        server.Get("/api/addtask", [&](const httplib::Request& req, httplib::Response& res) {
            try {
                // 从 Authorization 头获取 token
                auto auth_header = req.headers.find("Authorization");
                std::string token = auth_header->second.substr(7);
                std::cout << token << std::endl;

                int id = JwtManager::getInstance().getUserIdFromToken(token);
                std::cout << id << std::endl;
                if (users.count(id)) {
                    std::cout << "用户已登录" << std::endl;
                }
                User user = users[id]->getUser();

                json response = {
                    {"status", "success"},
                    {"user", {
                        {"id", user.id},
                        {"username", user.username},
                        {"email", user.email},
                        {"register_date", user.timestamp},
                        {"task_count", user.task.size()}
                    }},
                    {"tasks", json::array()}  // 任务列表将在下面填充
                };

                // 填充任务数据
                try{
                    for (const auto& task : user.task) {
                        json task_json = {
                            {"id", task.id},
                            {"title", task.title},
                            {"text", task.text},
                            {"completed", task.completed},
                            {"due_date", task.datetime},
                            {"created_at", task.timestamp}
                        };
                        response["tasks"].push_back(task_json);
                    }
                } catch (const std::exception& e) {
                    std::cout << e.what() << std::endl;
                }
                res.set_content(response.dump(), "application/json");
            } catch (const std::exception& e) {
                res.status = 400;
                res.set_content(R"({"status": "error", "message": "Login failed"})", "application/json");
            }
        });

        // API: GET /api/profile
        server.Get("/api/profile", [&](const httplib::Request& req, httplib::Response& res) {
            try {
                // 从 Authorization 头获取 token
                auto auth_header = req.headers.find("Authorization");
                std::string token = auth_header->second.substr(7);
                std::cout << token << std::endl;

                int id = JwtManager::getInstance().getUserIdFromToken(token);
                std::cout << id << std::endl;
                if (users.count(id)) {
                    std::cout << "用户已登录" << std::endl;
                }
                User user = users[id]->getUser();

                json response = {
                    {"status", "success"},
                    {"user", {
                        {"id", user.id},
                        {"username", user.username},
                        {"email", user.email},
                        {"register_date", user.timestamp},
                        {"task_count", user.task.size()}
                    }},
                    {"tasks", json::array()}  // 任务列表将在下面填充
                };

                // 填充任务数据
                try{
                    for (const auto& task : user.task) {
                        json task_json = {
                            {"id", task.id},
                            {"title", task.title},
                            {"text", task.text},
                            {"completed", task.completed},
                            {"due_date", task.datetime},
                            {"created_at", task.timestamp}
                        };
                        response["tasks"].push_back(task_json);
                    }
                } catch (const std::exception& e) {
                    std::cout << e.what() << std::endl;
                }
                res.set_content(response.dump(), "application/json");
            } catch (const std::exception& e) {
                res.status = 400;
                res.set_content(R"({"status": "error", "message": "Login failed"})", "application/json");
            }
        });

        // 静态文件服务（前端页面）
        server.set_mount_point("/", "./www");

        std::cout << "Server running on http://localhost:" << PORT << std::endl;
        server.listen("0.0.0.0", PORT);
    }

    // 添加 server 的访问方法
    httplib::Server& getServer() { return server; }
    const httplib::Server& getServer() const { return server; }
private:
    httplib::Server server;
    std::map<int, std::shared_ptr<Client>> users;
};

int main() {
    TodoAppServer app;

    // 在服务器设置中添加中间件
    app.getServer().set_pre_routing_handler([&](const httplib::Request& req, httplib::Response& res) {
        // 设置 CORS 头
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");

        // 处理预检请求
        if (req.method == "OPTIONS") {
            res.status = 200;
            return httplib::Server::HandlerResponse::Handled;
        }

        // 调试信息
        std::cout << "Request: " << req.method << " " << req.path << std::endl;

        // 允许所有静态文件访问（不需要认证）
        if (req.path.find("/api/") == std::string::npos) {
            std::cout << "Allowing static file: " << req.path << std::endl;
            return httplib::Server::HandlerResponse::Unhandled;
        }

        // 允许特定的公开API（不需要认证）
        if (req.path == "/api/login" || req.path == "/api/register" || req.path == "/api/health") {
            std::cout << "Allowing public API: " << req.path << std::endl;
            return httplib::Server::HandlerResponse::Unhandled;
        }

        // 只有需要认证的API请求才检查token
        std::cout << "Requiring auth for API: " << req.path << std::endl;

        // 检查 Authorization 头
        auto auth_header = req.headers.find("Authorization");
        if (auth_header == req.headers.end()) {
            std::cout << "Missing Authorization header" << std::endl;
            res.status = 401;
            res.set_content(R"({"status": "error", "message": "Authorization header required"})", "application/json");
            return httplib::Server::HandlerResponse::Handled;
        }

        std::string token = auth_header->second.substr(7);
        if (!JwtManager::getInstance().verifyToken(token)) {
            std::cout << "Token verification failed" << std::endl;
            res.status = 401;
            res.set_content(R"({"status": "error", "message": "Invalid or expired token"})", "application/json");
            return httplib::Server::HandlerResponse::Handled;
        }

        std::cout << "Token verification successful" << std::endl;
        return httplib::Server::HandlerResponse::Unhandled;
    });

    app.start();
    return 0;
}
