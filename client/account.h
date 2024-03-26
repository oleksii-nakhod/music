#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "qnetworkreply.h"
#include <QDialog>

namespace Ui {
class Account;
}

class Account : public QDialog
{
    Q_OBJECT

public:
    explicit Account(QWidget *parent = nullptr);
    QString username;
    ~Account();

private:
    Ui::Account *ui;

signals:
    void usernamechange_success(QString username);
    void error(QString message);
    void information(QString message);

private slots:
    void on_buttonPasschange_clicked();
    void passChangeFinished(QNetworkReply* reply);
    void usernameChangeFinished(QNetworkReply* reply);
    void on_buttonOK_clicked();
    void on_buttonUsernamechange_clicked();
};

#endif // ACCOUNT_H
