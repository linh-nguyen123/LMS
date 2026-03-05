#include "library.h"
#include "book.h"
#include "borrowrecord.h"
#include "qdatetime.h"
#include <fstream>
#include <sstream>
#include <ctime>
#include <iostream>
#include <QString>
#include <QDebug>
#include <QDate>
#include <utility>

Library::Library() :
    filename("InputBooks.txt"),
    borrowFilename("Borrowed_Books_Record.txt"),
    penaltyFilename("penalty.txt")
{
    loadBorrowRecords();
    loadPenalties();
}

void Library::docFile(string fname) {
    filename = fname;
    ifstream fileInput(filename);
    if(!fileInput.is_open()) {
        qWarning() << "Cannot open file:" << QString::fromStdString(fname);
        return;
    }

    books.clear();
    string line;
    int lineNum = 0;

    while(getline(fileInput, line)) {
        lineNum++;
        if(line.empty()) continue;

        stringstream ss(line);
        string title, author, ISBN, tempYear, tempPrice,
            tempStockInputted, tempStockAvailable, imgPath;

        getline(ss, title, '|');
        getline(ss, author, '|');
        getline(ss, tempYear, '|');
        getline(ss, tempPrice, '|');
        getline(ss, ISBN, '|');
        getline(ss, tempStockInputted, '|');
        getline(ss, tempStockAvailable, '|');
        getline(ss, imgPath, '|');

        try {
            int year = stoi(tempYear);
            float price = stof(tempPrice);
            int stockInputted = (tempStockInputted.empty()) ? 0 : stoi(tempStockInputted);
            int stockAvailable = (tempStockAvailable.empty()) ? 0 : stoi(tempStockAvailable);

            Book newBook(title, author, year, price, ISBN,
                         stockInputted, stockAvailable, imgPath);
            books.push_back(newBook);

            qDebug() << "📖 Loaded:" << QString::fromStdString(title)
                     << "- Image:" << QString::fromStdString(imgPath);
        }
        catch(const exception &e) {
            qWarning() << "❌ Error parsing line" << lineNum << ":" << e.what();
        }
    }
    fileInput.close();
    qDebug() << "✅ Loaded" << books.size() << "books";
}

void Library::addNewBook(string title, string author, int year, float price,
                         string ISBN, int stockInputted, int stockAvailable,
                         string imagePath) {
    // Check duplicate ISBN
    for(size_t i = 0; i < books.size(); i++) {
        if(books[i].getISBN() == ISBN) return;
    }

    books.push_back(Book(title, author, year, price, ISBN,
                         stockInputted, stockAvailable, imagePath));
    saveToFile();
}

bool Library::updateBook(string oldISBN, string newTitle, string newAuthor,
                         int newYear, float newPrice, int newStockInputted,
                         int newStockAvailable, string newImagePath) {
    for(auto &book : books) {
        if(book.getISBN() == oldISBN) {
            book = Book(newTitle, newAuthor, newYear, newPrice, oldISBN,
                        newStockInputted, newStockAvailable, newImagePath);
            saveToFile();
            return true;
        }
    }
    return false;
}

bool Library::deleteBook(string ISBN) {
    for(size_t i = 0; i < books.size(); i++) {
        if(books[i].getISBN() == ISBN) {

            if (books[i].getStockAvailable() != books[i].getStockInputted()) {
                return false;
            }

            books.erase(books.begin() + i);
            saveToFile();
            return true;
        }
    }
    return false;  // Book not found
}
string Library::borrowBook(string username, string ISBN) {
    for(size_t i = 0; i < books.size(); i++) {
        if(books[i].getISBN() == ISBN) {

            // Check if user already borrowed this book
            for(const auto& rec : borrowRecords) {
                if(rec.getUsername() == username && rec.getISBN() == ISBN) {
                    return "You have already borrowed this book!";
                }
            }

            // ✅ UPDATED: Check available stock (not total)
            if (books[i].getStockAvailable() <= 0) {
                return "Out of stock! This book is currently unavailable.";
            }

            // ✅ NEW: Decrement available (not total inputted)
            books[i].decrementAvailableStock(1);

            time_t now = time(0);
            tm* ltm = localtime(&now);
            char buffer[20];
            strftime(buffer, 20, "%d/%m/%Y", ltm);
            string borrowDate(buffer);

            time_t dueTime = now + (14LL * 24 * 60 * 60);
            tm* dueLtm = localtime(&dueTime);
            strftime(buffer, 20, "%d/%m/%Y", dueLtm);
            string dueDate(buffer);

            borrowRecords.push_back(BorrowRecord(username, ISBN, borrowDate, dueDate));
            saveToFile();
            saveBorrowRecords();
            return "OK";
        }
    }
    return "Book not found!";
}

bool Library::forceReturnBook(string username, string ISBN) {
    for(size_t i = 0; i< borrowRecords.size(); i++) {
        if(borrowRecords[i].getUsername() == username && borrowRecords[i].getISBN() == ISBN) {
            string bookTitle = getBookTitleByISBN(ISBN);
            string dueDate = borrowRecords[i].getDueDate();

            time_t now = time(0);
            tm* ltm = localtime(&now);
            char buffer[20];
            strftime(buffer, 20, "%d/%m/%y", ltm);
            string returnDate(buffer);
            QDate due = QDate::fromString(QString::fromStdString(dueDate), "d/M/yyyy");
            QDate today = QDate::currentDate();
            if(today > due) {
                int daysOverdue = due.daysTo(today);
            float penaltyAmount = daysOverdue*0.38;

                Penalty p(username, ISBN, bookTitle, dueDate, returnDate, penaltyAmount);
                penalties.push_back(p);

                qDebug() << "⚠️  Penalty created for" << QString::fromStdString(username)
                         << "- Days overdue:" << daysOverdue
                         << "- Penalty: $" << penaltyAmount;
            }
            for(size_t j = 0; j < books.size(); j++) {
                if(books[j].getISBN() == ISBN) {
                    books[j].incrementAvailableStock(1);
                    break;
                }
            }
            borrowRecords.erase(borrowRecords.begin() + i);
            saveToFile();
            saveBorrowRecords();
            savePenalties();
            return true;
        }
    }
    return false;
}

bool Library::returnBook(string username, string ISBN) {
    for(size_t i = 0; i < borrowRecords.size(); i++) {
        if(borrowRecords[i].getUsername() == username &&
            borrowRecords[i].getISBN() == ISBN) {

             string dueDate = borrowRecords[i].getDueDate();

            borrowRecords.erase(borrowRecords.begin() + i);

             time_t now = time(0);
             tm* ltm = localtime(&now);
             char buffer[20];
             strftime(buffer, 20, "%d/%m/%Y", ltm);
             string returnDate(buffer);

             QDate due = QDate::fromString(QString::fromStdString(dueDate), "d/M/yyyy");
             QDate today = QDate::currentDate();

             if(today > due) {
                 int daysOverdue = due.daysTo(today);
                 float penaltyAmount = daysOverdue * 0.38;
                 string bookTitle = getBookTitleByISBN(ISBN);

                 Penalty p(username, ISBN, bookTitle, dueDate,
                           returnDate, penaltyAmount);
                 penalties.push_back(p);

                 qDebug() << "⚠️  Penalty created - User returned late:"
                          << daysOverdue << "days";
             }

            for(size_t j = 0; j < books.size(); j++) {
                if(books[j].getISBN() == ISBN) {
                    books[j].incrementAvailableStock(1);
                    break;
                }
            }

            saveToFile();
            saveBorrowRecords();
            savePenalties();
            return true;
        }
    }
    return false;
}

vector<BorrowRecord> Library::getBorrowedBooksByUser(string username) {
    vector<BorrowRecord> result;
    for(const auto& rec : borrowRecords) {
        if(rec.getUsername() == username) result.push_back(rec);
    }
    return result;
}

vector<BorrowRecord> Library::getAllBorrowRecords() {
    return borrowRecords;
}

string Library::getBookTitleByISBN(string ISBN) {
    for(const auto& book : books) {
        if(book.getISBN() == ISBN) return book.getTitle();
    }
    return "Unknown Title";
}

int Library::getTotalStock() {
    int total = 0;
    for(const auto& b : books) {
        total += b.getStockInputted();  // ✅ Use inputted stock
    }
    return total;
}

vector<Book> Library::searchBooks(string query) {
    vector<Book> results;
    QString q = QString::fromStdString(query);
    for (const auto& book : books) {
        QString title = QString::fromStdString(book.getTitle());
        QString author = QString::fromStdString(book.getAuthor());
        QString isbn = QString::fromStdString(book.getISBN());
        if (title.contains(q, Qt::CaseInsensitive) ||
            author.contains(q, Qt::CaseInsensitive) ||
            isbn.contains(q, Qt::CaseInsensitive)) {
            results.push_back(book);
        }
    }
    return results;
}

void Library::swapBooks(Book &a, Book &b) {
    Book temp = a; a = b; b = temp;
}

int Library::partition(int low, int high) {
    string pivot = books[high].getTitle();
    int i = (low - 1);
    for (int j = low; j <= high - 1; j++) {
        if (books[j].getTitle() < pivot) {
            i++; swapBooks(books[i], books[j]);
        }
    }
    swapBooks(books[i + 1], books[high]);
    return (i + 1);
}

void Library::quickSort(int low, int high) {
    if (low < high) {
        int pi = partition(low, high);
        quickSort(low, pi - 1);
        quickSort(pi + 1, high);
    }
}

void Library::sortBooksByTitle() {
    if (books.empty()) return;
    quickSort(0, books.size() - 1);
}

void Library::saveBorrowRecords() {
    ofstream fileOutput(borrowFilename);
    if(!fileOutput.is_open()) return;
    for(auto& rec : borrowRecords) {
        fileOutput << rec.toFileFormat() << endl;
    }
    fileOutput.close();
}

void Library::loadBorrowRecords() {
    ifstream fileInput(borrowFilename);
    if(!fileInput.is_open()) return;
    borrowRecords.clear();
    string line;
    while(getline(fileInput, line)) {
        if(line.empty()) continue;
        stringstream ss(line);
        string username, ISBN, borrowDate, dueDate;
        getline(ss, username, '|');
        getline(ss, ISBN, '|');
        getline(ss, borrowDate, '|');
        getline(ss, dueDate, '|');
        borrowRecords.push_back(BorrowRecord(username, ISBN, borrowDate, dueDate));
    }
    fileInput.close();
}

vector<Penalty> Library::getUserPenalties(string username) {
    vector<Penalty> result;
    for(const auto& p : penalties) {
        if(p.getUsername() == username) {
            result.push_back(p);
        }
    }
    return result;
}

vector<Penalty> Library::getUnpaidPenalties() {
    vector<Penalty> result;
    for(const auto& p : penalties) {
        if(!p.getIsPaid()) {
            result.push_back(p);
        }
    }
    return result;
}

bool Library::payPenalty(string username, string ISBN) {
    for(size_t i = 0; i < penalties.size(); i++) {
        if(penalties[i].getUsername() == username &&
            penalties[i].getISBN() == ISBN &&
            !penalties[i].getIsPaid()) {

            penalties[i].setPaid(true);
            savePenalties();
            return true;
        }
    }
    return false;
}


void Library::loadPenalties() {
    ifstream fileInput(penaltyFilename);
    if(!fileInput.is_open()) return;

    penalties.clear();
    string line;

    while(getline(fileInput, line)) {
        if(line.empty()) continue;

        stringstream ss(line);
        string username, ISBN, bookTitle, dueDate, returnDate;
        string daysStr, penaltyStr, isPaidStr;

        getline(ss, username, '|');
        getline(ss, ISBN, '|');
        getline(ss, bookTitle, '|');
        getline(ss, dueDate, '|');
        getline(ss, returnDate, '|');
        getline(ss, daysStr, '|');
        getline(ss, penaltyStr, '|');
        getline(ss, isPaidStr, '|');

        try {
            float penaltyAmount = stof(penaltyStr);
            Penalty p(username, ISBN, bookTitle, dueDate, returnDate, penaltyAmount);

            if(isPaidStr == "1") {
                p.setPaid(true);
            }

            penalties.push_back(p);
        }
        catch(const exception &e) {
            qWarning() << "Error loading penalty:" << e.what();
        }
    }
    fileInput.close();
}

void Library::savePenalties() {
    ofstream fileOutput(penaltyFilename);
    if(!fileOutput.is_open()) return;

    for(auto& p : penalties) {
        fileOutput << p.toFileFormat() << endl;
    }
    fileOutput.close();
}

int Library::autoProcessOverdueBooks() {
    vector<pair<string, string>> overdueList; // Lưu tạm: Username + ISBN

    QDate today = QDate::currentDate();

    // BƯỚC 1: Quét tìm sách quá hạn
    for (const auto& rec : borrowRecords) {
        // Chuyển string date sang QDate để so sánh
        QDate due = QDate::fromString(QString::fromStdString(rec.getDueDate()), "d/M/yyyy");

        if (today > due) {
            // Nếu hôm nay lớn hơn hạn trả -> Lưu lại để xử lý
            overdueList.push_back({rec.getUsername(), rec.getISBN()});
        }
    }

    // BƯỚC 2: Thực hiện Force Return cho danh sách đã lọc
    int count = 0;
    for (const auto& item : overdueList) {
        // Gọi lại hàm forceReturnBook bạn đã viết sẵn
        if (forceReturnBook(item.first, item.second)) {
            count++;
        }
    }

    // Lưu lại thay đổi vào file
    saveBorrowRecords();
    savePenalties();
    saveToFile(); // Lưu lại số lượng sách trong kho

    return count; // Trả về số lượng người đã bị phạt
}


void Library::saveToFile() {
    ofstream fileOutput(filename);
    if(!fileOutput.is_open()) return;
    for(auto& book : books) {
        fileOutput << book.toFileFormat() << endl;
    }
    fileOutput.close();
}
