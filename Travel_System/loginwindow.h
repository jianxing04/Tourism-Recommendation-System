#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include "registrationwindow.h"
#include "homepage.h"

class LoginWindow : public QMainWindow
{
    Q_OBJECT

public:
    LoginWindow(QSqlDatabase DB);
    ~LoginWindow();
    int userId;
    QString userName;

private slots:
    void onLoginButtonClicked();
    void onRegisterButtonClicked();

private:
    QLineEdit *useraccountLineEdit;
    QLineEdit *passwordLineEdit;
    QPushButton *loginButton;
    QPushButton *registerButton;
    QSqlDatabase db;
    RegistrationWindow *registrationWindow;
    HomePage *homepage;
};
#endif // LOGINWINDOW_H
