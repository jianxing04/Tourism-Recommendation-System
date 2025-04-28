#include "blogwindow.h"
#include <QDebug>
#include<QSqlError>
#include<QDialog>
BlogWindow::BlogWindow(QWidget *parent,QSqlDatabase DB)
    : QMainWindow(parent)
{
    // 连接到 MySQL 数据库
    // db = QSqlDatabase::addDatabase("QMYSQL");
    // db.setHostName("localhost");
    // db.setDatabaseName("qt_test");
    // db.setUserName("root");
    // db.setPassword("wjxmhcjlyAzg04");
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
