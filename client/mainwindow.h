#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "qtoolbutton.h"
#include <QMainWindow>
#include <QtSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>
#include <QDebug>
#include <login.h>
#include <signup.h>
#include <account.h>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonValue>
#include <QJsonValueRef>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QMenu>
#include <QToolTip>
#include <QProxyStyle>
#include <QFont>
#include <QFontDatabase>
#include <QMediaMetaData>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QString username;
    QFont font_h1{"Segoe", 12, QFont::Bold};
    QFont font_p{"Segoe", 12};
    bool connected = false;
    int song_index = -1;
    QString song_google_id = "";
    QMenu* menu = new QMenu(this);
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void connect_account(QString username);
    void create_login_window();
    void create_signup_window();
    void create_account_window();
    void create_error_window(QString message);
    void create_information_window(QString message);
    void on_sliderSeekbar_sliderReleased();
    void check_like_finished(QNetworkReply* reply);
    void load_songs_finished(QNetworkReply* reply);
    void loadPlaylistsFinished(QNetworkReply* reply);
    void loadAlbumsFinished(QNetworkReply* reply);
    void on_tableSongs_cellDoubleClicked(int row, int column);
    void play_song(int index);
    void set_volume(int volume);
    void update_position(qint64 position);
    void update_slider(int position);
    void update_status(QMediaPlayer::MediaStatus status);
    void update_state(QMediaPlayer::PlaybackState state);
    void on_buttonPlay_clicked();
    void enable_buttons(bool enable);
    void on_buttonNext_clicked();
    void on_buttonPrevious_clicked();
    void artworkDownloaded(QNetworkReply* reply);
    void playlistArtworkDownloaded();
    void menuClicked(QAction* action);
    void on_lineSearch_textChanged(const QString &arg1);
    void playlist_clicked();
    void album_clicked();
    void on_sliderVolume_valueChanged(int value);

    void on_buttonHome_clicked();

    void on_buttonSearch_clicked();
    void set_fonts();

    void on_buttonLike_clicked();

    void on_buttonLibrary_clicked();

private:
    Ui::MainWindow *ui;
    QMediaPlayer* player = new QMediaPlayer;
    QAudioOutput* audioOutput = new QAudioOutput;
    QNetworkAccessManager* artworkManager = new QNetworkAccessManager;
};

class JumpSlider : public QProxyStyle
{
public:
    using QProxyStyle::QProxyStyle;

    int styleHint(QStyle::StyleHint hint, const QStyleOption* option = 0, const QWidget* widget = 0, QStyleHintReturn* returnData = 0) const
    {
        if (hint == QStyle::SH_Slider_AbsoluteSetButtons)
            return (Qt::LeftButton | Qt::MiddleButton | Qt::RightButton);
        return QProxyStyle::styleHint(hint, option, widget, returnData);
    }
};

#endif // MAINWINDOW_H
