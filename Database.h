#include <iostream>
#include <string> // 明确包含 std::string
#include <mysqlx/xdevapi.h>
#include "PasswordHasher.h"

// 使用命名空间，但避免 using namespace std; 以免冲突
using mysqlx::Session;
using mysqlx::SessionSettings;
using mysqlx::SessionOption;
using mysqlx::Schema;
using mysqlx::Table;
using mysqlx::Row;
using mysqlx::SqlResult;

// 使用 std:: 前缀明确指定标准库
using std::cout;
using std::cerr;
using std::endl;
using std::cin;

class Task {
public:
    Task() : id(0), completed(false) {}
    ~Task() {}

//private:
    int id;
    std::string title;
    std::string text;
    bool completed;
    std::string datetime;
    std::string timestamp;
};

class User {
public:
    User() : id(-1) {}
    ~User() {}

//private:
    int id;
    std::string username;
    std::string email;
    std::string timestamp;
    std::string password_hash;
    std::vector<Task> task;
};

class Database {
public:
    Database();
    ~Database();
    bool connect();
    void disconnect();
    bool registerUser(const std::string &username, const std::string &email, const std::string &password);
    bool loginUser(const std::string &username, const std::string &password);
    bool addTask(int id, const Task &t);
    bool delTask(int taskId);
    bool updateTaskStatus(int taskId, bool completed);
    std::vector<Task> getUserTasks(int id);
    User getUser(const std::string &username);
    User getUser(int id);
    int getIdByName(const std::string &username);

private:
    std::unique_ptr<mysqlx::Session> session_; // 使用智能指针
    bool connected_;
};


