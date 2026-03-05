#include "mainwindow.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QHeaderView>
#include <QStyle>
#include <QDate>
#include <QGroupBox>
#include <QGraphicsDropShadowEffect>
#include <QFile>       // [QUAN TRỌNG] Để dùng QFile::exists
#include <QFileDialog> // [QUAN TRỌNG] Để dùng QFileDialog::getOpenFileName
#include <QNetworkAccessManager> // [MỚI]
#include <QNetworkReply>         // [MỚI]
#include <QEventLoop>            // [MỚI] Xử lý tải đồng bộ
#include <QDir>                  // [MỚI]
#include <QTimer>           // ✅ THÊM DÒNG NÀY
#include <QDateTime>        // ✅ THÊM DÒNG NÀY (nếu chưa có)
#include <QInputDialog>     // ✅ THÊM DÒNG NÀY (cho dialog nhập URL)
#include <QUrl>            // ✅ THÊM DÒNG NÀY
#include <QDebug>
// --- 1. HELPER FUNCTIONS ---

QIcon getIcon(QStyle::StandardPixmap type) {
    return QApplication::style()->standardIcon(type);
}

// Tạo bóng đổ mềm mại (Soft Shadow)
void applyShadow(QWidget *widget, int blur = 20, int offset = 5) {
    if (!widget) return;
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(widget);
    shadow->setBlurRadius(blur);
    shadow->setXOffset(0);
    shadow->setYOffset(offset);
    shadow->setColor(QColor(0, 0, 0, 80)); // Màu đen mờ 80%
    widget->setGraphicsEffect(shadow);
}

// [ĐÃ SỬA LỖI] Hàm xử lý ảnh thông minh
QString processImagePath(QString inputPath) {
    if (inputPath.isEmpty()) return "";

    // Nếu là link online (http/https)
    if (inputPath.startsWith("http://") || inputPath.startsWith("https://")) {
        // 1. Tạo thư mục covers nếu chưa có
        QDir dir;
        if (!dir.exists("covers")) dir.mkpath("covers");

        // 2. Tạo tên file dựa trên thời gian để không trùng
        QString fileName = "covers/img_" + QString::number(QDateTime::currentMSecsSinceEpoch()) + ".jpg";

        // 3. [FIX QUAN TRỌNG] Tách QUrl ra biến riêng để tránh lỗi biên dịch
        QUrl url(inputPath);
        if (!url.isValid()) return ""; // Nếu link hỏng thì bỏ qua

        QNetworkRequest request(url);

        // 4. Khai báo Manager cục bộ để xử lý tải ngay tại đây
        QNetworkAccessManager manager;
        QNetworkReply *reply = manager.get(request);

        // 5. Dùng EventLoop để đợi tải xong (Biến bất đồng bộ thành đồng bộ)
        QEventLoop loop;
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec(); // Treo ở đây cho đến khi tải xong

        // 6. Kiểm tra lỗi và lưu file
        if (reply->error() == QNetworkReply::NoError) {
            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(reply->readAll());
                file.close();
                delete reply;
                return fileName; // Thành công: Trả về đường dẫn file trên máy
            }
        }
        delete reply;
        return ""; // Thất bại: Trả về rỗng
    }

    return inputPath; // Nếu là đường dẫn file có sẵn trên máy thì giữ nguyên
}

// Setup Bảng dữ liệu chuẩn "Pro"
void setupTableStyle(QTableWidget *table) {
    if (!table) return;

    table->setColumnCount(7);
    QStringList headers = {"COVER & TITLE", "", "AUTHOR", "YEAR", "PRICE", "ISBN", "STOCK"};
    table->setHorizontalHeaderLabels(headers);

    // Cột 0: Ảnh + tiêu đề
    table->setColumnWidth(0, 280);
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

    // Ẩn cột 1 (title ẩn)
    table->hideColumn(1);

    table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    for(int i = 3; i < 7; i++) {
        table->horizontalHeader()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
    }

    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(false);
    table->verticalHeader()->setVisible(false);
    table->setShowGrid(false);
    table->setFocusPolicy(Qt::NoFocus);

    // Chiều cao hàng mặc định
    table->verticalHeader()->setDefaultSectionSize(65);
    table->setIconSize(QSize(40, 55));

    table->setStyleSheet(
        "QTableWidget { "
        "   background-color: rgba(59, 66, 82, 0.9); "
        "   border: none; "
        "   border-radius: 15px; "
        "   gridline-color: transparent; "
        "   padding: 10px; "
        "} "
        "QTableWidget::item { "
        "   padding: 5px; "
        "   border-bottom: 1px solid #4C566A; "
        "} "
        "QTableWidget::item:selected { "
        "   background-color: #4C566A; "
        "   color: #88C0D0; "
        "} "
        "QHeaderView::section { "
        "   background-color: rgba(30, 35, 50, 0.9); "
        "   color: #00d2ff; "
        "   font-weight: bold; "
        "   border: none; "
        "   padding: 10px; "
        "   border-bottom: 2px solid #00d2ff; "
        "}"
        );
}

QString MainWindow::downloadUrlToLocal(const QString &urlStr) {
    if (urlStr.isEmpty()) return "";

    // Hash URL để tạo tên file duy nhất (cache)
    uint hash = qHash(urlStr);
    QString fileName = "covers/img_" + QString::number(hash) + ".jpg";

    // Nếu file đã tồn tại, trả về ngay (tránh tải lại)
    if (QFile::exists(fileName)) {
        return fileName;
    }

    // Tạo thư mục covers nếu chưa có
    QDir dir;
    if (!dir.exists("covers")) {
        dir.mkpath("covers");
    }

    QUrl url(urlStr);
    if (!url.isValid()) {
        qWarning() << "Invalid URL:" << urlStr;
        return "";
    }

    QNetworkAccessManager manager;
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0");

    QNetworkReply *reply = manager.get(request);

    // Dùng EventLoop để chờ tải xong (synchronous)
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QString result = "";

    // Kiểm tra lỗi tải
    if (reply->error() == QNetworkReply::NoError) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(reply->readAll());
            file.close();
            result = fileName;
            qDebug() << "Downloaded:" << fileName;
        } else {
            qWarning() << "Cannot write file:" << fileName;
        }
    } else {
        qWarning() << "Download error:" << reply->errorString();
    }

    delete reply;
    return result;
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), currentUser(nullptr) {
    // --- MODERN DASHBOARD THEME (NORD PALETTE) ---
    QString styleSheet = R"(
        /* Nền tổng thể */
        QMainWindow {
            /* Sửa đường dẫn ảnh nền của bạn tại đây */
            border-image: url("D:/Project_library_managment_system/LibraryManagementSystem/wdesktop wallpaper.jpg") 0 0 0 0 stretch stretch;
        }

        QWidget {
            font-family: 'Segoe UI', sans-serif;
            font-size: 14px;
            color: #D8DEE9; /* Nord White */
        }

        /* 1. Sidebar (Menu Trái) */
        QWidget#Sidebar {
            background-color: rgba(46, 52, 64, 0.95); /* Nord Dark Grey - Mờ nhẹ */
            border-radius: 0px 15px 15px 0px; /* Bo tròn góc phải */
            border-right: 1px solid #4C566A;
        }

        /* Nút Menu */
        QPushButton {
            background-color: transparent;
            color: #D8DEE9;
            text-align: left;
            padding: 12px 20px;
            border-radius: 8px;
            font-weight: 600;
            border: none;
            margin-bottom: 5px;
        }
        QPushButton:hover {
            background-color: #4C566A; /* Sáng hơn khi hover */
            color: #88C0D0; /* Chữ xanh dương nhạt */
            padding-left: 25px; /* Hiệu ứng trượt nhẹ sang phải */
        }
        QPushButton:pressed {
            background-color: #434C5E;
        }

        /* Nút Logout */
        QPushButton#logoutBtn {
            color: #BF616A; /* Màu đỏ nhạt */
            background-color: rgba(191, 97, 106, 0.1);
            margin-top: 20px;
            text-align: center;
        }
        QPushButton#logoutBtn:hover {
            background-color: #BF616A;
            color: white;
        }

        /* 2. Content Area (Khu vực chính) */
        /* Card thống kê */
        QGroupBox {
            background-color: rgba(59, 66, 82, 0.9);
            border: none;
            border-radius: 15px;
        }
        QLabel#statNum { font-size: 36px; font-weight: bold; color: #A3BE8C; /* Xanh lá */ }
        QLabel#statTitle { color: #81A1C1; font-weight: bold; font-size: 12px; letter-spacing: 1px; }

        /* 3. Bảng dữ liệu (Siêu đẹp) */
        QTableWidget {
            background-color: rgba(59, 66, 82, 0.9);
            border: none;
            border-radius: 15px;
            gridline-color: transparent;
            padding: 10px;
        }
        QTableWidget::item {
            border-bottom: 1px solid #4C566A; /* Chỉ kẻ ngang mờ */
            padding-left: 5px;
        }
        QTableWidget::item:selected {
            background-color: #4C566A;
            color: #88C0D0;
        }

        /* 4. Ô tìm kiếm */
        QLineEdit {
            background-color: rgba(67, 76, 94, 0.9);
            border: none;
            border-radius: 20px;
            padding: 10px 20px;
            color: white;
        }
        QLineEdit:focus {
            background-color: #4C566A;
            border: 1px solid #88C0D0;
        }

        /* 5. Login Screen */
        QWidget#LoginCard {
            background-color: rgba(46, 52, 64, 0.95);
            border-radius: 20px;
            border: 1px solid #4C566A;
        }
        QPushButton#LoginBtn {
            background-color: #5E81AC; /* Xanh Nord */
            color: white;
            text-align: center;
            border-radius: 20px;
            padding: 12px;
        }
        QPushButton#LoginBtn:hover { background-color: #81A1C1; }
    )";
    this->setStyleSheet(styleSheet);

    authSystem = new AuthSystem();
    library = new Library();
    library->loadBorrowRecords();
    networkManager = new QNetworkAccessManager(this);
    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);

    // ✅ KHỞI TẠO THƯMỤC
    initializeCoverDirectory();

    setupLoginPage();
    setupAdminPage();
    setupUserPage();

    setWindowTitle("Library Manager - Professional Dashboard");
    resize(1280, 800);
    stackedWidget->setCurrentIndex(0);
}

// --- 3. GIAO DIỆN LOGIN ---

void MainWindow::setupLoginPage() {
    QWidget *loginWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(loginWidget);
    layout->setAlignment(Qt::AlignCenter);

    // Card Login
    QWidget *card = new QWidget();
    card->setObjectName("LoginCard");
    card->setFixedWidth(400);
    applyShadow(card, 40, 10); // Bóng đổ đậm

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setSpacing(20);
    cardLayout->setContentsMargins(50, 60, 50, 60);

    QLabel *title = new QLabel("WELCOME BACK");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 24px; font-weight: bold; color: #ECEFF4;");

    QLabel *sub = new QLabel("Library Management System");
    sub->setAlignment(Qt::AlignCenter);
    sub->setStyleSheet("color: #D8DEE9; margin-bottom: 20px; font-style: italic;");

    usernameInput = new QLineEdit(); usernameInput->setPlaceholderText("Username");
    passwordInput = new QLineEdit(); passwordInput->setPlaceholderText("Password");
    passwordInput->setEchoMode(QLineEdit::Password);

    QPushButton *loginBtn = new QPushButton("Login");
    loginBtn->setObjectName("LoginBtn");
    loginBtn->setCursor(Qt::PointingHandCursor);
    //khi đang gõ username hoặc password
    connect(usernameInput, &QLineEdit::returnPressed, this, &MainWindow::onLoginClicked);
    connect(passwordInput, &QLineEdit::returnPressed, this, &MainWindow::onLoginClicked);
    connect(loginBtn, &QPushButton::clicked, this, &MainWindow::onLoginClicked);

    cardLayout->addWidget(title);
    cardLayout->addWidget(sub);
    cardLayout->addWidget(usernameInput);
    cardLayout->addWidget(passwordInput);
    cardLayout->addWidget(loginBtn);

    layout->addWidget(card);
    stackedWidget->addWidget(loginWidget);
}

// --- 4. GIAO DIỆN ADMIN (DASHBOARD STYLE) ---

void MainWindow::setupAdminPage() {
    QWidget *pageWidget = new QWidget();
    QHBoxLayout *mainLayout = new QHBoxLayout(pageWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0); // Full screen
    mainLayout->setSpacing(0);

    // --- LEFT SIDEBAR ---
    QWidget *sidebar = new QWidget();
    sidebar->setObjectName("Sidebar");
    sidebar->setFixedWidth(260);
    applyShadow(sidebar, 20, 0); // Bóng đổ sang phải

    QVBoxLayout *menuLayout = new QVBoxLayout(sidebar);
    menuLayout->setContentsMargins(15, 40, 15, 30);
    menuLayout->setSpacing(10);

    QLabel *logo = new QLabel(" ADMIN PANEL");
    logo->setStyleSheet("font-size: 22px; font-weight: 800; color: #88C0D0; margin-bottom: 40px; border-left: 5px solid #88C0D0; padding-left: 10px;");
    menuLayout->addWidget(logo);

    // Menu Buttons
    QPushButton *btn1 = new QPushButton(" Dashboard"); btn1->setIcon(getIcon(QStyle::SP_BrowserReload));
    QPushButton *btn2 = new QPushButton(" New Book"); btn2->setIcon(getIcon(QStyle::SP_FileDialogNewFolder));
    QPushButton *btn3 = new QPushButton(" Edit Book"); btn3->setIcon(getIcon(QStyle::SP_FileDialogInfoView));
    QPushButton *btn4 = new QPushButton(" Delete Book"); btn4->setIcon(getIcon(QStyle::SP_TrashIcon));
    QPushButton *btn5 = new QPushButton(" Borrow Records"); btn5->setIcon(getIcon(QStyle::SP_FileDialogDetailedView));
    QPushButton *btn6 = new QPushButton(" Users"); btn6->setIcon(getIcon(QStyle::SP_ComputerIcon));

    connect(btn1, &QPushButton::clicked, this, &MainWindow::onViewBooks);
    connect(btn2, &QPushButton::clicked, this, &MainWindow::onAddBook);
    connect(btn3, &QPushButton::clicked, this, &MainWindow::onEditBook);
    connect(btn4, &QPushButton::clicked, this, &MainWindow::onDeleteBook);
    connect(btn5, &QPushButton::clicked, this, &MainWindow::onManageLoansClicked);
    connect(btn6, &QPushButton::clicked, this, &MainWindow::onAddUserClicked);

    menuLayout->addWidget(btn1); menuLayout->addWidget(btn2); menuLayout->addWidget(btn3);
    menuLayout->addWidget(btn4); menuLayout->addWidget(btn5); menuLayout->addWidget(btn6);
    menuLayout->addStretch(); // Đẩy xuống đáy

    QPushButton *logout = new QPushButton(" Sign Out");
    logout->setObjectName("logoutBtn"); logout->setIcon(getIcon(QStyle::SP_DialogCloseButton));
    connect(logout, &QPushButton::clicked, this, &MainWindow::onLogoutClicked);
    menuLayout->addWidget(logout);

    // --- RIGHT CONTENT ---
    QWidget *content = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(30, 30, 30, 30);
    contentLayout->setSpacing(20);

    QHBoxLayout *searchLayout = new QHBoxLayout();

    adminSearchBar = new QLineEdit();
    adminSearchBar->setPlaceholderText("Search titles, authors, ISBNs...");
    adminSearchBar->setMinimumHeight(45);
    adminSearchBar->setStyleSheet(
        "QLineEdit { "
        "   background-color: rgba(67, 76, 94, 0.9); "
        "   border: 1px solid #4C566A; "
        "   border-radius: 25px; "
        "   padding: 12px 25px; "
        "   color: white; "
        "   font-size: 15px; "
        "} "
        "QLineEdit:focus { "
        "   background-color: #4C566A; "
        "   border: 2px solid #88C0D0; "
        "} "
        );
    connect(adminSearchBar, &QLineEdit::textChanged, this, &MainWindow::onAdminSearchBooks);

    searchLayout->addWidget(adminSearchBar);
    contentLayout->addLayout(searchLayout);

    QHBoxLayout *topBar = new QHBoxLayout();

    // Sort button
    QPushButton *btnSort = new QPushButton(" Sort A-Z");
    btnSort->setMinimumHeight(40);
    btnSort->setMaximumWidth(150);
    btnSort->setStyleSheet(
        "QPushButton { "
        "   background-color: #4C566A; "
        "   border-radius: 20px; "
        "   padding: 10px 20px; "
        "   color: white; "
        "   font-weight: bold; "
        "} "
        "QPushButton:hover { "
        "   background-color: #5E81AC; "
        "} "
        );
    connect(btnSort, &QPushButton::clicked, this, &MainWindow::onSortClicked);

    QLabel *lblStudentInfo = new QLabel(" 24AD11004 | Nguyen Van Linh");
    lblStudentInfo->setStyleSheet(
        "color: #A3BE8C; "
        "font-weight: bold; "
        "font-size: 15px; "
        "padding: 10px 20px; "
        "background-color: rgba(67, 76, 94, 0.7); "
        "border-radius: 15px; "
        );
    lblStudentInfo->setAlignment(Qt::AlignCenter);
    lblStudentInfo->setMaximumWidth(350);
    QLabel *lblStudentInfo1 = new QLabel(" 24TM11003 | Nguyen Chi Thanh");
    lblStudentInfo1->setStyleSheet(
        "color: #A3BE8C; "
        "font-weight: bold; "
        "font-size: 15px; "
        "padding: 10px 20px; "
        "background-color: rgba(67, 76, 94, 0.7); "
        "border-radius: 15px; "
        );
    lblStudentInfo1->setAlignment(Qt::AlignCenter);
    lblStudentInfo1->setMaximumWidth(350);
    QLabel *lblStudentInfo2 = new QLabel(" 23TM11064 | Le Sinh Cong");
    lblStudentInfo2->setStyleSheet(
        "color: #A3BE8C; "
        "font-weight: bold; "
        "font-size: 14px; "
        "padding: 10px 19px; "
        "background-color: rgba(67, 76, 94, 0.7); "
        "border-radius: 14px; "
        );
    lblStudentInfo2->setAlignment(Qt::AlignCenter);
    lblStudentInfo2->setMaximumWidth(350);

    topBar->addWidget(btnSort);
    topBar->addStretch();
    topBar->addWidget(lblStudentInfo);
    topBar->addWidget(lblStudentInfo1);
    topBar->addWidget(lblStudentInfo2);

    // Stats Cards
    QHBoxLayout *statsLayout = new QHBoxLayout();

    QGroupBox *card1 = new QGroupBox();
    QVBoxLayout *v1 = new QVBoxLayout(card1);
    QLabel *l1 = new QLabel("TOTAL STOCK"); l1->setObjectName("statTitle");
    lblTotalBooks = new QLabel("0"); lblTotalBooks->setObjectName("statNum");
    v1->addWidget(l1); v1->addWidget(lblTotalBooks);
    applyShadow(card1);

    QGroupBox *card2 = new QGroupBox();
    QVBoxLayout *v2 = new QVBoxLayout(card2);
    QLabel *l2 = new QLabel("BORROW LISTS"); l2->setObjectName("statTitle");
    lblTotalBorrowed = new QLabel("0"); lblTotalBorrowed->setObjectName("statNum");
    v2->addWidget(l2); v2->addWidget(lblTotalBorrowed);
    applyShadow(card2);

    statsLayout->addWidget(card1);
    statsLayout->addWidget(card2);

    // Main Table
    adminTable = new QTableWidget();
    setupTableStyle(adminTable);
    applyShadow(adminTable);

    // Assemble Right Content
    contentLayout->addLayout(topBar);
    contentLayout->addLayout(statsLayout);
    contentLayout->addWidget(adminTable);

    // Assemble Main Page
    mainLayout->addWidget(sidebar);
    mainLayout->addWidget(content);

    stackedWidget->addWidget(pageWidget);
}

// --- 5. GIAO DIỆN USER (DASHBOARD STYLE) ---

// ========== FIX setupUserPage() ==========
void MainWindow::setupUserPage() {
    QWidget *pageWidget = new QWidget();
    QHBoxLayout *mainLayout = new QHBoxLayout(pageWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Sidebar
    QWidget *sidebar = new QWidget();
    sidebar->setObjectName("Sidebar");
    sidebar->setFixedWidth(260);
    applyShadow(sidebar, 20, 0);

    QVBoxLayout *menuLayout = new QVBoxLayout(sidebar);
    menuLayout->setContentsMargins(15, 40, 15, 30);
    menuLayout->setSpacing(10);

    QLabel *logo = new QLabel(" LIBRARY USER");
    logo->setStyleSheet("font-size: 22px; font-weight: 800; color: #A3BE8C; margin-bottom: 40px; border-left: 5px solid #A3BE8C; padding-left: 10px;");
    menuLayout->addWidget(logo);

    QPushButton *btn1 = new QPushButton(" Browse Library"); btn1->setIcon(getIcon(QStyle::SP_BrowserReload));
    QPushButton *btn2 = new QPushButton(" Borrow Book"); btn2->setIcon(getIcon(QStyle::SP_DialogOkButton));
    QPushButton *btn3 = new QPushButton(" My Borrow Lists"); btn3->setIcon(getIcon(QStyle::SP_FileDialogDetailedView));
    QPushButton *btn4 = new QPushButton(" Change Pass"); btn4->setIcon(getIcon(QStyle::SP_ComputerIcon));
    QPushButton *btnPenalties = new QPushButton("My Penalties");
    btnPenalties->setIcon(getIcon(QStyle::SP_MessageBoxWarning));

    connect(btn1, &QPushButton::clicked, this, &MainWindow::onViewBooks);
    connect(btn2, &QPushButton::clicked, this, &MainWindow::onBorrowBook);
    connect(btn3, &QPushButton::clicked, this, &MainWindow::onViewMyBooks);
    connect(btn4, &QPushButton::clicked, this, &MainWindow::onChangePasswordClicked);
    connect(btnPenalties, &QPushButton::clicked, this, &MainWindow::onViewMyPenalties);

    menuLayout->addWidget(btn1); menuLayout->addWidget(btn2);
    menuLayout->addWidget(btn3); menuLayout->addWidget(btn4);
    menuLayout->addWidget(btnPenalties);
    menuLayout->addStretch();

    QPushButton *logout = new QPushButton(" Sign Out");
    logout->setObjectName("logoutBtn"); logout->setIcon(getIcon(QStyle::SP_DialogCloseButton));
    connect(logout, &QPushButton::clicked, this, &MainWindow::onLogoutClicked);
    menuLayout->addWidget(logout);

    // Content
    QWidget *content = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(30, 30, 30, 30);
    contentLayout->setSpacing(15);

    // ============ DÒNG 1: SEARCH BAR ============
    QHBoxLayout *searchLayout = new QHBoxLayout();

    searchBar = new QLineEdit();  // ✅ ĐÚNG - dùng searchBar
    searchBar->setPlaceholderText("Search titles, authors, ISBNs...");
    searchBar->setMinimumHeight(45);
    searchBar->setStyleSheet(
        "QLineEdit { "
        "   background-color: rgba(67, 76, 94, 0.9); "
        "   border: 1px solid #4C566A; "
        "   border-radius: 25px; "
        "   padding: 12px 25px; "
        "   color: white; "
        "   font-size: 15px; "
        "} "
        "QLineEdit:focus { "
        "   background-color: #4C566A; "
        "   border: 2px solid #88C0D0; "
        "} "
        );
    connect(searchBar, &QLineEdit::textChanged, this, &MainWindow::onSearchBooks);  // ✅ ĐÚNG

    searchLayout->addWidget(searchBar);
    contentLayout->addLayout(searchLayout);

    // ============ DÒNG 2: SORT + STUDENT INFO ============
    QHBoxLayout *topBar = new QHBoxLayout();

    QPushButton *btnSort = new QPushButton(" Sort A-Z");
    btnSort->setMinimumHeight(40);
    btnSort->setMaximumWidth(150);
    btnSort->setStyleSheet(
        "QPushButton { "
        "   background-color: #4C566A; "
        "   border-radius: 20px; "
        "   padding: 10px 20px; "
        "   color: white; "
        "   font-weight: bold; "
        "} "
        "QPushButton:hover { "
        "   background-color: #5E81AC; "
        "} "
        );
    connect(btnSort, &QPushButton::clicked, this, &MainWindow::onSortClicked);

    QLabel *lblStudentInfo = new QLabel(" 24AD11004 | Nguyen Van Linh");
    lblStudentInfo->setStyleSheet(
        "color: #A3BE8C; "
        "font-weight: bold; "
        "font-size: 14px; "
        "padding: 8px 16px; "
        "background-color: rgba(67, 76, 94, 0.7); "
        "border-radius: 12px; "
        );
    lblStudentInfo->setAlignment(Qt::AlignCenter);
    lblStudentInfo->setMaximumWidth(280);

    QLabel *lblStudentInfo1 = new QLabel(" 24TM11003 | Nguyen Chi Thanh");
    lblStudentInfo1->setStyleSheet(
        "color: #A3BE8C; "
        "font-weight: bold; "
        "font-size: 14px; "
        "padding: 8px 16px; "
        "background-color: rgba(67, 76, 94, 0.7); "
        "border-radius: 12px; "
        );
    lblStudentInfo1->setAlignment(Qt::AlignCenter);
    lblStudentInfo1->setMaximumWidth(280);

    QLabel *lblStudentInfo2 = new QLabel(" 23TM11064 | Le Sinh Cong");
    lblStudentInfo2->setStyleSheet(
        "color: #A3BE8C; "
        "font-weight: bold; "
        "font-size: 14px; "
        "padding: 8px 16px; "
        "background-color: rgba(67, 76, 94, 0.7); "
        "border-radius: 12px; "
        );
    lblStudentInfo2->setAlignment(Qt::AlignCenter);
    lblStudentInfo2->setMaximumWidth(280);

    topBar->addWidget(btnSort);
    topBar->addStretch();
    topBar->addWidget(lblStudentInfo);
    topBar->addWidget(lblStudentInfo1);
    topBar->addWidget(lblStudentInfo2);
    contentLayout->addLayout(topBar);

    // ============ TABLE ============
    userTable = new QTableWidget();
    setupTableStyle(userTable);
    applyShadow(userTable);

    contentLayout->addWidget(userTable);

    mainLayout->addWidget(sidebar);
    mainLayout->addWidget(content);

    stackedWidget->addWidget(pageWidget);
}
// --- 6. LOGIC FUNCTIONS (GIỮ NGUYÊN) ---

void MainWindow::onLoginClicked() {
    QString u = usernameInput->text();
    QString p = passwordInput->text();
    currentUser = authSystem->login(u.toStdString(), p.toStdString());

    if (currentUser) {
        library->docFile("InputBooks.txt");
        if (currentUser->getRole() == "admin") stackedWidget->setCurrentIndex(1);
        else stackedWidget->setCurrentIndex(2);
        onViewBooks();
    } else {
        QMessageBox::warning(this, "Error", "Wrong Username or Password!");
    }
}

void MainWindow::onLogoutClicked() {
    if (currentUser) delete currentUser;
    currentUser = nullptr;
    usernameInput->clear(); passwordInput->clear();
    stackedWidget->setCurrentIndex(0);
}

void MainWindow::refreshDashboardStats() {
    int totalStock = library->getTotalStock();
    int borrowed = library->getAllBorrowRecords().size();
    lblTotalBooks->setText(QString::number(totalStock));
    lblTotalBorrowed->setText(QString::number(borrowed));
}
void MainWindow::updateTableData(QTableWidget* table, const vector<Book>& books) {
    if(!table) return;

    table->blockSignals(true);
    table->setRowCount(0);
    table->setIconSize(QSize(40, 55));

    for (const auto& b : books) {
        int r = table->rowCount();
        table->insertRow(r);

        QString imgPath = QString::fromStdString(b.getImagePath());
        QIcon bookIcon;

        if (imgPath.startsWith("http://") || imgPath.startsWith("https://")) {
            imgPath = downloadAndCacheImage(imgPath);
        }

        if (!imgPath.isEmpty() && QFile::exists(imgPath)) {
            QPixmap pixmap(imgPath);
            if (!pixmap.isNull()) {
                bookIcon = QIcon(pixmap.scaledToHeight(55, Qt::SmoothTransformation));
            }
        } else {
            bookIcon = getIcon(QStyle::SP_FileIcon);
        }

        QTableWidgetItem *title = new QTableWidgetItem(bookIcon,
                                                       QString::fromStdString(b.getTitle()));
        title->setFont(QFont("Arial", 10, QFont::Bold));

        QTableWidgetItem *author = new QTableWidgetItem(QString::fromStdString(b.getAuthor()));
        QTableWidgetItem *year = new QTableWidgetItem(QString::number(b.getYear()));
        QTableWidgetItem *price = new QTableWidgetItem("$" +
                                                       QString::number(b.getPrice(), 'f', 2));
        price->setForeground(QBrush(QColor("#2ecc71")));

        QTableWidgetItem *isbn = new QTableWidgetItem(QString::fromStdString(b.getISBN()));

        // ✅ NEW: Display both stocks: "Available/Total"
        QTableWidgetItem *stock = new QTableWidgetItem(
            QString::number(b.getStockAvailable()) + "/" +
            QString::number(b.getStockInputted()));

        if(b.getStockAvailable() == 0) {
            stock->setForeground(QBrush(QColor("#e74c3c")));
            stock->setText("0/" + QString::number(b.getStockInputted()) + " (All Borrowed)");
        } else {
            stock->setForeground(QBrush(QColor("#00d2ff")));
        }

        table->setItem(r, 0, title);
        table->setItem(r, 1, new QTableWidgetItem(QString::fromStdString(b.getTitle())));
        table->setItem(r, 2, author);
        table->setItem(r, 3, year);
        table->setItem(r, 4, price);
        table->setItem(r, 5, isbn);
        table->setItem(r, 6, stock);

        table->setRowHeight(r, 65);
    }

    table->blockSignals(false);
}
void MainWindow::onViewBooks() {
    vector<Book> books = library->getBooks();
    if(adminSearchBar && adminSearchBar->text().isEmpty()) updateTableData(adminTable, books);
    if(searchBar && searchBar->text().isEmpty()) updateTableData(userTable, books);
    refreshDashboardStats();
}

void MainWindow::onSearchBooks(const QString &text) {
    if(text.isEmpty()) updateTableData(userTable, library->getBooks());
    else updateTableData(userTable, library->searchBooks(text.toStdString()));
}

void MainWindow::onAdminSearchBooks(const QString &text) {
    if(text.isEmpty()) updateTableData(adminTable, library->getBooks());
    else updateTableData(adminTable, library->searchBooks(text.toStdString()));
}

void MainWindow::onAddBook() {
    QDialog d(this);
    d.setWindowTitle("Add Book");
    d.setStyleSheet("background-color: #2b2d3a; color: white;");

    QFormLayout f(&d);
    QLineEdit *t = new QLineEdit(&d);
    QLineEdit *a = new QLineEdit(&d);
    QSpinBox *y = new QSpinBox(&d);
    y->setRange(1000, 2030);
    y->setValue(2023);

    QDoubleSpinBox *p = new QDoubleSpinBox(&d);
    p->setRange(0, 10000);

    QLineEdit *i = new QLineEdit(&d);

    // ✅ NEW: Two separate stock inputs
    QSpinBox *stockInputted = new QSpinBox(&d);
    stockInputted->setRange(0, 1000);
    stockInputted->setValue(1);
    stockInputted->setToolTip("Total stock imported to library");

    QSpinBox *stockAvailable = new QSpinBox(&d);
    stockAvailable->setRange(0, 1000);
    stockAvailable->setValue(1);
    stockAvailable->setToolTip("Stock available for borrowing");

    QLineEdit *imgPathInput = new QLineEdit(&d);
    imgPathInput->setPlaceholderText("Paste URL or Browse...");

    QPushButton *btnBrowse = new QPushButton("Browse Image...", &d);
    connect(btnBrowse, &QPushButton::clicked, [imgPathInput, this]() {
        QString fileName = QFileDialog::getOpenFileName(this, "Select Cover",
                                                        "", "Images (*.png *.jpg *.jpeg)");
        if (!fileName.isEmpty()) imgPathInput->setText(fileName);
    });

    QPushButton *btnUrl = new QPushButton("From URL", &d);
    btnUrl->setStyleSheet("background-color: #e67e22;");
    connect(btnUrl, &QPushButton::clicked, [this, imgPathInput]() {
        bool ok;
        QString text = QInputDialog::getText(this, "Image URL",
                                             "Paste image link here:",
                                             QLineEdit::Normal, "", &ok);
        if (ok && !text.isEmpty()) {
            downloadImageFromUrl(text, imgPathInput);
        }
    });

    QHBoxLayout *imgLayout = new QHBoxLayout();
    imgLayout->addWidget(imgPathInput);
    imgLayout->addWidget(btnBrowse);
    imgLayout->addWidget(btnUrl);

    f.addRow("Title:", t);
    f.addRow("Author:", a);
    f.addRow("Year:", y);
    f.addRow("Price:", p);
    f.addRow("ISBN:", i);

    // ✅ NEW: Display two stock types with labels
    f.addRow("Stock Inputted (Total):", stockInputted);
    f.addRow("Stock Available (For Borrow):", stockAvailable);

    f.addRow("Cover Image:", imgLayout);

    QDialogButtonBox b(QDialogButtonBox::Save | QDialogButtonBox::Cancel,
                       Qt::Horizontal, &d);
    f.addRow(&b);

    connect(&b, &QDialogButtonBox::accepted, &d, &QDialog::accept);
    connect(&b, &QDialogButtonBox::rejected, &d, &QDialog::reject);

    if (d.exec() == QDialog::Accepted) {
        if(t->text().isEmpty() || i->text().isEmpty()) {
            QMessageBox::warning(this, "Error", "Missing Info!");
            return;
        }

        // ✅ NEW: Pass both stocks
        library->addNewBook(t->text().toStdString(),
                            a->text().toStdString(),
                            y->value(), p->value(),
                            i->text().toStdString(),
                            stockInputted->value(),      // Stock inputted
                            stockAvailable->value(),     // Stock available
                            imgPathInput->text().toStdString());

        onViewBooks();
        QMessageBox::information(this, "Success", "Book added!");
    }
}
void MainWindow::onEditBook() {
    int row = adminTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Warning", "Select book!");
        return;
    }

    string oldTitle = adminTable->item(row, 1)->text().toStdString();
    string oldAuthor = adminTable->item(row, 2)->text().toStdString();
    int oldYear = adminTable->item(row, 3)->text().toInt();
    float oldPrice = adminTable->item(row, 4)->text().replace("$", "").toFloat();
    string isbn = adminTable->item(row, 5)->text().toStdString();

    // ✅ NEW: Get both stock values from table
    QString stockDisplay = adminTable->item(row, 6)->text();
    int oldStockInputted = 0;
    int oldStockAvailable = 0;

    // Parse display format: "Available/Total"
    QStringList parts = stockDisplay.split("/");
    if (parts.size() == 2) {
        oldStockAvailable = parts[0].toInt();
        oldStockInputted = parts[1].toInt();
    }

    QDialog d(this);
    d.setWindowTitle("Edit Book");
    QFormLayout f(&d);

    QLineEdit *t = new QLineEdit(&d);
    t->setText(QString::fromStdString(oldTitle));

    QLineEdit *a = new QLineEdit(&d);
    a->setText(QString::fromStdString(oldAuthor));

    QSpinBox *y = new QSpinBox(&d);
    y->setRange(1000, 2030);
    y->setValue(oldYear);

    QDoubleSpinBox *p = new QDoubleSpinBox(&d);
    p->setRange(0, 10000);
    p->setValue(oldPrice);

    QLabel *iLabel = new QLabel(QString::fromStdString(isbn));

    // ✅ NEW: Edit both stocks
    QSpinBox *stockInputted = new QSpinBox(&d);
    stockInputted->setRange(0, 1000);
    stockInputted->setValue(oldStockInputted);
    stockInputted->setToolTip("Total stock imported to library");

    QSpinBox *stockAvailable = new QSpinBox(&d);
    stockAvailable->setRange(0, 1000);
    stockAvailable->setValue(oldStockAvailable);
    stockAvailable->setToolTip("Stock available for borrowing");
    stockAvailable->setMaximum(oldStockInputted);  // Can't exceed inputted

    QLineEdit *imgPathInput = new QLineEdit(&d);
    imgPathInput->setPlaceholderText("Paste URL or Browse...");

    QPushButton *btnBrowse = new QPushButton("Change Cover...", &d);
    connect(btnBrowse, &QPushButton::clicked, [imgPathInput, this]() {
        QString fileName = QFileDialog::getOpenFileName(this, "Select Cover",
                                                        "", "Images (*.png *.jpg *.jpeg)");
        if (!fileName.isEmpty()) imgPathInput->setText(fileName);
    });

    QPushButton *btnUrl = new QPushButton("From URL", &d);
    btnUrl->setStyleSheet("background-color: #e67e22;");
    connect(btnUrl, &QPushButton::clicked, [this, imgPathInput]() {
        bool ok;
        QString text = QInputDialog::getText(this, "Image URL",
                                             "Paste image link here:",
                                             QLineEdit::Normal, "", &ok);
        if (ok && !text.isEmpty()) {
            downloadImageFromUrl(text, imgPathInput);
        }
    });

    QHBoxLayout *imgLayout = new QHBoxLayout();
    imgLayout->addWidget(imgPathInput);
    imgLayout->addWidget(btnBrowse);
    imgLayout->addWidget(btnUrl);

    f.addRow("Title:", t);
    f.addRow("Author:", a);
    f.addRow("Year:", y);
    f.addRow("Price:", p);
    f.addRow("ISBN:", iLabel);

    // ✅ NEW: Edit two stocks
    f.addRow("Stock Inputted (Total):", stockInputted);
    f.addRow("Stock Available (For Borrow):", stockAvailable);

    f.addRow("Cover Image:", imgLayout);

    QDialogButtonBox b(QDialogButtonBox::Save | QDialogButtonBox::Cancel,
                       Qt::Horizontal, &d);
    f.addRow(&b);

    connect(&b, &QDialogButtonBox::accepted, &d, &QDialog::accept);
    connect(&b, &QDialogButtonBox::rejected, &d, &QDialog::reject);

    if (d.exec() == QDialog::Accepted) {
        // ✅ NEW: Pass both stocks
        library->updateBook(isbn,
                            t->text().toStdString(),
                            a->text().toStdString(),
                            y->value(), p->value(),
                            stockInputted->value(),      // Stock inputted
                            stockAvailable->value(),     // Stock available
                            imgPathInput->text().toStdString());

        onViewBooks();
        QMessageBox::information(this, "Success", "Book updated!");
    }
}

void MainWindow::onDeleteBook() {
    int row = adminTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Warning", "Select book first!");
        return;
    }

    QString isbn = adminTable->item(row, 5)->text();

    // ✅ NEW: Check stock status before deletion
    // Get the book to check if all are returned
    vector<Book> books = library->getBooks();
    int stockInputted = 0;
    int stockAvailable = 0;

    for(const auto& b : books) {
        if(b.getISBN() == isbn.toStdString()) {
            stockInputted = b.getStockInputted();
            stockAvailable = b.getStockAvailable();
            break;
        }
    }

    // ✅ Check: Can only delete if all books returned
    if (stockAvailable != stockInputted) {
        int borrowed = stockInputted - stockAvailable;
        QMessageBox::warning(this, "Cannot Delete",
                             QString("Cannot delete this book!\n\n") +
                                 QString::number(borrowed) + " book(s) still borrowed.\n" +
                                 "Please wait for all books to be returned.");
        return;
    }

    if (QMessageBox::question(this, "Delete",
                              "Delete this book?",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {

        if (library->deleteBook(isbn.toStdString())) {
            onViewBooks();
            QMessageBox::information(this, "Success", "Book deleted!");
        } else {
            QMessageBox::warning(this, "Error", "Cannot delete this book!");
        }
    }
}

void MainWindow::onManageLoansClicked() {
    QDialog d(this);
    d.setWindowTitle("Loan Management & Penalties");
    d.resize(950, 600);
    d.setStyleSheet("background-color: #2E3440; color: white;");

    QVBoxLayout *mainLayout = new QVBoxLayout(&d);

    // ========== TAB 1: ACTIVE LOANS ==========
    QTabWidget *tabs = new QTabWidget(&d);

    QWidget *loansTab = new QWidget();
    QVBoxLayout *loansLayout = new QVBoxLayout(loansTab);

    QTableWidget *loansTable = new QTableWidget();
    loansTable->setColumnCount(6);
    loansTable->setHorizontalHeaderLabels({"User", "ISBN", "Title", "Due Date", "Status", "Action"});
    loansTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    loansTable->setStyleSheet("background-color: #3B4252; gridline-color: transparent; border: none;");
    loansTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    loansTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    loansTable->verticalHeader()->setVisible(false);

    vector<BorrowRecord> recs = library->getAllBorrowRecords();
    loansTable->setRowCount(0);
    QDate today = QDate::currentDate();

    for(const auto& r : recs) {
        int row = loansTable->rowCount();
        loansTable->insertRow(row);

        loansTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(r.getUsername())));
        loansTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(r.getISBN())));
        loansTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(library->getBookTitleByISBN(r.getISBN()))));
        loansTable->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(r.getDueDate())));

        QDate due = QDate::fromString(QString::fromStdString(r.getDueDate()), "d/M/yyyy");
        QTableWidgetItem *status;

        if(today > due) {
            int daysOverdue = due.daysTo(today);
            status = new QTableWidgetItem(QString("OVERDUE! (%1 days)").arg(daysOverdue));
            status->setForeground(QBrush(QColor("#e74c3c")));
        } else {
            int daysLeft = today.daysTo(due);
            status = new QTableWidgetItem(QString("On Time (%1 days left)").arg(daysLeft));
            status->setForeground(QBrush(QColor("#A3BE8C")));
        }
        loansTable->setItem(row, 4, status);
    }

    // ✅ NÚT AUTO SCAN & FINE (Đã sửa lỗi thiếu ngoặc)
    QPushButton *btnAutoScan = new QPushButton("⚡ AUTO SCAN & FINE OVERDUE");
    btnAutoScan->setStyleSheet("background-color: #BF616A; color: white; font-weight: bold; padding: 8px;");

    connect(btnAutoScan, &QPushButton::clicked, [this, &d]() {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(&d, "Auto Scan",
                                      "Do you want to scan and discard all overdue books immediately?\n"
                                      "This action will return the books to the library and generate a penalty slip.",
                                      QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            int count = library->autoProcessOverdueBooks();

            if (count > 0) {
                QMessageBox::information(&d, "Success",
                                         "Processing completed " + QString::number(count) + " Overdue case");
                d.accept();
                onManageLoansClicked();
            } else {
                QMessageBox::information(&d, "Info", "There are no books today.");
            }
        } // ✅ ĐÃ CÓ DẤU ĐÓNG NGOẶC NÀY
    });

    loansLayout->addWidget(btnAutoScan);
    loansLayout->addWidget(loansTable);
    tabs->addTab(loansTab, "📖 Active Loans");

    // ========== TAB 2: PENALTIES ==========
    QWidget *penaltiesTab = new QWidget();
    QVBoxLayout *penaltiesLayout = new QVBoxLayout(penaltiesTab);

    QTableWidget *penaltiesTable = new QTableWidget();
    penaltiesTable->setColumnCount(7);
    penaltiesTable->setHorizontalHeaderLabels({
        "User", "ISBN", "Book Title", "Days Overdue",
        "Penalty ($)", "Status", "Action"
    });
    penaltiesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    penaltiesTable->setStyleSheet("background-color: #3B4252; gridline-color: transparent; border: none;");
    penaltiesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    penaltiesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    penaltiesTable->verticalHeader()->setVisible(false);

    vector<Penalty> unpaidPenalties = library->getUnpaidPenalties();
    penaltiesTable->setRowCount(0);

    for(const auto& p : unpaidPenalties) {
        int row = penaltiesTable->rowCount();
        penaltiesTable->insertRow(row);

        penaltiesTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(p.getUsername())));
        penaltiesTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(p.getISBN())));
        penaltiesTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(p.getBookTitle())));
        penaltiesTable->setItem(row, 3, new QTableWidgetItem(QString::number(p.getDaysOverDue())));

        QTableWidgetItem *penaltyItem = new QTableWidgetItem("$" + QString::number(p.getPenaltyAmount(), 'f', 2));
        penaltyItem->setForeground(QBrush(QColor("#e74c3c")));
        penaltiesTable->setItem(row, 4, penaltyItem);

        QTableWidgetItem *statusItem = new QTableWidgetItem("❌ Unpaid");
        statusItem->setForeground(QBrush(QColor("#BF616A")));
        penaltiesTable->setItem(row, 5, statusItem);

        QPushButton *btnPayPenalty = new QPushButton("Pay Penalty");
        btnPayPenalty->setStyleSheet("background-color: #A3BE8C; color: black;");

        // Copy giá trị cần thiết vào lambda để tránh lỗi truy cập vùng nhớ
        string uName = p.getUsername();
        string uISBN = p.getISBN();
        float uAmount = p.getPenaltyAmount();

        connect(btnPayPenalty, &QPushButton::clicked, [this, &d, uName, uISBN, uAmount]() {
            if(QMessageBox::question(&d, "Confirm Payment",
                                      QString("Pay penalty of $%1 for user %2?")
                                          .arg(uAmount).arg(QString::fromStdString(uName)),
                                      QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {

                if(library->payPenalty(uName, uISBN)) {
                    QMessageBox::information(&d, "Success", "Penalty paid!");
                    d.accept();
                    onManageLoansClicked(); // Refresh lại
                }
            }
        });

        penaltiesTable->setCellWidget(row, 6, btnPayPenalty);
    }

    penaltiesLayout->addWidget(new QLabel("⚠️  Unpaid Penalties:"));
    penaltiesLayout->addWidget(penaltiesTable);

    // Show summary
    float totalPenalties = 0;
    for(const auto& p : unpaidPenalties) totalPenalties += p.getPenaltyAmount();

    QLabel *summaryLabel = new QLabel(QString("Total Unpaid Penalties: $%1").arg(totalPenalties, 0, 'f', 2));
    summaryLabel->setStyleSheet("color: #e74c3c; font-weight: bold; font-size: 14px;");
    penaltiesLayout->addWidget(summaryLabel);

    tabs->addTab(penaltiesTab, "⚠️  Penalties");

    mainLayout->addWidget(tabs);
    d.exec();
}

void MainWindow::onAddUserClicked() {
    QDialog d(this); d.setWindowTitle("New Account");
    QFormLayout f(&d);
    QLineEdit *u = new QLineEdit(&d); QLineEdit *p = new QLineEdit(&d); p->setEchoMode(QLineEdit::Password);
    QComboBox *r = new QComboBox(&d); r->addItem("user"); r->addItem("admin");
    f.addRow("User:", u); f.addRow("Pass:", p); f.addRow("Role:", r);
    QDialogButtonBox b(QDialogButtonBox::Save, Qt::Horizontal, &d);
    connect(&b, &QDialogButtonBox::accepted, &d, &QDialog::accept);
    f.addRow(&b);
    if(d.exec() == QDialog::Accepted) {
        if(u->text().isEmpty() || p->text().isEmpty()) return;
        if(authSystem->addUser(u->text().toStdString(), p->text().toStdString(), r->currentText().toStdString()))
            QMessageBox::information(this, "OK", "User created!");
        else QMessageBox::warning(this, "Error", "User exists!");
    }
}

void MainWindow::onChangePasswordClicked() {
    QDialog d(this); d.setWindowTitle("Change Password");
    QFormLayout f(&d);
    QLineEdit *oldP = new QLineEdit(&d); oldP->setEchoMode(QLineEdit::Password);
    QLineEdit *newP = new QLineEdit(&d); newP->setEchoMode(QLineEdit::Password);
    f.addRow("Old Pass:", oldP); f.addRow("New Pass:", newP);
    QDialogButtonBox b(QDialogButtonBox::Save, Qt::Horizontal, &d);
    connect(&b, &QDialogButtonBox::accepted, &d, &QDialog::accept);
    f.addRow(&b);

    if(d.exec() == QDialog::Accepted) {
        if(authSystem->changePassword(currentUser->getUsername(), oldP->text().toStdString(), newP->text().toStdString()))
            QMessageBox::information(this, "Success", "Password changed!");
        else
            QMessageBox::warning(this, "Error", "Wrong old password!");
    }
}

void MainWindow::onBorrowBook() {
    if (!currentUser) return;
    int currentRow = userTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Warning", "Please select a book from the list to borrow.");
        return;
    }
    QString title = userTable->item(currentRow, 1)->text();
    QString isbn = userTable->item(currentRow, 5)->text();

    if (QMessageBox::question(this, "Confirm Borrow",
                              "Do you want to borrow '" + title + "'?\n(Due date: 14 days)",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        string result = library->borrowBook(currentUser->getUsername(), isbn.toStdString());
        if (result == "OK") {
            QMessageBox::information(this, "Success", "You borrowed '" + title + "' successfully!");
            onViewBooks();
        } else {
            QMessageBox::warning(this, "Failed", QString::fromStdString(result));
        }
    }
}

void MainWindow::onViewMyBooks() {
    if (!currentUser) return;
    QDialog myBooksDialog(this); myBooksDialog.setWindowTitle("My Borrowed Books"); myBooksDialog.resize(700, 450);
    // Dark theme
    myBooksDialog.setStyleSheet("background-color: #2E3440; color: white;");

    QVBoxLayout *layout = new QVBoxLayout(&myBooksDialog);
    QTableWidget *borrowTable = new QTableWidget(&myBooksDialog);
    borrowTable->setColumnCount(4); borrowTable->setHorizontalHeaderLabels({"ISBN", "Title", "Borrow Date", "Due Date"});
    borrowTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    applyShadow(borrowTable);

    borrowTable->setStyleSheet("background-color: #3B4252; gridline-color: transparent; border: none; selection-background-color: #5E81AC;");
    borrowTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    borrowTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    borrowTable->verticalHeader()->setVisible(false);

    vector<BorrowRecord> records = library->getBorrowedBooksByUser(currentUser->getUsername());
    borrowTable->setRowCount(0);
    for (const auto& rec : records) {
        int row = borrowTable->rowCount(); borrowTable->insertRow(row);
        borrowTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(rec.getISBN())));
        borrowTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(library->getBookTitleByISBN(rec.getISBN()))));
        borrowTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(rec.getBorrowDate())));
        borrowTable->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(rec.getDueDate())));
    }
    layout->addWidget(new QLabel("List of books you are currently borrowing:"));
    layout->addWidget(borrowTable);

    QPushButton *returnBtn = new QPushButton(" Return Selected Book", &myBooksDialog);
    returnBtn->setStyleSheet("background-color: #A3BE8C; color: black; font-weight: bold;");
    layout->addWidget(returnBtn);
    connect(returnBtn, &QPushButton::clicked, [&]() {
        int currentRow = borrowTable->currentRow();
        if (currentRow < 0) { QMessageBox::warning(&myBooksDialog, "Warning", "Select a book to return."); return; }
        QString isbn = borrowTable->item(currentRow, 0)->text();
        if (QMessageBox::question(&myBooksDialog, "Return Book", "Do you want to return this book?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            if (library->returnBook(currentUser->getUsername(), isbn.toStdString())) {
                QMessageBox::information(&myBooksDialog, "Success", "Book returned successfully!");
                borrowTable->removeRow(currentRow); onViewBooks();
            } else { QMessageBox::warning(&myBooksDialog, "Error", "Could not return book."); }
        }
    });
    QPushButton *closeBtn = new QPushButton("Close", &myBooksDialog);
    connect(closeBtn, &QPushButton::clicked, &myBooksDialog, &QDialog::accept);
    layout->addWidget(closeBtn);
    myBooksDialog.exec();
}

// Hàm tải ảnh từ URL
void MainWindow::downloadImageFromUrl(const QString &urlStr, QLineEdit *targetInput) {
    QUrl url(urlStr);
    if (!url.isValid()) {
        QMessageBox::warning(this, "Error", "Invalid URL!");
        return;
    }

    QNetworkRequest request(url);
    QNetworkReply *reply = networkManager->get(request);

    // Xử lý khi tải xong (Asynchronous)
    connect(reply, &QNetworkReply::finished, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            // Tạo tên file duy nhất dựa trên thời gian để không bị trùng
            QString fileName = "covers/cover_" + QString::number(QDateTime::currentMSecsSinceEpoch()) + ".jpg";

            // Tạo thư mục covers nếu chưa có
            QDir().mkpath("covers");

            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(reply->readAll());
                file.close();

                // Tự động điền đường dẫn file vào ô nhập liệu
                targetInput->setText(fileName);
                QMessageBox::information(this, "Success", "Image downloaded successfully!");
            }
        } else {
            QMessageBox::warning(this, "Error", "Download failed: " + reply->errorString());
        }
        reply->deleteLater();
    });
}

bool MainWindow::initializeCoverDirectory() {
    QDir dir;

    // Nếu đã tồn tại, return true
    if (dir.exists("covers")) {
        qDebug() << "✅ Thư mục 'covers' đã tồn tại";
        return true;
    }

    // Tạo thư mục mới
    if (dir.mkpath("covers")) {
        qDebug() << "✅ Tạo thư mục 'covers' thành công";
        qDebug() << "📁 Đường dẫn tuyệt đối:" << QDir("covers").absolutePath();
        return true;
    } else {
        qWarning() << "❌ Không thể tạo thư mục 'covers'";
        qWarning() << "📁 Cwd:" << QDir::currentPath();
        return false;
    }
}

QString MainWindow::downloadAndCacheImage(const QString &urlStr) {
    if (urlStr.isEmpty()) {
        qWarning() << "❌ URL rỗng";
        return "";
    }

    qDebug() << "📥 Bắt đầu xử lý URL:" << urlStr;

    // Kiểm tra nếu là URL
    if (!urlStr.startsWith("http://") && !urlStr.startsWith("https://")) {
        // Nếu là đường dẫn file local, kiểm tra tồn tại
        if (QFile::exists(urlStr)) {
            qDebug() << "✅ Dùng file local:" << urlStr;
            return urlStr;
        }
        qWarning() << "❌ File local không tồn tại:" << urlStr;
        return "";
    }

    // ✅ Kiểm tra cache trước
    if (imageCache.contains(urlStr)) {
        QString cachedPath = imageCache[urlStr];
        if (QFile::exists(cachedPath)) {
            qDebug() << "✅ Dùng cache:" << cachedPath;
            return cachedPath;
        }
    }

    // Hash URL để tạo tên file cache
    QString hash = QString::number(qHash(urlStr));
    QString fileName = "covers/cover_" + hash + ".jpg";

    // Nếu file đã tồn tại, trả về ngay
    if (QFile::exists(fileName)) {
        qDebug() << "✅ File tồn tại:" << fileName;
        imageCache[urlStr] = fileName;  // Cập nhật cache
        return fileName;
    }

    // Tạo thư mục nếu chưa có
    if (!initializeCoverDirectory()) {
        qWarning() << "❌ Không thể tạo thư mục covers";
        return "";
    }

    // Tạo URL object
    QUrl url(urlStr);
    if (!url.isValid()) {
        qWarning() << "❌ URL không hợp lệ:" << urlStr;
        return "";
    }

    qDebug() << "📡 Tải từ URL:" << url.toString();

    // Tạo request với User-Agent
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");
    request.setRawHeader("Accept", "*/*");

    // Tạo network manager riêng cho mỗi request
    QNetworkAccessManager *manager = new QNetworkAccessManager();
    QNetworkReply *reply = manager->get(request);

    // Dùng EventLoop để chờ tải xong
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);

    // Timeout 15 giây (tăng lên vì OpenLibrary có thể chậm)
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(15000);

    loop.exec();
    timer.stop();

    QString result = "";

    // 🔍 DEBUG: In ra chi tiết kết quả
    qDebug() << "📊 Network Reply Error Code:" << (int)reply->error();
    qDebug() << "📊 Network Reply Error String:" << reply->errorString();
    qDebug() << "📊 HTTP Status Code:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    // Kiểm tra kết quả tải
    if (reply->error() == QNetworkReply::NoError) {
        qDebug() << "✅ Tải thành công!";

        // Kiểm tra content-type
        QVariant contentTypeVar = reply->header(QNetworkRequest::ContentTypeHeader);
        QString contentType = contentTypeVar.toString();
        qDebug() << "📋 Content-Type:" << contentType;

        // ✅ CẬP NHẬT: Kiểm tra content-type linh hoạt hơn
        bool isImage = contentType.startsWith("image/") ||
                       contentType.contains("jpeg") ||
                       contentType.contains("jpg") ||
                       contentType.isEmpty();  // Nếu empty, cứ thử lưu

        if (!isImage) {
            qWarning() << "⚠️ Content-Type không phải ảnh:" << contentType;
            // Vẫn thử lưu vì OpenLibrary có thể không gửi đúng header
        }

        // Đọc dữ liệu ảnh
        QByteArray imageData = reply->readAll();
        qDebug() << "📦 Kích thước dữ liệu tải:" << imageData.size() << "bytes";

        if (imageData.size() > 100) {  // Ít nhất 100 bytes
            // Kiểm tra header ảnh JPG
            if (imageData.startsWith("\xFF\xD8\xFF")) {
                qDebug() << "✅ Header ảnh JPG hợp lệ";
            }

            // Lưu file
            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly)) {
                qint64 written = file.write(imageData);
                file.close();
                qDebug() << "✅ Lưu file:" << fileName << "(" << written << "bytes)";

                // Kiểm tra file sau khi lưu
                if (QFile::exists(fileName)) {
                    qDebug() << "✅ File tồn tại sau lưu:" << fileName;
                    result = fileName;
                    imageCache[urlStr] = fileName;  // Cập nhật cache
                } else {
                    qWarning() << "❌ File không tồn tại sau lưu";
                }
            } else {
                qWarning() << "❌ Không thể mở file để ghi:" << fileName;
                qWarning() << "❌ Đường dẫn:" << QFileInfo(fileName).absolutePath();
            }
        } else {
            qWarning() << "❌ Dữ liệu tải quá nhỏ:" << imageData.size() << "bytes";
        }
    } else {
        qWarning() << "❌ Lỗi tải ảnh:" << reply->errorString();

        // Debug: In HTTP status code
        int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qWarning() << "❌ HTTP Status:" << httpStatus;
    }

    delete reply;
    delete manager;
    return result;
}

// Trong file mainwindow.cpp

void MainWindow::onViewMyPenalties() {
    if (!currentUser) return;

    // 1. Tạo Dialog giao diện tối (chuẩn Nord theme của bạn)
    QDialog d(this);
    d.setWindowTitle("My Penalties & Overdue History");
    d.resize(850, 500);
    d.setStyleSheet("background-color: #2E3440; color: white;");

    QVBoxLayout *layout = new QVBoxLayout(&d);

    // Tiêu đề
    QLabel *header = new QLabel("💸 YOUR PENALTY RECORD");
    header->setStyleSheet("font-size: 18px; font-weight: bold; color: #BF616A; margin-bottom: 10px;");
    header->setAlignment(Qt::AlignCenter);
    layout->addWidget(header);

    // 2. Tạo bảng hiển thị
    QTableWidget *table = new QTableWidget();
    table->setColumnCount(6);
    table->setHorizontalHeaderLabels({"Book Title", "Return Date", "Overdue", "Fine ($)", "Status", "Action"});

    // Setup giao diện bảng (Copy style từ code cũ của bạn cho đồng bộ)
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setStyleSheet("background-color: #3B4252; gridline-color: transparent; border: none;");
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->verticalHeader()->setVisible(false);
    applyShadow(table); // Hiệu ứng bóng đổ

    // 3. Lấy dữ liệu phạt của User hiện tại
    vector<Penalty> myPenalties = library->getUserPenalties(currentUser->getUsername());
    table->setRowCount(0);

    float totalUnpaid = 0;

    for(const auto& p : myPenalties) {
        int row = table->rowCount();
        table->insertRow(row);

        // Tên sách
        table->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(p.getBookTitle())));

        // Ngày trả
        table->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(p.getReturnDate())));

        // Số ngày quá hạn
        QString daysText = QString::number(p.getDaysOverDue()) + " days";
        table->setItem(row, 2, new QTableWidgetItem(daysText));

        // Tiền phạt
        QTableWidgetItem *fineItem = new QTableWidgetItem("$" + QString::number(p.getPenaltyAmount(), 'f', 2));
        fineItem->setForeground(QBrush(QColor("#BF616A"))); // Màu đỏ
        fineItem->setFont(QFont("Arial", 10, QFont::Bold));
        table->setItem(row, 3, fineItem);

        // Trạng thái & Nút thanh toán
        QTableWidgetItem *statusItem;
        QWidget *actionWidget = new QWidget();
        QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(0,0,0,0);
        actionLayout->setAlignment(Qt::AlignCenter);

        if (p.getIsPaid()) {
            statusItem = new QTableWidgetItem("✅ PAID");
            statusItem->setForeground(QBrush(QColor("#A3BE8C"))); // Màu xanh lá

            QLabel *lblDone = new QLabel("Done");
            lblDone->setStyleSheet("color: #666;");
            actionLayout->addWidget(lblDone);
        } else {
            statusItem = new QTableWidgetItem("❌ UNPAID");
            statusItem->setForeground(QBrush(QColor("#D08770"))); // Màu cam
            totalUnpaid += p.getPenaltyAmount();

            // Nút thanh toán nhanh
            QPushButton *btnPay = new QPushButton("Pay Now");
            btnPay->setStyleSheet("background-color: #A3BE8C; color: black; border-radius: 5px; padding: 5px; font-weight: bold;");
            btnPay->setCursor(Qt::PointingHandCursor);

            // Xử lý sự kiện nút Pay
            // Lưu ý: Cần copy biến p.getISBN() ra để dùng trong lambda
            string isbnStr = p.getISBN();
            string userStr = currentUser->getUsername();
            float amount = p.getPenaltyAmount();

            connect(btnPay, &QPushButton::clicked, [this, &d, userStr, isbnStr, amount]() {
                QMessageBox::StandardButton reply;
                reply = QMessageBox::question(&d, "Confirm Payment",
                                              "Pay penalty of $" + QString::number(amount, 'f', 2) + "?",
                                              QMessageBox::Yes | QMessageBox::No);
                if (reply == QMessageBox::Yes) {
                    if (library->payPenalty(userStr, isbnStr)) {
                        QMessageBox::information(&d, "Success", "Payment successful! Thank you.");
                        d.accept(); // Đóng dialog để refresh lại (hoặc gọi lại hàm view)
                        onViewMyPenalties(); // Mở lại để cập nhật trạng thái
                    }
                }
            });

            actionLayout->addWidget(btnPay);
        }
        table->setItem(row, 4, statusItem);
        table->setCellWidget(row, 5, actionWidget);
    }

    layout->addWidget(table);

    // 4. Tổng kết tiền phạt
    QLabel *footer = new QLabel();
    if (totalUnpaid > 0) {
        footer->setText("⚠️ TOTAL AMOUNT DUE: $" + QString::number(totalUnpaid, 'f', 2));
        footer->setStyleSheet("color: #BF616A; font-size: 16px; font-weight: bold; margin-top: 10px;");
    } else {
        footer->setText("🎉 You have no unpaid penalties. Great job!");
        footer->setStyleSheet("color: #A3BE8C; font-size: 16px; font-weight: bold; margin-top: 10px;");
    }
    footer->setAlignment(Qt::AlignRight);
    layout->addWidget(footer);

    // Nút đóng
    QPushButton *btnClose = new QPushButton("Close");
    btnClose->setStyleSheet("background-color: #4C566A; padding: 8px; border-radius: 5px; margin-top: 10px;");
    connect(btnClose, &QPushButton::clicked, &d, &QDialog::accept);
    layout->addWidget(btnClose);

    d.exec();
}

void MainWindow::onReturnBook() { onViewMyBooks(); }
void MainWindow::onSortClicked() {
    library->sortBooksByTitle();

    // Kiểm tra xem đang ở Admin hay User page
    if (adminSearchBar) {
        // Admin page
        if (!adminSearchBar->text().isEmpty()) {
            onAdminSearchBooks(adminSearchBar->text());
        } else {
            onViewBooks();
        }
    } else if (searchBar) {
        // User page
        if (!searchBar->text().isEmpty()) {
            onSearchBooks(searchBar->text());
        } else {
            onViewBooks();
        }
    } else {
        onViewBooks();
    }
}
MainWindow::~MainWindow() { if(currentUser) delete currentUser; delete authSystem; delete library; }
void MainWindow::showAdminMenu() {}
void MainWindow::showUserMenu() {}
