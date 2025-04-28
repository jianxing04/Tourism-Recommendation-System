#ifndef BLOGWINDOW_H
#define BLOGWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include<QLineEdit>
#include<QHBoxLayout>

class BlogWindow : public QMainWindow
{
    Q_OBJECT

public:
    BlogWindow(QWidget *parent = nullptr,QSqlDatabase DB=QSqlDatabase());
    ~BlogWindow();

private slots:
    void loadBlogs(bool sortByLikes = false, const QString& searchQuery = "");
    void likeBlog(int blogId);
    void addComment(int blogId,const QString &comment);
    void sortBlogsByLikes();
    void viewComments(int blogId);
    void searchBlogs();

private:
    QSqlDatabase db;
    QVBoxLayout *blogLayout;
    QScrollArea *scrollArea;
    QPushButton *sortButton;
    QHBoxLayout *searchLayout;
    QLineEdit *searchInput;
    QPushButton *searchButton;
};
#endif // BLOGWINDOW_H
