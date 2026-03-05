QT       += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

TARGET = LibraryManagementSystem
TEMPLATE = app

# --- KHAI BÁO FILE (Đường dẫn gốc) ---
SOURCES += \
    main.cpp \
    mainwindow.cpp \
    authsystem.cpp \
    penalty.cpp \
    user.cpp \
    book.cpp \
    borrowrecord.cpp \
    library.cpp

HEADERS += \
    mainwindow.h \
    authsystem.h \
    penalty.h \
    user.h \
    book.h \
    borrowrecord.h \
    library.h

FORMS += \
    mainwindow.ui
