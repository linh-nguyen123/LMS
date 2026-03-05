#include "penalty.h"
#include <ctime>
#include <sstream>
#include <iomanip>

Penalty::Penalty(string username, string ISBN, string bookTitle, string dueDate,string returnDate, float penaltyAmount) {
    this->username = username;
    this->ISBN = ISBN;
    this->bookTitle = bookTitle;
    this->dueDate = dueDate;
    this->returnDate = returnDate;
    this->penaltyAmount = penaltyAmount;
    isPaid = false;
    daysOverdue = calculateDayOverdue();
}

int Penalty::calculateDayOverdue() {
    int duDay, duMonth, duYear;
    int retDay, retMonth, retYear;

    sscanf(dueDate.c_str(), "%d/%d/%d", &duDay, &duMonth, &duYear);
    sscanf(returnDate.c_str(), "%d/%d/%d", &retDay, &retMonth, &retYear);


//hàm kiểm tra năm nhuận
auto isLeapYear  = [](int year) -> bool {
    return (year % 400 ==0) || (year % 4 == 0 && year % 100 !=0);
};

//hàm số ngày trong 1 tháng
auto getDaysInMonth = [isLeapYear](int month, int year) -> int {
    int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if(month==2 && isLeapYear(year)) {
        return 29;
    }

    return days[month-1];
};

//chuyển ngày -> số ngày tuyệt đối
auto dateToTotalDays = [&](int d, int m, int y) -> long long {
    long long total = y*365 + (y/4) - (y/100) + (y/400);
    for(int i=1;i<m;i++) {
        total += getDaysInMonth(i,y);
    }
    total+=d;
    return total;
};

long long dueTotalDays = dateToTotalDays(duDay, duMonth, duYear);
long long retTotalDays = dateToTotalDays(retDay, retMonth, retYear);

int overdue = retTotalDays-dueTotalDays;

return overdue > 0 ? overdue : 0;
}

string Penalty::toFileFormat() {
    ostringstream oss;
    oss << fixed << setprecision(2) << penaltyAmount;
    return username + "|" + ISBN + "|" + bookTitle + "|" +
           dueDate + "|" + returnDate + "|" + to_string(daysOverdue) + "|" +
           oss.str() + "|" + (isPaid ? "1" : "0");
}
