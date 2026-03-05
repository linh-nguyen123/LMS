#ifndef AUTHSYSTEM_H
#define AUTHSYSTEM_H

#include <vector>
#include <string>
#include "user.h"

using namespace std;

class AuthSystem
{
private:
    vector<User*> users; // vector con trỏ
    string filename;

    void saveUsers(); // Hàm lưu user xuống file

public:
    AuthSystem(); // constructor không có tham số
    ~AuthSystem(); // destructor không có tham số
    bool changePassword(string username, string oldPass, string newPass); // hàm thay đổi password của user
    User* login(string username, string password); // hàm đăng nhập

    bool addUser(string username, string password, string role); // hàm lưu danh sách user xuống file

    void loadUsers(); // load user từ file txt lên chương trình
};

#endif
