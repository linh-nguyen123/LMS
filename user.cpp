#include "user.h"

User::User(string username, string password, string role)
    : username(username), password(password), role(role)
{
}

bool User::verify(string pass) {
    return this->password == pass;
}
