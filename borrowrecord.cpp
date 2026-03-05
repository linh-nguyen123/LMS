#include "borrowrecord.h"

BorrowRecord::BorrowRecord(string username, string ISBN, string borrowDate, string dueDate)
    : username(username), ISBN(ISBN), borrowDate(borrowDate), dueDate(dueDate)
{
}

string BorrowRecord::toFileFormat() {
    return username + "|" + ISBN + "|" + borrowDate + "|" + dueDate;
}
