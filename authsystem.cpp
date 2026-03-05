#include "authsystem.h"
#include <fstream>
#include <sstream>
#include <iostream>

AuthSystem::AuthSystem() : filename("users.txt") {
    loadUsers(); // Tự động load khi khởi động

    // Nếu file chưa có gì hoặc không tồn tại, tạo tài khoản admin mặc định
    if (users.empty()) {
        addUser("admin", "admin", "admin");
        addUser("user", "123", "user");
    }
}
//cài đặt destructor
AuthSystem::~AuthSystem() {
    for (auto u : users) {
        delete u;
    }
    users.clear();
}

//cài đặt login dựa vào con trỏ
User* AuthSystem::login(string username, string password) {
    for (auto u : users) {
        if (u->getUsername() == username && u->verify(password)) {
            return new User(*u);
        }
    }
    return nullptr;
}

// Hàm thêm User
bool AuthSystem::addUser(string username, string password, string role) {
    // Kiểm tra trùng tên
    for (auto u : users) {
        if (u->getUsername() == username) {
            return false;
        }
    }

    // Thêm vào danh sách và lưu file
    users.push_back(new User(username, password, role));
    saveUsers();
    return true;
}

//Lưu danh sách xuống file
void AuthSystem::saveUsers() {
    ofstream fileOutput(filename);
    if (!fileOutput.is_open()) return;

    for (auto u : users) {
        fileOutput << u->getUsername() << "|" << u->getPassword() << "|" << u->getRole() << endl;
    }
    fileOutput.close();
}

//Đọc danh sách từ file
void AuthSystem::loadUsers() {
    ifstream fileInput(filename);
    if (!fileInput.is_open()) return;

    // Xóa danh sách cũ trong RAM để tránh trùng lặp
    for (auto u : users) delete u;
    users.clear();

    string line;
    while (getline(fileInput, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string user, pass, role;

        getline(ss, user, '|');
        getline(ss, pass, '|');
        getline(ss, role, '|');

        if (!user.empty() && !pass.empty()) {
            users.push_back(new User(user, pass, role));
        }
    }
    fileInput.close();
}

//hàm thay đổi password của user
bool AuthSystem::changePassword(string username, string oldPass, string newPass) {
    for (auto u : users) {
        if (u->getUsername() == username && u->verify(oldPass)) {
            u->setPassword(newPass);
            saveUsers();
            return true;
        }
    }
    return false;
}
