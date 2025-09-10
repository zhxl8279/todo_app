#include <iostream>
#include <string> // 明确包含 std::string
#include <mysqlx/xdevapi.h>
#include "PasswordHasher.h"
#include "Database.h"

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

// 数据库配置
const std::string HOST = "localhost";
const int PORT = 33060;
const std::string USER = "root";
const std::string PASSWORD = "1234";
const std::string DATABASE = "todo_app";

Database::Database() : session_(nullptr), connected_(false) {

}

Database::~Database() {
    disconnect();
}

// 创建 Session（数据库连接）
bool Database::connect() {
    bool ret = false;
    try {
        SessionSettings settings(
            SessionOption::HOST, HOST,
            SessionOption::PORT, PORT,
            SessionOption::USER, USER,
            SessionOption::PWD, PASSWORD,
            SessionOption::DB, DATABASE
        );
        std::unique_ptr<Session> s(new Session(settings));
        session_.swap(s);
        connected_ =  true;
        cerr << "数据库连接成功: " << endl;
        ret = true;
    } catch (const mysqlx::Error &e) {
        cerr << "数据库连接错误: " << e.what() << endl;
        throw;
    }
    return ret;
}

void Database::disconnect() {
    if (session_) {
        session_->close();
        session_.reset();
    }
    connected_ = false;
}

// 用户注册
bool Database::registerUser(const std::string &username, const std::string &email, const std::string &password) {
    if (!connected_) {
        return false;
    }

    try {
        Schema db = session_->getSchema(DATABASE);
        Table users = db.getTable("users");

        // 检查用户名是否已存在
        auto result = users.select("id")
                       .where("username = :username OR email = :email")
                       .bind("username", username)
                       .bind("email", email)
                       .execute();

        if (result.count() > 0) {
            cout << "错误: 用户名或邮箱已存在!" << endl;
            return false;
        }

        // 检查密码
        PasswordHasher& hasher = PasswordHasher::getInstance();

        // 检查密码强度
        if (!hasher.isPasswordStrong(password)) {
            cout << "警告: 密码安全等级较低!" << endl;
        }

        // 哈希密码
        std::string hashedPassword = hasher.hashPassword(password);

        // 插入新用户
        users.insert("username", "email", "password_hash")
              .values(username, email, hashedPassword)
              .execute();

        cout << "用户注册成功!" << endl;
        return true;

    } catch (const mysqlx::Error &e) {
        cerr << "注册失败: " << e.what() << endl;
        return false;
    }
}

// 用户登录验证
bool Database::loginUser(const std::string &username, const std::string &password) {
    if (!connected_) {
        return false;
    }

    try {
        Schema db = session_->getSchema(DATABASE);
        Table users = db.getTable("users");

        // 查询用户信息
        auto result = users.select("id", "password_hash")
                       .where("username = :username")
                       .bind("username", username)
                       .execute();

        if (result.count() == 0) {
            cout << "错误: 用户不存在!" << endl;
            return false;
        }

        Row row = result.fetchOne();
        // 使用正确的 getter 方法
        std::string storedPassword = row[1].get<std::string>();

         // 检查密码
        PasswordHasher& hasher = PasswordHasher::getInstance();
        if (hasher.verifyPassword(storedPassword, password)) {
            int userId = row[0].get<int>();
            cout << "登录成功! 用户ID: " << userId << endl;
            return true;
        } else {
            cout << "错误: 密码不正确!" << endl;
            return false;
        }

    } catch (const mysqlx::Error &e) {
        cerr << "登录失败: " << e.what() << endl;
        return false;
    }
}

// 获取用户的任务列表
std::vector<Task> Database::getUserTasks(int id) {
    std::vector<Task> ret;
    if (!connected_) {
        return ret;
    }

    try {
        Schema db = session_->getSchema(DATABASE);

        // 使用 SQL 语句进行复杂查询
        SqlResult result = session_->sql(
            "SELECT t.* FROM tasks t "
            "JOIN users u ON t.user_id = u.id "
            "WHERE u.id = ? "
            "ORDER BY t.created_at DESC"
        ).bind(id).execute();

        cout << "\n=== " << id << "的任务列表 ===" << endl;

        for (Row row : result) {
            // 使用正确的 getter 方法
            Task task;
            task.id        = row[0].get<int>();
            task.title     = row[1].get<std::string>();
            task.text      = row[2].get<std::string>();
            task.completed = row[3].get<bool>();
            task.datetime  = row[4].get<std::string>();
            task.timestamp = row[5].get<std::string>();
            ret.push_back(task);
            cout << "ID: " << row[0].get<int>()
                 << ", 标题: " << row[1].get<std::string>()
                 << ", 状态: " << (row[3].get<bool>() ? "已完成" : "未完成")
                 << endl;
        }

    } catch (const mysqlx::Error &e) {
        cerr << "获取任务失败: " << e.what() << endl;
    }
    return ret;
}

User Database::getUser(const std::string &username) {
    User user;
    if (!connected_) {
        return user;
    }

    try {
        Schema db = session_->getSchema(DATABASE);

        // 使用 SQL 语句进行复杂查询
        SqlResult result = session_->sql(
            "SELECT * FROM users "
            "WHERE username = ? "
        ).bind(username).execute();

        cout << "\n=== " << username << "的任务列表 ===" << endl;

        for (Row row : result) {
            // 使用正确的 getter 方法
            user.id           = row[0].get<int>();
            user.username     = row[1].get<std::string>();
            user.email        = row[2].get<std::string>();
            user.password_hash= row[3].get<std::string>();
            //user.created_at   = row[3].get<std::string>();
            cout << "ID: " << row[0].get<int>()
                 << ", username: " << row[1].get<std::string>()
                 << ", email: " << (row[2].get<std::string>())
                 << endl;
        }

    } catch (const mysqlx::Error &e) {
        cerr << "获取任务失败: " << e.what() << endl;
    }
    return user;
}

User Database::getUser(int id) {
    User user;
    if (!connected_) {
        return user;
    }

    try {
        Schema db = session_->getSchema(DATABASE);

        // 使用 SQL 语句进行复杂查询
        SqlResult result = session_->sql(
            "SELECT * FROM users "
            "WHERE id = ? "
        ).bind(id).execute();

        cout << "\n=== " << id << "的任务列表 ===" << endl;

        for (Row row : result) {
            // 使用正确的 getter 方法
            user.id           = row[0].get<int>();
            user.username     = row[1].get<std::string>();
            user.email        = row[2].get<std::string>();
            user.password_hash= row[3].get<std::string>();
            //user.created_at   = row[3].get<std::string>();
            cout << "ID: " << row[0].get<int>()
                 << ", username: " << row[1].get<std::string>()
                 << ", email: " << (row[2].get<std::string>())
                 << endl;
        }

    } catch (const mysqlx::Error &e) {
        cerr << "获取任务失败: " << e.what() << endl;
    }
    return user;
}

int Database::getIdByName(const std::string &username) {
    if (!connected_) {
        return -1;
    }

    int id = -1;
    try {
        Schema db = session_->getSchema(DATABASE);
        Table users = db.getTable("users");

        // 查询用户信息
        auto result = users.select("id")
                       .where("username = :username")
                       .bind("username", username)
                       .execute();

        if (result.count() == 0) {
            cout << "错误: 用户不存在!" << endl;
            id =-1;
        } else {
            Row row = result.fetchOne();
            // 使用正确的 getter 方法
            id = row[0].get<int>();
        }
    } catch (const mysqlx::Error &e) {
        cerr << "登录失败: " << e.what() << endl;
    }
    return id;
}
