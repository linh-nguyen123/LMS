#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <fstream>
#include <cmath>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <ctime>
#include <limits> 

using namespace std;

// Helper function to convert string to lowercase
string toLower(const string &s) {
    string result = s;
    transform(result.begin(), result.end(), result.begin(),
              [](unsigned char c){ return tolower(c); });
    return result;
}

// ========== CLASS BOOK ==========
class Book {
private:
    string title;
    string author;
    int year;
    float price;
    string ISBN;

public:
    Book(string title, string author, int year, float price, string ISBN)
    {
        this->title = title;
        this->author = author;
        this->year = year;
        this->price = price;
        this->ISBN = ISBN;
    }

    // Displays full book details
    void display(){
        cout << "Title:  " << title << endl;
        cout << "Author: " << author << endl;
        cout << "Year:   " << year << endl;
        cout << "Price:  $" << fixed << setprecision(2) << price << endl;
        cout << "ISBN:   " << ISBN << endl;
    }

    string getTitle() { return title; }
    string getAuthor() { return author; }
    int getYear() { return year; }
    float getPrice() { return price; }
    string getISBN() { return ISBN; }

    string toFileFormat() {
        ostringstream oss;
        oss << fixed << setprecision(2) << price;
        return title + "|" + author + "|" + to_string(year) + "|" +
               oss.str() + "|" + ISBN;
    }

    // Matching functions for search
    bool matchTitle(string query) {
        string titleLower = toLower(title);
        string queryLower = toLower(query);
        return titleLower.find(queryLower) != string::npos;
    }

    bool matchAuthor(string query) {
        string authorLower = toLower(author);
        string queryLower = toLower(query);
        return authorLower.find(queryLower) != string::npos;
    }

    bool matchPrice(float query) {
        return price == query;
    }

    bool matchISBN(string query) {
        string isbnLower = toLower(ISBN);
        string queryLower = toLower(query);
        return isbnLower == queryLower;
    }

    bool matchYear(int query) {
        return year == query;
    }
};

// ========== CLASS BORROW RECORD ==========
class BorrowRecord {
private:
    string username;
    string ISBN;
    string borrowDate;
    string dueDate;

public:
    BorrowRecord(string username, string ISBN, string borrowDate, string dueDate)
        : username(username), ISBN(ISBN), borrowDate(borrowDate), dueDate(dueDate) {}

    string getUsername() { return username; }
    string getISBN() { return ISBN; }
    string getBorrowDate() { return borrowDate; }
    string getDueDate() { return dueDate; }

    string toFileFormat() {
        return username + "|" + ISBN + "|" + borrowDate + "|" + dueDate;
    }
};

// ========== CLASS USER ==========
class User
{
protected:
    string username;
    string password;
    string role;

public:
    User(string username, string password, string role)
    {
        this->username = username;
        this->password = password;
        this->role = role;
    }
    string getUsername() { return username; }
    string getRole() { return role; }
    bool verify(string pass) { return password == pass; }

    virtual ~User() {}
};

// ========== CLASS AUTH SYSTEM ==========
class AuthSystem
{
private:
    vector<User> users;

public:
    AuthSystem(){
        users.push_back(User("admin", "admin123", "admin"));
        users.push_back(User("user1", "user123", "user"));
    }

    User* login(string username, string password)
    {
        for(int i=0; i<users.size();i++)
        {
            if(users[i].getUsername() == username && users[i].verify(password)) {
                cout << "\nLogin successful!" << endl;
                cout << "Role: " << users[i].getRole() << endl;
                return new User(users[i]);
            }
        }
        cout << "\nWrong username or password!" << endl;
        return nullptr;
    }

    void registerUser(string username, string password, string role = "user") {
        for(int i = 0; i < users.size(); i++) {
            if(users[i].getUsername() == username) {
                cout << "Username already exists!" << endl;
                return;
            }
        }
        users.push_back(User(username, password, role));
        cout << "Add user successfully!" << endl;
    }
};

// ========== CLASS LIBRARY ==========
class Library {
private:
    vector<Book> books;
    vector<BorrowRecord> borrowRecords;
    string filename;
    string borrowFilename;

public:
    Library() : filename("InputBooks.txt"), borrowFilename("Borrowed_Books_Record.txt") {}

    // Load Books
    void docFile(string fname){
        filename = fname;
        ifstream fileInput(filename);
        if(!fileInput.is_open()){
            cout << "Error: File not found '" << filename << "'" << endl;
            return;
        }

        books.clear();
        int count = 0;
        string line;

        while(getline(fileInput, line)) {
            if(line.empty()) continue;
            stringstream ss(line);
            string title, author, ISBN, tempYear, tempPrice;

            getline(ss, title, '|'); getline(ss, author, '|');
            getline(ss, tempYear, '|'); getline(ss, tempPrice, '|');
            getline(ss, ISBN, '|');

            try {
                int year = stoi(tempYear);
                float price = stof(tempPrice);
                Book newBook(title, author, year, price, ISBN);
                books.push_back(newBook);
                count++;
            } catch(...) {}
        }
        fileInput.close();
        cout << "Loaded " << count << " books from file successfully!" << endl;
    }

    // Add New Book
    void addNewBook() {
        string title, author, ISBN;
        int year;
        float price;

        cout << "\n========== ADD NEW BOOK ==========\n";
        cout << "Title: "; cin.ignore(); getline(cin, title);
        cout << "Author: "; getline(cin, author);
        cout << "Year: "; cin >> year;
        cout << "Price: $"; cin >> price;
        cout << "ISBN: "; cin.ignore(); getline(cin, ISBN);

        for(int i = 0; i < books.size(); i++) {
            if(books[i].getISBN() == ISBN) {
                cout << "ISBN already exists!" << endl;
                return;
            }
        }
        Book newBook(title, author, year, price, ISBN);
        books.push_back(newBook);
        saveToFile();
        cout << "Book added successfully!" << endl;
    }

    // Delete Book
    void deleteBook() {
        if(books.empty()) { cout << "Library is empty!" << endl; return; }
        string searchISBN;
        cout << "\n========== DELETE BOOK ==========\n";
        cout << "Enter ISBN to delete: "; cin.ignore(); getline(cin, searchISBN);

        for(int i = 0; i < books.size(); i++) {
            if(books[i].getISBN() == searchISBN) {
                cout << "\nDeleting this book:\n";
                books[i].display();
                cout << "---------------------\n";
                books.erase(books.begin() + i);
                saveToFile();
                cout << "Book deleted successfully!" << endl;
                return;
            }
        }
        cout << "Book not found!" << endl;
    }

    // ========== BORROW BOOK ==========
    void borrowBook(string username) {
        if(books.empty()) {
            cout << "Library is empty! Cannot borrow." << endl;
            return;
        }

        int option;
        string searchISBN = ""; 

        cout << "\n========== BORROW BOOK OPTION ==========\n";
        cout << "1. Search for a book to borrow\n";
        cout << "2. Enter/Paste ISBN directly\n";
        cout << "Selection: ";
        if (!(cin >> option)) {
             cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
             cout << "Invalid input!\n"; return;
        }

        if (option == 1) {
            string query;
            int searchChoice;
            cout << "\nSearch by: 1.Title  2.Author  3.Year  4.Price  5.ISBN\nChoice: ";
            if(!(cin >> searchChoice)) {
                cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); return;
            }
            cout << "Enter query: "; cin.ignore(); getline(cin, query);

            vector<int> foundIndices; 
            cout << "\n========== SEARCH RESULTS ==========\n";
            int count = 0;
            for(int i = 0; i < books.size(); i++) {
                bool isMatch = false;
                switch(searchChoice) {
                    case 1: isMatch = books[i].matchTitle(query); break;
                    case 2: isMatch = books[i].matchAuthor(query); break;
                    case 3: try { isMatch = books[i].matchYear(stoi(query)); } catch(...) {} break;
                    case 4: try { isMatch = books[i].matchPrice(stof(query)); } catch(...) {} break;
                    case 5: isMatch = books[i].matchISBN(query); break;
                }
                if(isMatch) {
                    foundIndices.push_back(i); 
                    count++;
                    cout << "NO: [" << count << "]\n"; 
                    books[i].display();
                    cout << "---------------------\n";
                }
            }

            if(count == 0) { cout << "No books found!\n"; return; }

            int selectNum;
            cout << "Enter the NO. of the book you want to borrow (1-" << count << "): ";
            if (!(cin >> selectNum) || selectNum < 1 || selectNum > count) {
                cout << "Invalid selection!\n"; cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); return;
            }
            int realIndex = foundIndices[selectNum - 1];
            searchISBN = books[realIndex].getISBN();
            cout << "You selected: " << books[realIndex].getTitle() << endl;
        } else {
            cout << "Enter ISBN of book to borrow: "; cin.ignore(); getline(cin, searchISBN);
        }

        for(int i = 0; i < books.size(); i++) {
            if(books[i].getISBN() == searchISBN) {
                for(int j = 0; j < borrowRecords.size(); j++) {
                    if(borrowRecords[j].getISBN() == searchISBN) {
                        cout << "This book is already borrowed by another user!" << endl; return;
                    }
                }
                cout << "\nProcessing borrowing request...\n";
                time_t now = time(0); tm* ltm = localtime(&now);
                string borrowDate = to_string(ltm->tm_mday) + "/" + to_string(ltm->tm_mon + 1) + "/" + to_string(ltm->tm_year + 1900);
                time_t dueTime = now + (14LL * 24 * 60 * 60); 
                tm* dueLtm = localtime(&dueTime);
                string dueDate = to_string(dueLtm->tm_mday) + "/" + to_string(dueLtm->tm_mon + 1) + "/" + to_string(dueLtm->tm_year + 1900);

                BorrowRecord record(username, searchISBN, borrowDate, dueDate);
                borrowRecords.push_back(record);
                saveBorrowRecords();
                cout << "Book borrowed successfully! (Due in 14 days)" << endl;
                return;
            }
        }
        cout << "Book with ISBN " << searchISBN << " not found!" << endl;
    }

    // ========== RETURN BOOK ==========
    void returnBook(string username) {
        if(borrowRecords.empty()) {
            cout << "No borrow records in the system!" << endl;
            return;
        }

        vector<int> userRecordIndices; 
        
        cout << "\n================= RETURN BOOK =================\n";
        cout << "Here are the books you have borrowed:\n";
        int displayCount = 0;

        for(int i = 0; i < borrowRecords.size(); i++) {
            if(borrowRecords[i].getUsername() == username) {
                userRecordIndices.push_back(i);
                displayCount++;
                
                cout << "TAG [" << displayCount << "]\n";
                
                string isbn = borrowRecords[i].getISBN();
                bool bookFound = false;
                for(auto& b : books) {
                    if(b.getISBN() == isbn) {
                        b.display(); 
                        bookFound = true;
                        break;
                    }
                }
                if(!bookFound) {
                    cout << "ISBN: " << isbn << " (Book details unavailable/deleted)\n";
                }
                
                cout << "Borrow Date: " << borrowRecords[i].getBorrowDate() << endl;
                cout << "Due Date:    " << borrowRecords[i].getDueDate() << endl;
                cout << "--------------------------------------------\n";
            }
        }

        if(userRecordIndices.empty()) {
            cout << "You currently have no books to return.\n";
            return;
        }

        int choice;
        cout << "Enter the TAG number of the book to return (0 to Cancel): ";
        if (!(cin >> choice)) {
             cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
             cout << "Invalid input!\n";
             return;
        }

        if (choice == 0) {
            cout << "Return action cancelled.\n";
            return;
        }

        if (choice < 1 || choice > userRecordIndices.size()) {
            cout << "Invalid Tag number selected.\n";
            return;
        }

        int indexToDelete = userRecordIndices[choice - 1]; 
        
        cout << "\nReturning book..." << endl;
        string isbnToDelete = borrowRecords[indexToDelete].getISBN();
        for(auto& b : books) {
            if(b.getISBN() == isbnToDelete) {
                cout << "Returned: " << b.getTitle() << endl;
                break;
            }
        }

        borrowRecords.erase(borrowRecords.begin() + indexToDelete);
        saveBorrowRecords();
        cout << "Book returned successfully!" << endl;
    }

    // ========== DISPLAY MY BORROWED BOOKS ==========
    void displayMyBorrowedBooks(string username) {
        cout << "\n============================================\n";
        cout << "            MY BORROWED BOOKS               \n";
        cout << "============================================\n";
        int count = 0;

        for(int i = 0; i < borrowRecords.size(); i++) {
            if(borrowRecords[i].getUsername() == username) {
                cout << "NO. [" << (count + 1) << "]\n";
                
                bool bookFound = false;
                string currentISBN = borrowRecords[i].getISBN();
                
                for(int j = 0; j < books.size(); j++) {
                    if(books[j].getISBN() == currentISBN) {
                        books[j].display(); 
                        bookFound = true;
                        break;
                    }
                }
                if(!bookFound) {
                    cout << "[Details unavailable] ISBN: " << currentISBN << endl;
                }
                cout << "Borrow Date: " << borrowRecords[i].getBorrowDate() << endl;
                cout << "Due Date:    " << borrowRecords[i].getDueDate() << endl;
                cout << "--------------------------------------------\n";
                count++;
            }
        }
        if(count == 0) cout << "You haven't borrowed any books!" << endl;
    }

    // ========== FILE OPERATIONS ==========
    void saveBorrowRecords() {
        ofstream fileOutput(borrowFilename);
        if(!fileOutput.is_open()) { cout << "Error: Cannot open file to save!" << endl; return; }
        for(int i = 0; i < borrowRecords.size(); i++) {
            fileOutput << borrowRecords[i].toFileFormat() << endl;
        }
        fileOutput.close();
    }

    void loadBorrowRecords() {
        ifstream fileInput(borrowFilename);
        if(!fileInput.is_open()){ return; }
        borrowRecords.clear();
        string line;
        while(getline(fileInput, line)) {
            if(line.empty()) continue;
            stringstream ss(line);
            string username, ISBN, borrowDate, dueDate;
            getline(ss, username, '|'); getline(ss, ISBN, '|');
            getline(ss, borrowDate, '|'); getline(ss, dueDate, '|');
            BorrowRecord record(username, ISBN, borrowDate, dueDate);
            borrowRecords.push_back(record);
        }
        fileInput.close();
    }

    void saveToFile() {
        ofstream fileOutput(filename);
        if(!fileOutput.is_open()) { cout << "Error: Cannot open file to save!" << endl; return; }
        for(int i = 0; i < books.size(); i++) {
            fileOutput << books[i].toFileFormat() << endl;
        }
        fileOutput.close();
    }

    // ========== ADMIN & UTILS ==========
    void ManagementBorrower(string username){ 
        cout << "\n========== BORROWER BOOKS LIST ==========\n";
        int count = 0;
        for(int i = 0; i < borrowRecords.size(); i++) {
            if(borrowRecords[i].getUsername() == username) {
                cout << "[" << (count + 1) << "]\n";
                bool bookFound = false;
                string currentISBN = borrowRecords[i].getISBN();
                for(int j = 0; j < books.size(); j++) {
                    if(books[j].getISBN() == currentISBN) {
                        books[j].display();
                        bookFound = true;
                        break;
                    }
                }
                if(!bookFound) cout << "[Details unavailable] ISBN: " << currentISBN << endl;
                
                cout << "Borrow Date: " << borrowRecords[i].getBorrowDate() << endl;
                cout << "Due Date:    " << borrowRecords[i].getDueDate() << endl;
                cout << "---------------------\n";
                count++;
            }
        }
        if(count == 0) cout << "This user hasn't borrowed any books!" << endl;
        else cout << "\nTotal borrowed books: " << count << endl;
    }

    void adminViewBorrowerDetails() {
        string username;
        cout << "\n========== VIEW BORROWER DETAILS ==========\n";
        cout << "Enter username: "; cin.ignore(); getline(cin, username);
        ManagementBorrower(username);
    }

    void adminForceReturn(){ 
        string username, ISBN;
        cout << "\n========== FORCE RETURN BOOK ==========\n";
        cout << "Enter username: "; cin.ignore(); getline(cin, username);
        cout << "Enter ISBN: "; getline(cin, ISBN);

        for(int i=0;i<borrowRecords.size(); i++) {
            if(borrowRecords[i].getUsername() == username && borrowRecords[i].getISBN() == ISBN) {
                cout << "\nForcing return of: \n";
                 for(int j=0; j<books.size(); j++) {
                    if(books[j].getISBN() == ISBN) { books[j].display(); break; }
                }
                borrowRecords.erase(borrowRecords.begin()+i);
                saveBorrowRecords();
                cout << "Book return forced successfully!" << endl; return;
            }
        }
        cout << "This user hasn't borrowed this book!" << endl;
    }

    void displayAllBorrowers() {
        cout << "\n========== ALL BORROWERS ==========\n";
        vector<string> borrowers; 
        for(int i=0;i<borrowRecords.size();i++) {
            string currentUsername = borrowRecords[i].getUsername();
            bool found = false;
            for(int j=0;j<borrowers.size(); j++) {
                if(borrowers[j] == currentUsername) { found = true; break; }
            }
            if(!found) {
                borrowers.push_back(currentUsername);
                int bookCount = 0;
                for(int j=0; j<borrowRecords.size(); j++) {
                    if(borrowRecords[j].getUsername() == currentUsername) bookCount++;
                }
                cout << "[User: " << currentUsername << "] - Books borrowed: " << bookCount << endl;
            }
        }
        if(borrowers.empty()) cout << "No borrowers found!" << endl;
    }

    void displayBooks(){
        if(books.empty()) { cout << "Library is empty. No books to display." << endl; } 
        else {
            cout << "\n========== ALL BOOKS ==========\n";
            for(int i=0; i<books.size(); i++) {
                cout << "[" << (i+1) << "]\n";
                books[i].display();
                cout << "---------------------\n";
            }
        }
    }

void searchBooks() {
        if(books.empty()) { cout << "Library is empty. No books to search." << endl; } 
        else {
            string query; 
            int choice;
            cout << "\n========== SEARCH BOOK ==========\n";
            cout << "1. Title\n2. Author\n3. Year\n4. Price\n5. ISBN\nChoice: "; 
            
            // ========== FIX: Validate Input ==========
            if (!(cin >> choice)) {
                cout << "Invalid input! Please enter a number (1-5)." << endl;
                cin.clear(); // Clear error flag
                cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Remove bad input from buffer
                return; // Go back to menu
            }
            // =========================================

            cout << "Enter query: "; cin.ignore(); getline(cin, query);
            cout << "\n========== SEARCH RESULTS ==========\n";
            int count = 0;
            for(int i=0; i<books.size(); i++) {
                bool isMatch = false;
                switch (choice) {
                    case 1: isMatch = books[i].matchTitle(query); break;
                    case 2: isMatch = books[i].matchAuthor(query); break;
                    case 3: try { isMatch = books[i].matchYear(stoi(query)); } catch(...) {} break;
                    case 4: try { isMatch = books[i].matchPrice(stof(query)); } catch(...) {} break;
                    case 5: isMatch = books[i].matchISBN(query); break;
                    default: cout << "Invalid choice." << endl; return;
                }
                if(isMatch) {
                    cout << "[" << (count+1) << "]\n";
                    books[i].display();
                    cout << "---------------------\n";
                    count++;
                }
            }
            if(count == 0) cout << "No books found!" << endl;
        }
    }

    int getTotalBooks() { return books.size(); }

    void displayBorrowManagementMenu(){
        cout << "\n====================================\n";
        cout << "  BORROW MANAGEMENT (ADMIN)\n";
        cout << "\n====================================\n";
        cout << "1. View all borrowers\n2. View borrower details\n3. Force return book\n4. Back to admin menu\nSelection: ";
    }
};

// ========== MENUS & MAIN ==========
void displayAdminMenu(User* currentUser, Library& myLib) {
    cout << "\n====================================\n";
    cout << "       MENU MANAGEMENT (ADMIN)\n";
    cout << "====================================\n";
    // UPDATED: Removed "Load books from file" option
    cout << "1. Add new book\n2. Delete book\n3. See all books\n4. Add new user\n5. Manage borrow records\n6. Total books: " << myLib.getTotalBooks() << "\n7. Log out\nSelection: ";
}

void displayUserMenu() {
    cout << "\n====================================\n";
    cout << "       MENU USER (USER)\n";
    cout << "====================================\n";
    cout << "1. View all books\n2. Search book\n3. Borrow book\n4. Return book\n5. View my borrowed books\n6. Log out\nSelection: ";
}

int main()
{
    AuthSystem authSystem;
    Library myLib;
    myLib.loadBorrowRecords();
    User* currentUser = nullptr;
    int choice;

    while(true) {
        if(currentUser == nullptr) {
            cout << "\n====================================\n";
            cout << "   LIBRARY MANAGEMENT SYSTEM (LMS)\n";
            cout << "====================================\n";
            cout << "1. Log in\n2. Exit\nSelection: ";
            
            // ========== FIX 1: Main Menu Protection ==========
            if (!(cin >> choice)) {
                 cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
                 cout << "Invalid input. Please enter a number.\n"; continue;
            }
            // =================================================

            if(choice == 1) {
                int loginAttempts = 0;
                const int MAX_ATTEMPTS = 3;
                while(loginAttempts < MAX_ATTEMPTS) {
                    string username, password;
                    cin.ignore();
                    cout << "\nUsername: "; getline(cin, username);
                    cout << "Password: "; getline(cin, password);

                    currentUser = authSystem.login(username, password);
                    if(currentUser != nullptr) {
                        myLib.docFile("InputBooks.txt"); 
                        break;
                    }
                    loginAttempts++;
                    cout << "Attempt: " << loginAttempts << "/" << MAX_ATTEMPTS << endl;
                    if(loginAttempts >= MAX_ATTEMPTS) {
                        cout << "\nWrong password!\n"; break;
                    }
                }
            } else if(choice == 2) { cout << "Goodbye!" << endl; break; }
        }
        else {
            if(currentUser->getRole() == "admin") {
                displayAdminMenu(currentUser, myLib);
                
                // ========== FIX 2: Admin Menu Protection ==========
                if (!(cin >> choice)) {
                     cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
                     cout << "Invalid input. Please enter a number.\n"; continue;
                }
                // ==================================================

                switch(choice) {
                    case 1: myLib.addNewBook(); break;
                    case 2: myLib.deleteBook(); break;
                    case 3: myLib.displayBooks(); break;
                    case 4: {
                        string username, password; cin.ignore();
                        cout << "\nNew Username: "; getline(cin, username);
                        cout << "Password: "; getline(cin, password);
                        authSystem.registerUser(username, password, "user"); break;
                    }
                    case 5: {
                        bool inBorrowMenu = true;
                        while(inBorrowMenu) {
                            myLib.displayBorrowManagementMenu();
                            int borrowChoice; 
                            
                            // Fix inner menu too
                            if (!(cin >> borrowChoice)) {
                                cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                cout << "Invalid input!\n"; continue;
                            }

                            switch(borrowChoice) {
                                case 1: myLib.displayAllBorrowers(); break;
                                case 2: myLib.adminViewBorrowerDetails(); break;
                                case 3: myLib.adminForceReturn(); break;
                                case 4: inBorrowMenu = false; break;
                                default: cout << "Invalid choice!\n";
                            }
                        }
                        break;
                    }
                    case 6: cout << "\nTotal books: " << myLib.getTotalBooks() << endl; break;
                    case 7: cout << "Log out. Goodbye!\n"; delete currentUser; currentUser = nullptr; break;
                    default: cout << "Selection is invalid!\n";
                }
            }
            else if(currentUser->getRole() == "user") {
                displayUserMenu();
                
                // ========== FIX 3: User Menu Protection ==========
                if (!(cin >> choice)) {
                     cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
                     cout << "Invalid input. Please enter a number.\n"; continue;
                }
                // =================================================

                switch(choice) {
                    case 1: myLib.displayBooks(); break;
                    case 2: myLib.searchBooks(); break;
                    case 3: myLib.borrowBook(currentUser->getUsername()); break;
                    case 4: myLib.returnBook(currentUser->getUsername()); break;
                    case 5: myLib.displayMyBorrowedBooks(currentUser->getUsername()); break;
                    case 6: cout << "Log out. Goodbye!\n"; delete currentUser; currentUser = nullptr; break;
                    default: cout << "Selection is invalid!\n";
                }
            }
        }
    }
    return 0;
}