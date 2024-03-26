#ifndef SIGNUP_H
#define SIGNUP_H

#include <QDialog>
#include "qnetworkaccessmanager.h"

namespace Ui {
class SignUp;
}

class SignUp : public QDialog
{
    Q_OBJECT

public:
    explicit SignUp(QWidget *parent = nullptr);
    ~SignUp();

private slots:
    void on_buttonLogin_clicked();

    void on_buttonSignup_clicked();

    void signupFinished(QNetworkReply* reply);
signals:
    void signup_success(QString username);
    void login_clicked();
    void error(QString message);
    void information(QString message);
private:
    Ui::SignUp *ui;
};

#endif // SIGNUP_H
