#include "account.h"
#include "ui_account.h"
#include "mainwindow.h"
#include "config.h"

Account::Account(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Account)
{
    ui->setupUi(this);
}

Account::~Account()
{
    delete ui;
}

void Account::on_buttonPasschange_clicked()
{
    QString password_old = ui->linePassold->text();
    QString password_new = ui->linePassnew->text();
    QString password_confirm = ui->linePassconfirm->text();

    if (password_new.length()<6) {
        emit error("New password is too short");
        return;
    }
    if (password_new != password_confirm) {
        emit error("Passwords don't match");
        return;
    }
    QUrl url = QUrl(Config::SERVER_URL + "/change_password");
    QJsonObject obj;
    obj.insert("username", username);
    obj.insert("password_old", password_old);
    obj.insert("password_new", password_new);

    QJsonDocument doc(obj);

    QByteArray data = doc.toJson();
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(passChangeFinished(QNetworkReply*)));
    manager->post(request, data);
}

void Account::passChangeFinished(QNetworkReply* reply) {
    QByteArray buffer = reply->readAll();
    reply->deleteLater();
    QJsonDocument jsonDoc(QJsonDocument::fromJson(buffer));
    QJsonObject jsonObj = jsonDoc.object();
    int status = jsonObj.value("status").toInt();
    switch (status) {
        case 0:
            emit information("Password changed");
            ui->linePassold->clear();
            ui->linePassnew->clear();
            ui->linePassconfirm->clear();
            break;
        case 1:
            emit error("Password is incorrect");
            break;
    }
}


void Account::on_buttonOK_clicked()
{
    close();
}


void Account::on_buttonUsernamechange_clicked()
{
    QString password = ui->linePass->text();
    QString username_new = ui->lineUsernamenew->text();

    if (username_new.length()<4) {
        emit error("New username is too short");
        return;
    }
    QUrl url = QUrl(Config::SERVER_URL + "/change_username");
    QJsonObject obj;
    obj.insert("username_old", username);
    obj.insert("password", password);
    obj.insert("username_new", username_new);

    QJsonDocument doc(obj);

    QByteArray data = doc.toJson();
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(usernameChangeFinished(QNetworkReply*)));
    manager->post(request, data);
}

void Account::usernameChangeFinished(QNetworkReply* reply) {
    QByteArray buffer = reply->readAll();
    reply->deleteLater();
    QJsonDocument jsonDoc(QJsonDocument::fromJson(buffer));
    QJsonObject jsonObj = jsonDoc.object();
    QString username_new = jsonObj.value("username").toString();
    int status = jsonObj.value("status").toInt();
    switch (status) {
        case 0:
            emit information("Username changed");
            username = username_new;
            emit usernamechange_success(username);
            ui->linePass->clear();
            ui->lineUsernamenew->clear();
            break;
        case 1:
            emit error("Password is incorrect");
            break;
    }
}
