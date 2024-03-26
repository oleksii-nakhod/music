#include "mainwindow.h"
#include "signup.h"
#include "ui_signup.h"
#include "login.h"
#include "config.h"

SignUp::SignUp(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SignUp)
{
    ui->setupUi(this);
}

SignUp::~SignUp()
{
    delete ui;
}

void SignUp::on_buttonLogin_clicked()
{
    close();
    emit login_clicked();
}


void SignUp::on_buttonSignup_clicked()
{
    QString username = ui->lineUsername->text();
    QString email = ui->lineEmail->text();
    QString password = ui->linePassword->text();
    QString password_confirm = ui->linePassconfirm->text();
    int role = ui->comboRole->currentIndex()+2;
    QRegularExpression rx("\\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}\\b",
                              QRegularExpression::CaseInsensitiveOption);
    ui->lineEmail->setValidator(new QRegularExpressionValidator(rx, this));

    if (username.length()<4) {
        emit error("Username is too short");
        return;
    }
    if (!ui->lineEmail->hasAcceptableInput()) {
        emit error("Email has incorrect format");
        return;
    }
    if (password.length()<6) {
        emit error("Password is too short");
        return;
    }
    if (password != password_confirm) {
        emit error("Passwords don't match");
        return;
    }
    QUrl url = QUrl(Config::SERVER_URL + "/signup");
    QJsonObject obj;
    obj.insert("username", username);
    obj.insert("email", email);
    obj.insert("password", password);
    obj.insert("role", role);

    QJsonDocument doc(obj);

    QByteArray data = doc.toJson();
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(signupFinished(QNetworkReply*)));
    manager->post(request, data);
}

void SignUp::signupFinished(QNetworkReply* reply) {
    QByteArray buffer = reply->readAll();
    reply->deleteLater();
    QJsonDocument jsonDoc(QJsonDocument::fromJson(buffer));
    QJsonObject jsonObj = jsonDoc.object();
    QString username = jsonObj.value("username").toString();
    int status = jsonObj.value("status").toInt();
    switch (status) {
        case 0:
            emit signup_success(username);
            emit information("Account created");
            close();
            break;
        case 1:
            emit error("Username already exists");
            break;
        case 2:
            emit error("Email already exists");
            break;
    }
}

