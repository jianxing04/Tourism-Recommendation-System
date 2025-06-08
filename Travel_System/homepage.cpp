#include "homepage.h"
#include<QMessageBox>
#include<QSqlError>
#include<QFile>

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

    btnEnterMap=new QPushButton("进入地图",this);

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
    rightLayout->addWidget(btnEnterMap);

    QWidget *centralWidget=new QWidget(this);
    centralWidget->setLayout(rightLayout);

    setCentralWidget(centralWidget);
    blogWindow=new BlogWindow(this,db);
    travelMap=new MainWindow();

    connect(btnSaveBlog,&QPushButton::clicked,this,&HomePage::onBtnSaveBlog);
    connect(btnEnterBlog,&QPushButton::clicked,this,&HomePage::onBtnEnterBlog);
    connect(btnEnterMap,&QPushButton::clicked,this,&HomePage::onBtnEnterMap);
}
HomePage::~HomePage(){

}

void HomePage::onBtnSaveBlog(){
    QString title=BlogTitleLineEdit->text();
    QString content=BlogContentLineEdit->text();

    QPair<QString, QMap<QChar, QString>> tem;
    tem=blogWindow->huffmanCompress(content);
    // 处理JSON文件追加
    QFile file("blogs.json");
    QJsonArray blogArray;

    // 读取现有内容
    if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QByteArray existingData = file.readAll();
        file.close();

        QJsonParseError parseError;
        QJsonDocument existingDoc = QJsonDocument::fromJson(existingData, &parseError);

        if (parseError.error == QJsonParseError::NoError && existingDoc.isArray()) {
            blogArray = existingDoc.array();
        }
    }

    // 创建新博客条目
    QJsonObject blogObject;
    blogObject["compressedContent"] = tem.first;
    qDebug()<<tem.first;
    // 转换哈夫曼表
    QJsonObject huffmanTableObject;
    for (auto it = tem.second.constBegin(); it != tem.second.constEnd(); ++it) {
        huffmanTableObject[QString(it.key())] = it.value();
        qDebug()<<QString(it.key())<<":"<<it.value();
    }
    blogObject["huffmanTable"] = huffmanTableObject;

    // 添加到数组
    blogArray.append(blogObject);

    // 写回文件
    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QJsonDocument jsonDoc(blogArray);
        QTextStream out(&file);
        out << jsonDoc.toJson(QJsonDocument::Indented);
        qDebug() << "添加评论成功";
        file.close();
    } else {
        qDebug() << "无法打开文件：" << file.errorString();
    }

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

void HomePage::onBtnEnterMap(){
    travelMap->show();
}
