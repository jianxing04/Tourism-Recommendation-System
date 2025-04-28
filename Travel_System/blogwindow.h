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
#include<vector>

// 哈夫曼树节点结构体
struct HuffmanNode {
    QChar data;
    int freq;
    HuffmanNode *left, *right;

    HuffmanNode(QChar data, int freq) : data(data), freq(freq), left(nullptr), right(nullptr) {}
};
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
    HuffmanNode* buildHuffmanTree(const QString& input);
    void generateCodes(HuffmanNode* root, QString str, QMap<QChar, QString>& huffmanCode);
    QPair<QString, QMap<QChar, QString>> huffmanCompress(const QString& input);
    QString huffmanDecompress(const QString& compressed, const QMap<QChar, QString>& huffmanCode);
    int editDistance(const QString& s1, const QString& s2);
    std::vector<int> computeLPS(const QString& pattern);
    bool kmpSearch(const QString& text, const QString& pattern);
    int stringMatch(const QString& str1, const QString& str2);
};
#endif // BLOGWINDOW_H
