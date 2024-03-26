#ifndef LOGIN_H
#define LOGIN_H

#include "qnetworkaccessmanager.h"
#include <QDialog>

namespace Ui {
class Login;
}

class Login : public QDialog
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = nullptr);
    ~Login();

private slots:
    void on_buttonLogin_clicked();

    void on_buttonSignup_clicked();

    void loginFinished(QNetworkReply* reply);
signals:
    void login_success(QString username);
    void signup_clicked();
    void error(QString message);

private:
    Ui::Login *ui;
};

#endif // LOGIN_H
