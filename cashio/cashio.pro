#-------------------------------------------------
#
# Project created by QtCreator 2011-08-22T15:07:11
#
#-------------------------------------------------

QT       += core gui

TARGET = cashio
TEMPLATE = app

SOURCES += main.cpp\
        MainWindow.cpp \
    CashDb.cpp \
    TableRendererCash.cpp \
    ComboBoxDelegate.cpp

HEADERS  += MainWindow.h \
    DbDef.h \
    CashDb.h \
    SqliteBase.h \
    TableRendererCash.h \
    ComboBoxDelegate.h

FORMS    += MainWindow.ui

# You should put sqlite3.h into "include",
# and sqlite3.lib into "lib" for Windows.
win32 {
    INCLUDEPATH += $$_PRO_FILE_PWD_\include
    LIBS += -L$$_PRO_FILE_PWD_\lib\ -lsqlite3.lib

}

# You should have sqlite3 installed under /usr .
unix {
    INCLUDEPATH += /usr/local/include
    LIBS += -lsqlite3
}
