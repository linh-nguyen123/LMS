#ifndef PENALTY_H
#define PENALTY_H

#include <string>
#include <iostream>

using namespace std;

class Penalty {
private:
    string username;
    string ISBN;
    string bookTitle;
    string dueDate; // hạn trả dự kiến của user
    string returnDate;// ngày trả thực tế
    int daysOverdue; // số ngày quá hạn
    float penaltyAmount; // tiền phạt
    bool isPaid; //kiểm tra xem user đã thanh toán tiền phạt hay chưa

public:
    Penalty(string username, string ISBN, string bookTitle, string dueDate, string returnDate, float penaltyAmount);

    string getUsername() const {return username;}
    string getISBN() const {return ISBN;}
    string getBookTitle() const {return bookTitle;}
    string getDueData() const {return dueDate;}
    string getReturnDate() const {return returnDate;}
    int getDaysOverDue() const {return daysOverdue;}
    float getPenaltyAmount() const {return penaltyAmount;}
    bool getIsPaid() const {return isPaid;}

    void setPaid(bool paid) {isPaid = paid;}

    int calculateDayOverdue();

    string toFileFormat();
 };

#endif
