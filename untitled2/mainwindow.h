#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void loadBlogs(bool sortByLikes = false, const QString& searchQuery = "");
    void likeBlog(int blogId);
    void addComment(int blogId,const QString &comment);
    void sortBlogsByLikes();
    void viewComments(int blogId);
    void searchBlogs();

private:
    Ui::MainWindow *ui;
    QSqlDatabase db;
    QVBoxLayout *blogLayout;
    QScrollArea *scrollArea;
    QPushButton *sortButton;
    QHBoxLayout *searchLayout;
    QLineEdit *searchInput;
    QPushButton *searchButton;
};
#endif // MAINWINDOW_H
