#include "book.h"
#include <sstream>
#include <iomanip>
#include <cctype>
#include <cstring>

string toLower(const string &s) {
    string result = s;
    for(char &c : result) {
        c = tolower((unsigned char)c);
    }
    return result;
}

Book::Book(string title, string author, int year, float price, string ISBN,int stockInputted, int stockAvailable, string imagePath)
    : title(title), author(author), year(year), price(price), ISBN(ISBN), stockInputted(stockInputted),
    stockAvailable(stockAvailable), imagePath(imagePath)
{
    if (this->stockAvailable > this->stockInputted) {
        this->stockAvailable = this->stockInputted;
    }
}


string Book::toFileFormat() {
    ostringstream oss;
    oss << fixed << setprecision(2) << price;

    return title + "|" + author + "|" + to_string(year) + "|" +
           oss.str() + "|" + ISBN + "|" + to_string(stockInputted) + "|" +
           to_string(stockAvailable) + "|" + imagePath;
}

bool Book::matchTitle(string query) {
    string titleLower = toLower(title);
    string queryLower = toLower(query);
    return titleLower.find(queryLower) != string::npos;
}

bool Book::matchAuthor(string query) {
    string authorLower = toLower(author);
    string queryLower = toLower(query);
    return authorLower.find(queryLower) != string::npos;
}

bool Book::matchPrice(float query) {
    return price == query;
}

bool Book::matchISBN(string query) {
    string isbnLower = toLower(ISBN);
    string queryLower = toLower(query);
    return isbnLower == queryLower;
}

bool Book::matchYear(int query) {
    return year == query;
}
