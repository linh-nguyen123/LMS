#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QComboBox>
#include <vector>
#include <string>
#include <QDate>

// [MỚI] Các thư viện đồ họa nâng cao
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

#include "authsystem.h"
#include "library.h"
#include "user.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QInputDialog>
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void onLoginClicked();
    void onLogoutClicked();

    void onViewBooks();
    void onSearchBooks(const QString &text);
    void onAdminSearchBooks(const QString &text);
    void onAddBook();
    void onEditBook();
    void onDeleteBook();
    void onBorrowBook();
    void onReturnBook();
    void onViewMyBooks();
    void onSortClicked();
    void onManageLoansClicked();
    void onAddUserClicked();
    void onChangePasswordClicked();
    void onViewMyPenalties();
    void showAdminMenu();
    void showUserMenu();

private:
    void setupLoginPage();
    void setupAdminPage();
    void setupUserPage();

    void updateTableData(QTableWidget* table, const vector<Book>& booksToShow);
    void refreshDashboardStats();
    void downloadImageFromUrl(const QString &url, QLineEdit *targetInput);
    // [MỚI] Hàm tạo hiệu ứng đổ bóng cho Widget
    void addShadow(QWidget *widget);

    QStackedWidget *stackedWidget;
    QLineEdit *usernameInput;
    QLineEdit *passwordInput;
    QLineEdit *searchBar;
    QLineEdit *adminSearchBar;

    QTableWidget *adminTable;
    QTableWidget *userTable;

    QLabel *lblTotalBooks;
    QLabel *lblTotalBorrowed;
    QLabel *statusLabel;
    QString downloadUrlToLocal(const QString &urlStr);
    QString downloadAndCacheImage(const QString &urlStr);
    bool initializeCoverDirectory();
    QHash<QString, QString> imageCache;  // Cache tránh tải lại
    AuthSystem *authSystem;
    Library *library;
    User *currentUser;
    QNetworkAccessManager *networkManager;
};

#endif // MAINWINDOW_H
