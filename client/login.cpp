#include "mainwindow.h"
#include "login.h"
#include "ui_login.h"
#include "config.h"

Login::Login(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Login)
{
    ui->setupUi(this);
}

Login::~Login()
{
    delete ui;
}

void Login::on_buttonLogin_clicked()
{
    QString username = ui->lineUsername->text();
    QString password = ui->linePassword->text();
    QUrl url = QUrl(Config::SERVER_URL + "/login");
    QJsonObject obj;
    obj.insert("username", username);
    obj.insert("password", password);

    QJsonDocument doc(obj);

    QByteArray data = doc.toJson();
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(loginFinished(QNetworkReply*)));
    manager->post(request, data);
}


void Login::on_buttonSignup_clicked()
{
    close();
    emit signup_clicked();
}

void Login::loginFinished(QNetworkReply* reply) {
    QByteArray buffer = reply->readAll();
    reply->deleteLater();
    QJsonDocument jsonDoc(QJsonDocument::fromJson(buffer));
    QJsonObject jsonObj = jsonDoc.object();
    QString username = jsonObj.value("username").toString();
    int status = jsonObj.value("status").toInt();
    switch (status) {
        case 0:
            emit login_success(username);
            close();
            break;
        case 1:
            emit error("Incorrect username or password");
            break;
    }
}
