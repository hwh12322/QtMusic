#include "searchlist.h"
#include "UI/ToolWidget/mymenu.h"
#include <QScrollBar>
#include <QHeaderView>
#include <QPainter>
#include <QContextMenuEvent>

#include <QtDebug>

SearchList::SearchList(QWidget *parent)
        : QTableWidget(parent)
{
    this->setFrameStyle(QFrame::NoFrame);//去边框
    //this->setSelectionMode(QAbstractItemView::NoSelection);
    this->setStyleSheet("background:rgba(244,244,244,40%);"
                        "selection-background-color:rgba(128,128,128,40%);"
                        "selection-color:rgba(255,255,255,100%)");
    this->verticalScrollBar()->setStyleSheet(
            "QScrollBar:vertical"
            "{"
                "width:12px;"
                "background:rgba(0,0,0,0%);"
                "margin:0px,0px,0px,0px;"
                "padding-top:9px;"
                "padding-bottom:9px;"
            "}"
            "QScrollBar::handle:vertical"
            "{"
                "width:8px;"
                "background:rgba(0,0,0,25%);"
                " border-radius:4px;"
                "min-height:20;"
            "}"
            "QScrollBar::handle:vertical:hover"
            "{"
                "width:8px;"
                "background:rgba(0,0,0,50%);"
                " border-radius:4px;"
                "min-height:20;"
            "}"
            "QScrollBar::add-line:vertical"
            "{"
                "height:9px;width:8px;"
                "subcontrol-position:bottom;"
                "border-image:url(:/images/3.png);"
            "}"
            "QScrollBar::sub-line:vertical"
            "{"
                "height:9px;width:8px;"
                "subcontrol-position:top;"
                "border-image:url(:/images/1.png);"
            "}"
            "QScrollBar::add-line:vertical:hover"
            "{"
                "height:9px;width:8px;"
                "subcontrol-position:bottom;"
                "border-image:url(:/images/4.png);"
            "}"
            "QScrollBar::sub-line:vertical:hover"
            "{"
                "height:9px;width:8px;"
                "subcontrol-position:top;"
                "border-image:url(:/images/2.png);"
            "}"
            "QScrollBar::add-page:vertical,QScrollBar::sub-page:vertical"
            "{"
                "background:rgba(0,0,0,10%);"
                "border-radius:4px;"
            "}"
            );


    setColumnCount(5);
    setSelectionBehavior(QAbstractItemView::SelectRows);//设置选中一行
    setEditTriggers(QAbstractItemView::NoEditTriggers);//设置不可修改
    setShowGrid(false);//格子线不显示
    this->hideColumn(3); // 隐藏第四列
    this->hideColumn(4); // 隐藏第五列
    verticalHeader()->setVisible(false);
    horizontalHeader()->setVisible(true); // 确保水平表头是可见的
    QStringList headers;
    headers << "歌曲名称" << "歌手" << "专辑" << "封面URL"; // 添加第四个标题用于封面URL
    setHorizontalHeaderLabels(headers);

    setFocusPolicy(Qt::NoFocus);
    horizontalHeader()->setHighlightSections(false);
    horizontalHeader()->resizeSection(0, 80);
    horizontalHeader()->setStretchLastSection(true);//占满表头

    MyMenu *listMenu = new MyMenu(this);
    QAction *addToCurrentList = new QAction("添加到当前列表", listMenu);
    QAction *downloadAction = new QAction("下载此歌曲", listMenu);
    connect(addToCurrentList, SIGNAL(triggered(bool)), this, SLOT(addSongsToCurrentList()));
    connect(downloadAction, SIGNAL(triggered(bool)), this, SLOT(onDownloadActionTriggered()));
    listMenu->addAction(addToCurrentList);
    listMenu->addAction(downloadAction);
    connect(this, SIGNAL(rightClicked()), listMenu, SLOT(menuVisiable()));
}

void SearchList::addSong(const QString &name, const QString &artist, const QString &albumName,const QString &coverUrl,const QString &url)
{
    int now = this->rowCount();
    this->insertRow(now);
    QTableWidgetItem *itemName = new QTableWidgetItem(name);
    this->setItem(now, 0, itemName);
    QTableWidgetItem *itemArtist = new QTableWidgetItem(artist);
    this->setItem(now, 1, itemArtist);
    QTableWidgetItem *itemAlbum = new QTableWidgetItem(albumName);  // 修改此处为专辑名称
    this->setItem(now, 2, itemAlbum);  // 修改此处为专辑名称
    QTableWidgetItem *itemAlbumurl = new QTableWidgetItem(coverUrl);  // 修改此处为专辑coverUrl
    this->setItem(now, 3, itemAlbumurl);  // 修改此处为专辑coverUrl
    QTableWidgetItem *itemurl = new QTableWidgetItem(url);  // 修改此处为歌曲url
    this->setItem(now, 4, itemurl);  // 修改此处为歌曲url
}


void SearchList::clearSongs()
{
    int len = this->rowCount();
    for (int i = 0; i < len; ++i)
    {

        for (int j = 0; j < 3; ++ j)
        {
            QTableWidgetItem *item = this->takeItem(0, j);
            delete item;
        }
        this->removeRow(0);
    }
}

void SearchList::addSongsToCurrentList()
{
    QList<QTableWidgetItem*>items = this->selectedItems();
    int cnt = items.count(), row;
    QVector<int> songs;
    for(int i = 0; i < cnt; i += 4)
    {
       row = this->row(items.at(i));//获取选中的行
       songs.push_back(row);
    }
    emit addSongsToCurrentList(songs);
}

void SearchList::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    int w = this->width();
    this->horizontalHeader()->resizeSection(0, w/3);
    this->horizontalHeader()->resizeSection(1, w/3);
    this->horizontalHeader()->resizeSection(2, w/3);
}

void SearchList::contextMenuEvent(QContextMenuEvent *event)
{
    QPoint point = event->pos();//得到窗口坐标
    QTableWidgetItem *item = this->itemAt(point);
    if(item != NULL)
    {
        //clickEvent();
        emit rightClicked();
    }
    //QWidget::contextMenuEvent(event);
}

void SearchList::onDownloadActionTriggered() {
    int row = currentRow();
    if (row < 0) return;

    QString url = item(row, 4)->text();
    qDebug() << "555" << url;
    emit downloadRequested(url);
}
