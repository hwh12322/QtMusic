#ifndef MUSICWINDOW_H
#define MUSICWINDOW_H

#include "UI/ToolWidget/mainwindow.h"
#include "Core/musicinfo.h"

class MySystemTrayIcon;
class TopBar;
class QStackedWidget;
class LyricLabel;
class NetworkPage;
class MusicPage;
class BottomBar;
class MyPlayer;
class QNetworkAccessManager;
class QNetworkReply;

class MusicWindow : public MainWindow
{
    Q_OBJECT

public:
    MusicWindow(QWidget *parent = 0);
    ~MusicWindow();

public slots:
    void setBackgroud(QString dir);
    void aboutQtMusic();


private slots:
    void clickPlay();
    void playPre();
    void playNext();
    void playerStateChanged(int state);
    void musicChanged(QString listName, int index);
    void tryToCreateList(QString name);
    void addMusics();
    void addMusic(MusicInfo musicInfo);
    void removeTheMusic(QString listName, int index);
    void removeAllMusics(QString listName);
    void moveMusic(QString listName, int from, int to);
    void deleteList(QString name);
    void updateLyrics(const QString& lyrics);
    void onCoverImageDownloaded(QNetworkReply* reply);  // 添加新的槽函数声明

private:
    void readData();
    void paintEvent(QPaintEvent *);
    void dealMouse();
    int extractSongIdFromUrl(const QString& url);

private:
    QPixmap currentBackground;
    MySystemTrayIcon *systemTrayIcon;
    TopBar *topBar;
    QStackedWidget *fuctionPage;
    LyricLabel *lyricLabel;
    NetworkPage *networkPage;
    MusicPage *musicPage;
    BottomBar *bottomBar;
    MyPlayer *player;
    QNetworkAccessManager *networkManager;
    int retryCount = 0;
    const int maxRetryCount = 100; // 设置最大重试次数
    QString currentCoverUrl; // 用于存储当前正在下载的封面图 URL
    void downloadCoverImage(const QString &url);
};

#endif // MUSICWINDOW_H
