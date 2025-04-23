#include "loginwindow.h"

LoginWindow::LoginWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 创建输入框和按钮
    useraccountLineEdit = new QLineEdit();
    passwordLineEdit = new QLineEdit(this);
    passwordLineEdit->setEchoMode(QLineEdit::Password);
    loginButton = new QPushButton("登录", this);
    registerButton = new QPushButton("注册", this);

    // 创建布局
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(new QLabel("账号:", this));
    mainLayout->addWidget(useraccountLineEdit);
    mainLayout->addWidget(new QLabel("密码:", this));
    mainLayout->addWidget(passwordLineEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(loginButton);
    buttonLayout->addWidget(registerButton);
    mainLayout->addLayout(buttonLayout);

    // 创建中央部件并设置布局
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    // 连接信号和槽
    connect(loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginButtonClicked);
    connect(registerButton, &QPushButton::clicked, this, &LoginWindow::onRegisterButtonClicked);

    // 连接数据库
    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setDatabaseName("qt_test");
    db.setUserName("root");
    db.setPassword("wjxmhcjlyAzg04");
    if (!db.open()) {
        QMessageBox::critical(this, "数据库连接错误", db.lastError().text());
    }

    //初始化一个注册窗口
    registrationWindow = new RegistrationWindow(this, db);
}

LoginWindow::~LoginWindow()
{
    db.close();
}

void LoginWindow::onLoginButtonClicked()
{
    QString useraccount = useraccountLineEdit->text();
    QString password = passwordLineEdit->text();

    QSqlQuery query(db);
    query.prepare("SELECT * FROM users WHERE userAccount = :useraccount");
    query.bindValue(":useraccount", useraccount);
    if (query.exec()) {
        if (query.next()) {
            QString storedPassword = query.value("userPassword").toString();
            if (storedPassword == password) {
                QMessageBox::information(this, "登录成功", "欢迎登录！");
            } else {
                QMessageBox::warning(this, "密码错误", "密码不正确，请重试。");
            }
        } else {
            QMessageBox::warning(this, "账号不存在", "该账号不存在，请注册。");
        }
    } else {
        QMessageBox::critical(this, "数据库查询错误", query.lastError().text());
    }
}

void LoginWindow::onRegisterButtonClicked()
{
    registrationWindow->show();
}
