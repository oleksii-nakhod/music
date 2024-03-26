#include "mainwindow.h"
#include "login.h"
#include "./ui_mainwindow.h"
#include "config.h"
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QTableWidget* table = ui->tableSongs;
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    table->setColumnHidden(3, true);
    table->setColumnHidden(4, true);
    player->setAudioOutput(audioOutput);
    audioOutput->setVolume(float(ui->sliderVolume->value())/100.0);
    connect(player, SIGNAL(positionChanged(qint64)), this, SLOT(update_position(qint64)));
    connect(ui->sliderSeekbar, SIGNAL(valueChanged(int)), this, SLOT(update_slider(int)));
    connect(player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(update_status(QMediaPlayer::MediaStatus)));
    connect(player, SIGNAL(playbackStateChanged(QMediaPlayer::PlaybackState)), this, SLOT(update_state(QMediaPlayer::PlaybackState)));
    connect(ui->sliderVolume, SIGNAL(valueChanged(int)), this, SLOT(set_volume(int)));
    connect(artworkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(artworkDownloaded(QNetworkReply*)));
    connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(menuClicked(QAction*)));
    menu->addAction(tr("Log in"));
    menu->addAction(tr("Sign up"));
    ui->buttonActions->setMenu(menu);
    ui->lineSearch->addAction(QIcon(":/resources/icons/search.png"), QLineEdit::LeadingPosition);

    ui->sliderSeekbar->setStyle(new JumpSlider(ui->sliderSeekbar->style()));
    ui->sliderVolume->setStyle(new JumpSlider(ui->sliderSeekbar->style()));

    QUrl urlPlaylists(Config::SERVER_URL + "/load_playlists");
    QNetworkAccessManager *managerPlaylists = new QNetworkAccessManager(this);
    QNetworkRequest requestPlaylists(urlPlaylists);
    connect(managerPlaylists, SIGNAL(finished(QNetworkReply*)), this, SLOT(loadPlaylistsFinished(QNetworkReply*)));
    managerPlaylists->get(requestPlaylists);

    QUrl urlAlbums = QUrl(Config::SERVER_URL + "/load_albums");
    QNetworkAccessManager *managerAlbums = new QNetworkAccessManager(this);
    QNetworkRequest requestAlbums(urlAlbums);
    connect(managerAlbums, SIGNAL(finished(QNetworkReply*)), this, SLOT(loadAlbumsFinished(QNetworkReply*)));
    managerAlbums->get(requestAlbums);

    ui->lineSearch->hide();
    ui->buttonLike->hide();
    ui->buttonLike->setProperty("active", 0);
    ui->buttonLibrary->hide();
    set_fonts();

    ui->scrollArea->setFrameShape(QFrame::NoFrame);
    enable_buttons(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString secondsToDuration(int seconds) {
    QString strSeconds;
    if (seconds%60<10) {
        strSeconds = "0"+QString::number(seconds%60);
    }
    else {
        strSeconds = QString::number(seconds%60);
    }
    return QString::number(seconds/60)+":"+strSeconds;
}

void MainWindow::connect_account(QString name) {
    username = name;
    connected = true;
    ui->buttonActions->setText(username);
    ui->buttonLike->show();
    ui->buttonLibrary->show();
    menu->clear();
    menu->addAction(tr("Account"));
    menu->addAction(tr("Options"));
    menu->addAction(tr("Log out"));
}

void MainWindow::create_login_window() {
    Login *login = new Login();
    connect(login, &Login::login_success, this, &MainWindow::connect_account);
    connect(login, &Login::signup_clicked, this, &MainWindow::create_signup_window);
    connect(login, &Login::error, this, &MainWindow::create_error_window);
    login->exec();
}

void MainWindow::create_signup_window() {
    SignUp *signup = new SignUp();
    connect(signup, &SignUp::signup_success, this, &MainWindow::connect_account);
    connect(signup, &SignUp::login_clicked, this, &MainWindow::create_login_window);
    connect(signup, &SignUp::information, this, &MainWindow::create_information_window);
    connect(signup, &SignUp::error, this, &MainWindow::create_error_window);
    signup->exec();
}

void MainWindow::create_account_window() {
    Account *account = new Account();
    connect(account, &Account::usernamechange_success, this, &MainWindow::connect_account);
    connect(account, &Account::error, this, &MainWindow::create_error_window);
    connect(account, &Account::information, this, &MainWindow::create_information_window);
    account->username=this->username;
    account->exec();
}

void MainWindow::create_error_window(QString message) {
    QMessageBox msg;
    msg.setText(message);
    msg.setIcon(QMessageBox::Critical);
    msg.exec();
}

void MainWindow::create_information_window(QString message) {
    QMessageBox msg;
    msg.setText(message);
    msg.setIcon(QMessageBox::Information);
    msg.exec();
}

void MainWindow::on_sliderSeekbar_sliderReleased()
{
    player->setPosition(float(ui->sliderSeekbar->value())/100*float(player->duration()));
}

void MainWindow::load_songs_finished(QNetworkReply* reply) {
    QTableWidget* table = ui->tableSongs;
    table->setRowCount(0);
    QByteArray buffer = reply->readAll();
    qDebug() << buffer;
    reply->deleteLater();
    QJsonDocument jsonDoc(QJsonDocument::fromJson(buffer));
    QJsonArray jsonArray = jsonDoc.array();
    foreach (const QJsonValue& value, jsonArray) {
        QJsonObject song = value.toObject();
        QString song_id = song.value("song_id").toString();
        QString song_google_id = song.value("song_url").toString();
        QString song_title = song.value("song_title").toString();
        QString album_id = song.value("album_id").toString();
        QString album_google_id = song.value("album_url").toString();
        QString album_title = song.value("album_title").toString();
        QString artist_name = song.value("artist_name").toString();
        table->insertRow(table->rowCount());
        table->setItem(table->rowCount()-1, 0, new QTableWidgetItem(song_title));
        table->setItem(table->rowCount()-1, 1, new QTableWidgetItem(artist_name));
        table->setItem(table->rowCount()-1, 2, new QTableWidgetItem(album_title));
        table->setItem(table->rowCount()-1, 3, new QTableWidgetItem(song_google_id));
        table->setItem(table->rowCount()-1, 4, new QTableWidgetItem(album_google_id));
    }
    ui->contents->setCurrentIndex(0);
}

void MainWindow::playlistArtworkDownloaded() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    QToolButton* playlist_button = qobject_cast<QToolButton*>(reply->parent());
    QByteArray buffer = reply->readAll();
    reply->deleteLater();
    QPixmap pixmap;
    pixmap.loadFromData(buffer);
    pixmap = pixmap.scaled(playlist_button->width(),playlist_button->height(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
    playlist_button->setIcon(pixmap);
}

void MainWindow::loadPlaylistsFinished(QNetworkReply* reply) {
    int playlist_counter = 0;
    QByteArray buffer = reply->readAll();
    reply->deleteLater();
    QJsonDocument jsonDoc(QJsonDocument::fromJson(buffer));
    QJsonArray jsonArray = jsonDoc.array();
    QHBoxLayout* layout;
    foreach (const QJsonValue& value, jsonArray) {
        QJsonObject playlist = value.toObject();
        int playlist_id = playlist.value("playlist_id").toInt();
        QString playlist_google_id = playlist.value("playlist_url").toString();
        QString playlist_title = playlist.value("playlist_title").toString();

        QToolButton* playlist_button = new QToolButton();
        playlist_button->setMinimumSize(QSize(100,130));
        playlist_button->setIconSize(QSize(100,100));
        playlist_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        playlist_button->setText(playlist_title);
        playlist_button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        playlist_button->setProperty("id", playlist_id);
        playlist_button->setProperty("title", playlist_title);
        connect(playlist_button, SIGNAL(clicked()), this, SLOT(playlist_clicked()));

        QUrl playlist_url = QUrl(playlist_google_id);
        QNetworkAccessManager* manager = new QNetworkAccessManager(this);
        QNetworkReply* reply = manager->get(QNetworkRequest(playlist_url));
        reply->setParent(playlist_button);
        connect(reply, SIGNAL(finished()), this, SLOT(playlistArtworkDownloaded()));

        if (playlist_counter%4==0) {
            layout = new QHBoxLayout();
            ui->spaceCategories->addLayout(layout);
        }
        layout->addWidget(playlist_button);
        playlist_counter+=1;
    }
}

void MainWindow::loadAlbumsFinished(QNetworkReply* reply) {
    int album_counter = 0;
    QByteArray buffer = reply->readAll();

    reply->deleteLater();
    QJsonDocument jsonDoc(QJsonDocument::fromJson(buffer));
    QJsonArray jsonArray = jsonDoc.array();
    QHBoxLayout* layout;
    foreach (const QJsonValue& value, jsonArray) {
        QJsonObject album = value.toObject();
        int album_id = album.value("album_id").toInt();
        QString album_google_id = album.value("album_url").toString();
        QString album_title = album.value("album_title").toString();

        QToolButton* album_button = new QToolButton();
        album_button->setMinimumSize(QSize(100,130));
        album_button->setIconSize(QSize(100,100));
        album_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        album_button->setText(album_title);
        album_button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        album_button->setProperty("id", album_id);
        album_button->setProperty("title", album_title);
        connect(album_button, SIGNAL(clicked()), this, SLOT(album_clicked()));

        QUrl album_url = QUrl(album_google_id);
        QNetworkAccessManager* manager = new QNetworkAccessManager(this);
        QNetworkReply* reply = manager->get(QNetworkRequest(album_url));
        reply->setParent(album_button);
        connect(reply, SIGNAL(finished()), this, SLOT(playlistArtworkDownloaded()));

        if (album_counter%4==0) {
            layout = new QHBoxLayout();
            ui->spaceCategories->addLayout(layout);
        }
        layout->addWidget(album_button);
        album_counter+=1;
    }
}

void MainWindow::playlist_clicked() {
    QToolButton* button_sender = qobject_cast<QToolButton*>(sender());
    QString playlist_id = button_sender->property("id").toString();
    QString playlist_title = button_sender->property("title").toString();
    QUrl url_playlist_songs(Config::SERVER_URL + "/load_playlist_songs");
    QUrlQuery query;
    query.addQueryItem("playlist_id", playlist_id);
    url_playlist_songs.setQuery(query);

    QNetworkAccessManager *manager_playlist_songs = new QNetworkAccessManager(this);
    QNetworkRequest request_playlist_songs(url_playlist_songs);
    connect(manager_playlist_songs, SIGNAL(finished(QNetworkReply*)), this, SLOT(load_songs_finished(QNetworkReply*)));
    manager_playlist_songs->get(request_playlist_songs);
    ui->label_playlist_title->setText(playlist_title);
    ui->tableSongs->setColumnHidden(1, false);
    ui->tableSongs->setColumnHidden(2, false);
}

void MainWindow::album_clicked() {
    QToolButton* button_sender = qobject_cast<QToolButton*>(sender());
    QString album_id = button_sender->property("id").toString();
    QString album_title = button_sender->property("title").toString();

    QUrl url_album_songs(Config::SERVER_URL + "/load_album_songs");
    QUrlQuery query;
    query.addQueryItem("album_id", album_id);
    url_album_songs.setQuery(query);

    QNetworkAccessManager *manager_album_songs = new QNetworkAccessManager(this);
    QNetworkRequest request_album_songs(url_album_songs);

    connect(manager_album_songs, SIGNAL(finished(QNetworkReply*)), this, SLOT(load_songs_finished(QNetworkReply*)));

    manager_album_songs->get(request_album_songs);

    ui->label_playlist_title->setText(album_title);
    ui->tableSongs->setColumnHidden(1, true);
    ui->tableSongs->setColumnHidden(2, true);
}

void MainWindow::artworkDownloaded(QNetworkReply* reply) {
    QByteArray buffer = reply->readAll();
    reply->deleteLater();
    QPixmap pixmap;
    pixmap.loadFromData(buffer);
    pixmap = pixmap.scaled(ui->labelArtwork->width(),ui->labelArtwork->height(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
    ui->labelArtwork->setPixmap(pixmap);
}



void MainWindow::on_tableSongs_cellDoubleClicked(int row, int column)
{
    play_song(row);
}

void MainWindow::play_song(int index) {
    std::cout << "Test";
    player->stop();
    int index_max = ui->tableSongs->rowCount();
    if (index<0) {
        index += index_max;
    }
    if (index>=index_max) {
        index -= index_max;
    }
    song_index = index;
    song_google_id = ui->tableSongs->item(index, 3)->text();
    ui->tableSongs->selectRow(index);
    ui->labelTitle->setText(ui->tableSongs->item(index, 0)->text());
    ui->labelArtist->setText(ui->tableSongs->item(index, 1)->text());
    ui->buttonLike->setProperty("active", 0);
    ui->buttonLike->setIcon(QIcon(":/resources/icons/heart_empty.png"));
    QString song_url = ui->tableSongs->item(index, 3)->text();
    QUrl album_url = QUrl(ui->tableSongs->item(index, 4)->text());
    player->setSource(QUrl(song_url));
    player->play();
    artworkManager->get(QNetworkRequest(album_url));
    if (connected) {
        QUrl url(Config::SERVER_URL + "/check_like");
        QUrlQuery query;
        query.addQueryItem("username", username);
        query.addQueryItem("song_url", song_google_id);
        url.setQuery(query);

        QNetworkAccessManager* managerLike = new QNetworkAccessManager(this);
        QNetworkRequest request(url);

        connect(managerLike, SIGNAL(finished(QNetworkReply*)), this, SLOT(check_like_finished(QNetworkReply*)));

        managerLike->get(request);
    }

}

void MainWindow::set_volume(int volume) {
    audioOutput->setVolume(float(volume)/100.0);
}

void MainWindow::update_position(qint64 position) {
    if (!ui->sliderSeekbar->isSliderDown()) {
        ui->sliderSeekbar->setSliderPosition(float(player->position())/float(player->duration())*100);
        ui->labelStartTime->setText(secondsToDuration(position/1000));
    }
}

void MainWindow::update_slider(int value) {
    if (value==100) {
        update_status(QMediaPlayer::EndOfMedia);
    }
    if (ui->sliderSeekbar->isSliderDown()) {
        ui->labelStartTime->setText(secondsToDuration(float(ui->sliderSeekbar->value())/100.0*float(player->duration())/1000.0));
    }
}

void MainWindow::update_status(QMediaPlayer::MediaStatus status) {
    qDebug() << status;
    switch (status) {

        case 0:
            //enable_buttons(false);
        case 1:
            enable_buttons(true);
            ui->labelStartTime->setText("0:00");
            ui->labelEndTime->setText("0:00");

            break;
        case 5:
            //enable_buttons(true);
            ui->labelEndTime->setText(secondsToDuration(player->duration()/1000));
            break;
        case 6:
            play_song(song_index+1);
    }
}

void MainWindow::update_state(QMediaPlayer::PlaybackState state) {
    switch (state) {
        case QMediaPlayer::StoppedState:
            ui->buttonPlay->setIcon(QIcon(":/resources/icons/play.png"));
            break;
        case QMediaPlayer::PlayingState:
            ui->buttonPlay->setIcon(QIcon(":/resources/icons/pause.png"));
            break;
        case QMediaPlayer::PausedState:
            ui->buttonPlay->setIcon(QIcon(":/resources/icons/play.png"));
            break;
        default:
            break;
    }
}

void MainWindow::on_buttonPlay_clicked()
{
    if (song_index == -1) {
        play_song(0);
    }
    else {
        switch (player->playbackState()) {
            case 0:
                player->play();
                break;
            case 1:
                player->pause();
                break;
            case 2:
                player->play();
                break;
        }
    }
}

void MainWindow::enable_buttons(bool enable) {
    if (enable) {
        ui->buttonPlay->setEnabled(true);
        ui->buttonNext->setEnabled(true);
        ui->buttonPrevious->setEnabled(true);
        ui->buttonLike->setEnabled(true);
    }
    else {
        ui->buttonPlay->setEnabled(false);
        ui->buttonNext->setEnabled(false);
        ui->buttonPrevious->setEnabled(false);
        ui->buttonLike->setEnabled(false);
    }
}

void MainWindow::on_buttonNext_clicked()
{
    play_song(song_index+1);
}

void MainWindow::on_buttonPrevious_clicked()
{

    if (song_index == -1) {
        play_song(-1);
    }
    else {
        play_song(song_index-1);
    }
}

void MainWindow::menuClicked(QAction* action) {
    if (action->text()=="Log in") {
        create_login_window();
    }
    else if (action->text()=="Sign up") {
        create_signup_window();
    }
    else if (action->text()=="Log out") {
        ui->buttonActions->setText("Guest");
        username = "Guest";
        ui->buttonLike->hide();
        ui->buttonLibrary->hide();
        connected = false;
        menu->clear();
        menu->addAction(tr("Log in"));
        menu->addAction(tr("Sign up"));

    }
    else if (action->text()=="Account") {
        create_account_window();
    }
}

void MainWindow::on_lineSearch_textChanged(const QString &arg1)
{
    QString search_query = ui->lineSearch->text();
    QUrl url(Config::SERVER_URL + "/search?q=" + QUrl::toPercentEncoding(search_query));

    QNetworkAccessManager *managerSearch = new QNetworkAccessManager(this);
    QNetworkRequest request(url);
    connect(managerSearch, SIGNAL(finished(QNetworkReply*)), this, SLOT(load_songs_finished(QNetworkReply*)));
    managerSearch->get(request);
}

void MainWindow::on_sliderVolume_valueChanged(int value)
{
    QSlider* slider = ui->sliderVolume;
    if (value==0) {
        ui->labelVolume->setPixmap(QPixmap(":/resources/icons/mute.png"));
    }
    else {
        ui->labelVolume->setPixmap(QPixmap(":/resources/icons/volume.png"));
    }
    QToolTip::showText(ui->sliderVolume->mapToGlobal(QPoint(slider->width()*value/100, 0)), QString::number(ui->sliderVolume->value()));
}


void MainWindow::on_buttonHome_clicked()
{
    ui->label_playlist_title->setText("Home");
    ui->lineSearch->hide();
    ui->contents->setCurrentIndex(1);
}


void MainWindow::on_buttonSearch_clicked()
{
    ui->label_playlist_title->setText("Search Results");
    ui->lineSearch->show();
    on_lineSearch_textChanged("");
    ui->lineSearch->setFocus();
    ui->contents->setCurrentIndex(0);
    ui->tableSongs->setColumnHidden(1, false);
    ui->tableSongs->setColumnHidden(2, false);
}

void MainWindow::set_fonts() {
    ui->label_playlist_title->setFont(font_p);
    ui->buttonHome->setFont(font_p);
    ui->buttonSearch->setFont(font_p);
    ui->buttonLibrary->setFont(font_p);
}


void MainWindow::on_buttonLike_clicked()
{
    QUrl url;
    if (ui->buttonLike->property("active") == 0) {
        ui->buttonLike->setProperty("active", 1);
        ui->buttonLike->setIcon(QIcon(":/resources/icons/heart_filled.png"));
        url = QUrl(Config::SERVER_URL + "/add_like");
        qDebug() << "Adding like";
    }
    else {
        ui->buttonLike->setProperty("active", 0);
        ui->buttonLike->setIcon(QIcon(":/resources/icons/heart_empty.png"));
        url = QUrl(Config::SERVER_URL + "/remove_like");
        qDebug() << "Removing like";
    }

    QJsonObject obj;
    obj.insert("username", username);
    obj.insert("song_url", song_google_id);

    QJsonDocument doc(obj);

    QByteArray data = doc.toJson();
    QNetworkAccessManager *managerLike = new QNetworkAccessManager(this);
    qDebug() << url;
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    managerLike->post(request, data);
}

void MainWindow::check_like_finished(QNetworkReply* reply) {
    QByteArray buffer = reply->readAll();
    QJsonDocument jsonDoc(QJsonDocument::fromJson(buffer));
    QJsonObject jsonObj = jsonDoc.object();
    if (jsonObj.value("exists").toInt() == 0) {
        ui->buttonLike->setProperty("active", 0);
        ui->buttonLike->setIcon(QIcon(":/resources/icons/heart_empty.png"));
    }
    else {
        ui->buttonLike->setProperty("active", 1);
        ui->buttonLike->setIcon(QIcon(":/resources/icons/heart_filled.png"));
    }
}

void MainWindow::on_buttonLibrary_clicked()
{
    ui->label_playlist_title->setText("Library");
    ui->lineSearch->hide();
    ui->contents->setCurrentIndex(0);
    ui->tableSongs->setColumnHidden(1, false);
    ui->tableSongs->setColumnHidden(2, false);
    QUrl url(Config::SERVER_URL + "/load_library");
    QUrlQuery query;
    query.addQueryItem("username", username);
    url.setQuery(query);
    QNetworkAccessManager *managerLibrary = new QNetworkAccessManager(this);
    QNetworkRequest request(url);
    connect(managerLibrary, SIGNAL(finished(QNetworkReply*)), this, SLOT(load_songs_finished(QNetworkReply*)));
    managerLibrary->get(request);
}

