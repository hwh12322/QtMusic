#ifndef SEARCHLIST_H
#define SEARCHLIST_H

#include <QTableWidget>
#include <QVector>

class SearchList : public QTableWidget
{
    Q_OBJECT
public:
    SearchList(QWidget *parent = 0);
    void addSong(const QString &name, const QString &artist, const QString &albumName,const QString &coverUrl,const QString &url);
    void clearSongs();

signals:
    void rightClicked();
    void addSongsToCurrentList(QVector<int> &songs);
    void downloadRequested(const QString& url);

private slots:
    void addSongsToCurrentList();
    void onDownloadActionTriggered();

private:
    void resizeEvent(QResizeEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);//右击事件
};

#endif // SEARCHLIST_H
