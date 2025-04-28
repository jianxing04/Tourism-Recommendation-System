#include "loginwindow.h"

#include <QApplication>

QSqlDatabase db;
void LinkToMySQL(){
    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setDatabaseName("qt_test");
    db.setUserName("root");
    db.setPassword("wjxmhcjlyAzg04");
    if (!db.open()) {
        qDebug() << "无法连接到数据库：" << db.lastError().text();
        return;
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LinkToMySQL();
    LoginWindow w(db);
    w.show();
    return a.exec();
}
