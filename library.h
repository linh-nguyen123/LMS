#ifndef LIBRARY_H
#define LIBRARY_H

#include <vector>
#include <string>
#include "book.h"
#include "borrowrecord.h"
#include "penalty.h"
#include <QDate>
using namespace std;

class Library
{
private:
    vector<Book> books;
    vector<BorrowRecord> borrowRecords;
    vector<Penalty> penalties;
    string filename;
    string borrowFilename;
    string penaltyFilename;

    void saveToFile();
    void saveBorrowRecords();
    void savePenalties();
    void loadPenalties();

public:
    Library();
    void docFile(string fname);
    vector<Book> getBooks() { return books; }

    void loadBorrowRecords();

    void addNewBook(string title, string author, int year, float price,
                    string ISBN, int stockInputted, int stockAvailable,
                    string imagePath);
    bool updateBook(string oldISBN, string newTitle, string newAuthor,
                    int newYear, float newPrice, int newStockInputted,
                    int newStockAvailable, string newImagePath);

    bool deleteBook(string ISBN);
    bool forceReturnBook(string username, string ISBN);
    string borrowBook(string username, string ISBN);
    bool returnBook(string username, string ISBN);


    vector<BorrowRecord> getBorrowedBooksByUser(string username);
    vector<BorrowRecord> getAllBorrowRecords();

    string getBookTitleByISBN(string ISBN);

    //Hàm tính tổng tồn kho
    int getTotalStock();

    // Tìm kiếm và Sắp xếp
    vector<Book> searchBooks(string query);
    void sortBooksByTitle();

    vector<Penalty> getUserPenalties(string username);
    vector<Penalty> getUnpaidPenalties();
    bool payPenalty(string username, string ISBN);
    int autoProcessOverdueBooks();

private:
    void quickSort(int low, int high);
    int partition(int low, int high);
    void swapBooks(Book &a, Book &b);
};

#endif
