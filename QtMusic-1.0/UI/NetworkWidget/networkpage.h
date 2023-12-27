#ifndef NETWORKPAGE_H
#define NETWORKPAGE_H

#include <Core/musicinfo.h>
#include <QWidget>
#include <QMap>

class LabelButton;
class QNetworkAccessManager;
class QNetworkReply;
class QLineEdit;
class SearchList;

class QTextEdit;

class NetworkPage : public QWidget
{
    Q_OBJECT
public:
    explicit NetworkPage(QWidget *parent = 0);

signals:
    void mouseEnter();
    void tryAddSongToCurrentList(MusicInfo musicInfo);
    void lyricsAvailable(const QString &lyrics);

protected:
    void enterEvent(QEvent *e);//鼠标进入事件

private slots:
    void searchSongs();
    void searchFinished(QNetworkReply *reply);
    void addSongsToCurrentList(QVector<int> &songs);
    void getSongFinished(QNetworkReply *reply);


public slots:
    void fetchLyrics(int songId);
    void lyricsFetched(QNetworkReply* reply);
    void onDownloadRequested(const QString& url);
    void onDownloadFinished(QNetworkReply* reply);

private:
    void tryAddSongToCurrentList(int which, int id);

private:
    LabelButton *logoButton;
    LabelButton *searchButton;
    QLineEdit *keyText;
    QNetworkAccessManager *accessManager;
    QNetworkAccessManager *songManager;
    QNetworkAccessManager *coverImageManager;
    SearchList *searchList;
    int nowPage;
    int pageCount;
    QVector<int> songId;
    QMap<QNetworkReply *, MusicInfo> songHash;
};

#endif // NETWORKPAGE_H
