#ifndef USER_H
#define USER_H

#include <string>
#include <iostream>

using namespace std;

class User
{
private:
    string username;
    string password;
    string role;

public:
    User(string username, string password, string role);

    string getUsername() const { return username; }

    string getPassword() const { return password; }

    string getRole() const { return role; }
    void setPassword(string newPass) { password = newPass; }
    bool verify(string pass);
};

#endif
