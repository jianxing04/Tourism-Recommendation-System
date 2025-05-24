#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include<QMainWindow>
#include<QLabel>
#include<QVBoxLayout>
#include<QHBoxLayout>
#include<QSqlDatabase>
#include"blogwindow.h"
#include"mainwindow.h"
#include<QLineEdit>
#include<QPushButton>

class HomePage : public QMainWindow
{
    Q_OBJECT

public:
    HomePage(const QString &username,const int &userid,QSqlDatabase DB);
    ~HomePage();
    QString userName;
    int userId;
private slots:
    void onBtnSaveBlog();
    void onBtnEnterBlog();
    void onBtnEnterMap();
private:
    BlogWindow *blogWindow;
    MainWindow *travelMap;
    QLabel *welcome;
    QLabel *lbBlogTitle;
    QLabel *lbBlogContent;
    QLineEdit *BlogTitleLineEdit;
    QLineEdit *BlogContentLineEdit;
    QPushButton *btnSaveBlog;
    QPushButton *btnEnterBlog;
    QPushButton *btnEnterMap;
    QSqlDatabase db;
};

#endif // HOMEPAGE_H
