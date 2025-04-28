#include "homepage.h"
#include<QMessageBox>
#include<QSqlError>

HomePage::HomePage(const QString &username,const int &userid,QSqlDatabase DB) {
    userName=username;
    userId=userid;
    db=DB;
    if (!db.open()) {
        qDebug() << "无法连接到数据库：" << db.lastError().text();
        return;
    }

    welcome=new QLabel("welcome! "+userName,this);
    welcome->setAlignment(Qt::AlignCenter);

    lbBlogTitle=new QLabel("标题",this);
    lbBlogContent=new QLabel("内容",this);

    BlogTitleLineEdit=new QLineEdit(this);
    BlogContentLineEdit=new QLineEdit(this);

    btnSaveBlog=new QPushButton("发送",this);
    btnEnterBlog=new QPushButton("进入博客区",this);

    QHBoxLayout *titleLayout=new QHBoxLayout();
    titleLayout->addWidget(lbBlogTitle);
    titleLayout->addWidget(BlogTitleLineEdit);

    QHBoxLayout *contentLayout=new QHBoxLayout();
    contentLayout->addWidget(lbBlogContent);
    contentLayout->addWidget(BlogContentLineEdit);

    QVBoxLayout *rightLayout=new QVBoxLayout;
    rightLayout->addWidget(welcome);
    rightLayout->addLayout(titleLayout);
    rightLayout->addLayout(contentLayout);
    rightLayout->addWidget(btnSaveBlog);
    rightLayout->addWidget(btnEnterBlog);

    QWidget *centralWidget=new QWidget(this);
    centralWidget->setLayout(rightLayout);
    //mainLayout=new QHBoxLayout;
    //mainLayout->addLayout(rightLayout);

    setCentralWidget(centralWidget);
    blogWindow=new BlogWindow(this,db);

    connect(btnSaveBlog,&QPushButton::clicked,this,&HomePage::onBtnSaveBlog);
    connect(btnEnterBlog,&QPushButton::clicked,this,&HomePage::onBtnEnterBlog);
}
HomePage::~HomePage(){

}

void HomePage::onBtnSaveBlog(){
    QString title=BlogTitleLineEdit->text();
    QString content=BlogContentLineEdit->text();
    int likes=0;
    if (!db.open()) {
        qDebug() << "无法连接到数据库：" << db.lastError().text();
        return;
    }
    QSqlQuery query;
    query.prepare("INSERT INTO blogs (userId, title, content,likes) VALUES (:id, :title, :content,:likes)");
    query.bindValue(":id", userId);
    query.bindValue(":title", title);
    query.bindValue(":content", content);
    query.bindValue(":likes", likes);
    if (query.exec()) {
        QMessageBox::information(this, "写博客成功", "恭喜，写博客成功！");
    } else {
        QMessageBox::critical(this, "博客插入错误", query.lastError().text());
    }
}

void HomePage::onBtnEnterBlog(){
    blogWindow->show();
}
