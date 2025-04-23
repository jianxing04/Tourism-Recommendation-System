#ifndef REGISTRATIONWINDOW_H
#define REGISTRATIONWINDOW_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QLabel>

class RegistrationWindow : public QDialog
{
    Q_OBJECT

public:
    RegistrationWindow(QWidget *parent = nullptr, QSqlDatabase db = QSqlDatabase());
    ~RegistrationWindow();

private slots:
    void onRegisterConfirmButtonClicked();

private:
    QLineEdit *nameLineEdit;
    QLineEdit *useraccountLineEdit1;
    QLineEdit *useraccountLineEdit2;
    QLineEdit *passwordLineEdit1;
    QLineEdit *passwordLineEdit2;
    QPushButton *registerConfirmButton;
    QSqlDatabase db;
};
#endif // REGISTRATIONWINDOW_H
