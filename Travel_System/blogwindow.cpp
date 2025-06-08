#include "blogwindow.h"
#include <QDebug>
#include<QSqlError>
#include<QDialog>
#include <QMap>
#include<queue>
BlogWindow::BlogWindow(QWidget *parent,QSqlDatabase DB)
    : QMainWindow(parent)
{
    db=DB;
    if (!db.open()) {
        qDebug() << "无法连接到数据库：" << db.lastError().text();
    }

    // 创建滚动区域和布局
    scrollArea = new QScrollArea(this);
    QWidget *scrollWidget = new QWidget(scrollArea);
    blogLayout = new QVBoxLayout(scrollWidget);
    scrollArea->setWidget(scrollWidget);
    scrollArea->setWidgetResizable(true);

    // 添加按点赞数排序的按钮
    sortButton = new QPushButton("按点赞数排序", this);
    connect(sortButton, &QPushButton::clicked, this, &BlogWindow::sortBlogsByLikes);

    // 设置搜索按钮
    searchInput = new QLineEdit;
    searchInput->setPlaceholderText("输入景点名称搜索");
    searchButton = new QPushButton("搜索");
    connect(searchButton, &QPushButton::clicked, this, &BlogWindow::searchBlogs);
    searchLayout=new QHBoxLayout;
    searchLayout->addWidget(searchInput);
    searchLayout->addWidget(searchButton);


    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(sortButton);
    mainLayout->addLayout(searchLayout);
    mainLayout->addWidget(scrollArea);


    // 设置为中央布局
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    // 加载博客
    loadBlogs();
}

BlogWindow::~BlogWindow()
{
    db.close();
}


// 比较器，用于优先队列
struct Compare {
    bool operator()(HuffmanNode* l, HuffmanNode* r) {
        return l->freq > r->freq;
    }
};

// 构建哈夫曼树
HuffmanNode* BlogWindow::buildHuffmanTree(const QString& input) {
    QMap<QChar, int> freqMap;
    for (const QChar& c : input) {
        freqMap[c]++;
    }

    std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, Compare> minHeap;
    for (const auto& pair : freqMap.asKeyValueRange()) {
        minHeap.push(new HuffmanNode(pair.first, pair.second));
    }

    while (minHeap.size() != 1) {
        HuffmanNode* left = minHeap.top();
        minHeap.pop();
        HuffmanNode* right = minHeap.top();
        minHeap.pop();

        HuffmanNode* top = new HuffmanNode('\0', left->freq + right->freq);
        top->left = left;
        top->right = right;

        minHeap.push(top);
    }

    return minHeap.top();
}

// 生成哈夫曼编码表
void BlogWindow::generateCodes(HuffmanNode* root, QString str, QMap<QChar, QString>& huffmanCode) {
    if (!root) return;

    if (!root->left && !root->right) {
        huffmanCode[root->data] = str;
    }

    generateCodes(root->left, str + "0", huffmanCode);
    generateCodes(root->right, str + "1", huffmanCode);
}

// 哈夫曼压缩函数
QPair<QString, QMap<QChar, QString>> BlogWindow::huffmanCompress(const QString& input) {
    HuffmanNode* root = buildHuffmanTree(input);
    QMap<QChar, QString> huffmanCode;
    generateCodes(root, "", huffmanCode);

    QString compressed;
    for (const QChar& c : input) {
        compressed += huffmanCode[c];
    }

    return {compressed, huffmanCode};
}

// 哈夫曼解压缩函数
QString BlogWindow::huffmanDecompress(const QString& compressed, const QMap<QChar, QString>& huffmanCode) {
    QMap<QString, QChar> reverseCode;
    for (const auto& pair : huffmanCode.asKeyValueRange()) {
        reverseCode[pair.second] = pair.first;
    }

    QString decompressed;
    QString currentCode;
    for (const QChar& bit : compressed) {
        currentCode += bit;
        if (reverseCode.contains(currentCode)) {
            decompressed += reverseCode[currentCode];
            currentCode.clear();
        }
    }

    return decompressed;
}

//编辑距离
int BlogWindow::editDistance(const QString& s1, const QString& s2) {
    int m = s1.length();
    int n = s2.length();
    std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1));

    for (int i = 0; i <= m; ++i) {
        for (int j = 0; j <= n; ++j) {
            if (i == 0) {
                dp[i][j] = j;
            } else if (j == 0) {
                dp[i][j] = i;
            } else if (s1[i - 1] == s2[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1];
            } else {
                dp[i][j] = 1 + std::min({dp[i - 1][j], dp[i][j - 1], dp[i - 1][j - 1]});
            }
        }
    }
    return dp[m][n];
}

// 计算 KMP 算法的部分匹配表
std::vector<int> BlogWindow::computeLPS(const QString& pattern) {
    int m = pattern.length();
    std::vector<int> lps(m);
    int len = 0;
    lps[0] = 0;
    int i = 1;
    while (i < m) {
        if (pattern[i] == pattern[len]) {
            len++;
            lps[i] = len;
            i++;
        } else {
            if (len != 0) {
                len = lps[len - 1];
            } else {
                lps[i] = 0;
                i++;
            }
        }
    }
    return lps;
}

// KMP 字符串匹配函数
bool BlogWindow::kmpSearch(const QString& text, const QString& pattern) {
    int n = text.length();
    int m = pattern.length();
    std::vector<int> lps = computeLPS(pattern);
    int i = 0;
    int j = 0;
    while (i < n) {
        if (pattern[j] == text[i]) {
            j++;
            i++;
        }
        if (j == m) {
            return true;
        } else if (i < n && pattern[j] != text[i]) {
            if (j != 0) {
                j = lps[j - 1];
            } else {
                i++;
            }
        }
    }
    return false;
}

// 字符串匹配函数
int BlogWindow::stringMatch(const QString& str1, const QString& str2) {
    if (str1 == str2) {
        return 1;
    }

    QString shorter = str1.length() < str2.length() ? str1 : str2;
    QString longer = str1.length() < str2.length() ? str2 : str1;

    if (kmpSearch(longer, shorter)) {
        return 2;
    }

    return editDistance(str1, str2);
}

void BlogWindow::loadBlogs(bool sortByLikes, const QString& searchQuery )
{
    if (!db.open()) {
        qDebug() << "无法连接到数据库：" << db.lastError().text();
        return;
    }
    QSqlQuery query(db);
    QString sql = "SELECT blogs.id, users.userName, blogs.title, blogs.content, blogs.likes FROM blogs JOIN users ON blogs.userId = users.id";
    if (!searchQuery.isEmpty()) {
        sql += " WHERE blogs.title LIKE '%" + searchQuery + "%' OR blogs.content LIKE '%" + searchQuery + "%'";
    }
    if (sortByLikes) {
        sql += " ORDER BY blogs.likes DESC";
    }
    if (!query.exec(sql)) {
        qDebug() << "查询博客失败：" << query.lastError().text();
        return;
    }

    // 清空现有博客布局
    while (blogLayout->count() > 0) {
        QWidget *widget = blogLayout->takeAt(0)->widget();
        if (widget) {
            delete widget;
        }
    }

    while (query.next()) {
        int blogId = query.value(0).toInt();
        QString username = query.value(1).toString();
        QString title = query.value(2).toString();
        QString content = query.value(3).toString();
        int likes = query.value(4).toInt();

        // 创建博客容器
        QWidget *blogWidget = new QWidget();
        QVBoxLayout *blogItemLayout = new QVBoxLayout(blogWidget);

        // 显示博客标题和作者
        QLabel *titleLabel = new QLabel(title + " - " + username);
        blogItemLayout->addWidget(titleLabel);

        // 显示博客内容
        QLabel *contentLabel = new QLabel(content);
        blogItemLayout->addWidget(contentLabel);

        // 显示点赞数
        QLabel *likesLabel = new QLabel("点赞数：" + QString::number(likes));
        blogItemLayout->addWidget(likesLabel);

        // 点赞按钮
        QPushButton *likeButton = new QPushButton("点赞");
        connect(likeButton, &QPushButton::clicked, [this, blogId]() {
            likeBlog(blogId);
        });
        blogItemLayout->addWidget(likeButton);

        // 留言输入框
        QTextEdit *commentEdit = new QTextEdit();
        blogItemLayout->addWidget(commentEdit);

        // 留言按钮
        QPushButton *commentButton = new QPushButton("留言");
        connect(commentButton, &QPushButton::clicked, [this, blogId, commentEdit]() {
            QString comment = commentEdit->toPlainText();
            addComment(blogId, comment);
            commentEdit->clear();
        });
        blogItemLayout->addWidget(commentButton);

        // 查看回复按钮
        QPushButton *viewCommentsButton = new QPushButton("查看回复");
        connect(viewCommentsButton, &QPushButton::clicked, [this, blogId]() {
            viewComments(blogId);
        });
        blogItemLayout->addWidget(viewCommentsButton);

        // 将博客容器添加到布局中
        blogLayout->addWidget(blogWidget);
    }
}

void BlogWindow::likeBlog(int blogId)
{
    if (!db.open()) {
        qDebug() << "无法连接到数据库：" << db.lastError().text();
        return;
    }
    QSqlQuery query(db);
    query.prepare("UPDATE blogs SET likes = likes + 1 WHERE id = :id");
    query.bindValue(":id", blogId);
    if (query.exec()) {
        // 点赞成功后重新加载博客
        loadBlogs();
    } else {
        qDebug() << "点赞失败：" << query.lastError().text();
    }
}

void BlogWindow::addComment(int blogId, const QString& comment)
{
    if (!db.open()) {
        qDebug() << "无法连接到数据库：" << db.lastError().text();
        return;
    }
    QSqlQuery query(db);
    query.prepare("INSERT INTO comments (blogId, comment) VALUES (:blog_id, :comment)");
    query.bindValue(":blog_id", blogId);
    query.bindValue(":comment", comment);
    if (query.exec()) {
        qDebug() << "留言成功";
    } else {
        qDebug() << "留言失败：" << query.lastError().text();
    }
}

void BlogWindow::sortBlogsByLikes()
{
    QString str=this->searchInput->text();
    loadBlogs(true,str);
}

void BlogWindow::viewComments(int blogId)
{
    if (!db.open()) {
        qDebug() << "无法连接到数据库：" << db.lastError().text();
        return;
    }
    QSqlQuery query(db);
    query.prepare("SELECT comments.comment FROM comments JOIN blogs ON comments.blogId = blogs.id WHERE comments.blogId = :blog_id");
    query.bindValue(":blog_id", blogId);
    if (!query.exec()) {
        qDebug() << "查询留言失败：" << query.lastError().text();
        return;
    }

    QWidget *commentsWidget = new QWidget();
    QVBoxLayout *commentsLayout = new QVBoxLayout(commentsWidget);

    while (query.next()) {
        QString comment = query.value(0).toString();

        // 显示留言信息
        QLabel *commentLabel = new QLabel(comment);
        commentsLayout->addWidget(commentLabel);

    }
    QPushButton *btnClose=new QPushButton("确定");
    commentsLayout->addWidget(btnClose);
    connect(btnClose,&QPushButton::clicked,commentsWidget,&QWidget::close);
    commentsWidget->show();
}

void BlogWindow::searchBlogs()
{
    QString searchQuery = searchInput->text();
    loadBlogs(false, searchQuery);
}
