#ifndef BOOK_H
#define BOOK_H

#include <string>
#include <iostream>

using namespace std;

class Book
{
private:
    string title;
    string author;
    int year;
    float price;
    string ISBN;
    int stockInputted; //stock được nhập kho
    int stockAvailable; //stock còn tồn
    string imagePath;
public:
    Book(string title, string author, int year, float price, string ISBN,int stockInputted,int stockAvailable , string imagePath="");

    string getTitle() const { return title; }
    string getAuthor() const { return author; }
    int getYear() const { return year; }
    float getPrice() const { return price; }
    string getISBN() const { return ISBN; }

    int getStockInputted() const {return stockInputted;}
    int getStockAvailable() const {return stockAvailable;}
    string getImagePath() const { return imagePath; }
    int getTotalStock() const {
        return stockInputted;
    }
    int getStockBorrowed() const {
        return stockInputted - stockAvailable;
    }
    // qty = 1 là số thao tác mượn của user
    void decrementAvailableStock(int qty = 1) {
        if (stockAvailable >= qty) {
            stockAvailable -= qty;
        }
    }
    //qty = 1 là số thao tác trả sách của user
    void incrementAvailableStock(int qty = 1) {
        if (stockAvailable + qty <= stockInputted) {
            stockAvailable += qty;
        }
    }
    void setStockInputted(int newStock) {
        stockInputted = newStock;
        if (stockAvailable > stockInputted) {
            stockAvailable = stockInputted;
        }
    }
    void setStockAvailable(int newQty) {
        if (newQty >= 0 && newQty <= stockInputted) {
            stockAvailable = newQty;
        }
    }
    void setImagePath(string path) { imagePath = path; }
    string toFileFormat();

    // Các hàm tìm kiếm
    bool matchTitle(string query);
    bool matchAuthor(string query);
    bool matchPrice(float query);
    bool matchISBN(string query);
    bool matchYear(int query);
};

#endif
