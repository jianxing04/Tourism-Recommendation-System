#include "registrationwindow.h"

RegistrationWindow::RegistrationWindow(QWidget *parent, QSqlDatabase db)
    : QDialog(parent), db(db)
{
    // 创建输入框和按钮
    nameLineEdit = new QLineEdit(this);
    useraccountLineEdit1 = new QLineEdit(this);
    useraccountLineEdit2 = new QLineEdit(this);
    passwordLineEdit1 = new QLineEdit(this);
    passwordLineEdit1->setEchoMode(QLineEdit::Password);
    passwordLineEdit2 = new QLineEdit(this);
    passwordLineEdit2->setEchoMode(QLineEdit::Password);
    registerConfirmButton = new QPushButton("确认注册", this);

    // 创建布局
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(new QLabel("用户名:", this));
    mainLayout->addWidget(nameLineEdit);
    mainLayout->addWidget(new QLabel("账号:", this));
    mainLayout->addWidget(useraccountLineEdit1);
    mainLayout->addWidget(new QLabel("再次输入账号:", this));
    mainLayout->addWidget(useraccountLineEdit2);
    mainLayout->addWidget(new QLabel("密码:", this));
    mainLayout->addWidget(passwordLineEdit1);
    mainLayout->addWidget(new QLabel("再次输入密码:", this));
    mainLayout->addWidget(passwordLineEdit2);
    mainLayout->addWidget(registerConfirmButton);

    // 设置布局
    setLayout(mainLayout);

    // 连接信号和槽
    connect(registerConfirmButton, &QPushButton::clicked, this, &RegistrationWindow::onRegisterConfirmButtonClicked);
}

RegistrationWindow::~RegistrationWindow()
{
}

void RegistrationWindow::onRegisterConfirmButtonClicked()
{
    QString name = nameLineEdit->text();
    QString useraccount1 = useraccountLineEdit1->text();
    QString useraccount2 = useraccountLineEdit2->text();
    QString password1 = passwordLineEdit1->text();
    QString password2 = passwordLineEdit2->text();

    if (name.isEmpty()||useraccount1.isEmpty()||useraccount2.isEmpty()||password1.isEmpty()||password2.isEmpty()) {
        QMessageBox::warning(this, "注册失败", "任何信息不能为空。");
        return;
    }

    if (useraccount1 != useraccount2) {
        QMessageBox::warning(this, "注册失败", "两次输入的账号不一致。");
        return;
    }

    if (password1 != password2) {
        QMessageBox::warning(this, "注册失败", "两次输入的密码不一致。");
        return;
    }

    QSqlQuery query(db);
    query.prepare("SELECT * FROM users WHERE userAccount = :useraccount");
    query.bindValue(":useraccount", useraccount1);
    if (query.exec()) {
        if (query.next()) {
            QMessageBox::warning(this, "注册失败", "该账号已存在，请选择其他账号。");
            return;
        }
    } else {
        QMessageBox::critical(this, "数据库查询错误", query.lastError().text());
        return;
    }

    query.prepare("INSERT INTO users (userName, userAccount, userPassword) VALUES (:name, :account, :password)");
    query.bindValue(":name", name);
    query.bindValue(":account", useraccount1);
    query.bindValue(":password", password1);
    if (query.exec()) {
        QMessageBox::information(this, "注册成功", "恭喜，注册成功！");
        close();
    } else {
        QMessageBox::critical(this, "数据库插入错误", query.lastError().text());
    }
}
