#ifndef BORROWRECORD_H
#define BORROWRECORD_H

#include <string>
#include <iostream>

using namespace std;

class BorrowRecord
{
private:
    string username;
    string ISBN;
    string borrowDate;
    string dueDate;

public:
    BorrowRecord(string username, string ISBN, string borrowDate, string dueDate);

    string getUsername() const { return username; }
    string getISBN() const { return ISBN; }
    string getBorrowDate() const { return borrowDate; }
    string getDueDate() const { return dueDate; }

    string toFileFormat();
};

#endif
