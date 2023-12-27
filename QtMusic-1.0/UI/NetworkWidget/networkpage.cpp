#include "networkpage.h"
#include "UI/NetworkWidget/labelbutton.h"
#include "UI/NetworkWidget/searchlist.h"
#include "UI/musicwindow.h"
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QEvent>
#include <QTextEdit>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QSqlQuery>
#include <QtDebug>
#include <QUrlQuery>
#include <QFileDialog>
NetworkPage::NetworkPage(QWidget *parent) : QWidget(parent)
{
    accessManager = new QNetworkAccessManager(this);
    connect(accessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(searchFinished(QNetworkReply*)));


    QPalette palette;//调色板
    palette.setColor(QPalette::Background, QColor(244, 244, 244, 102));//设置调色板参数为背景色，RGB颜色为深蓝色
    this->setAutoFillBackground(true);//将组件的背景设为可调
    this->setPalette(palette);//将调色板应用于组件

    // 设置整个 NetworkPage 的透明度
       this->setStyleSheet("background: transparent;");

       // 对于整个 NetworkPage 内的所有子部件，设置背景为透明
       setAttribute(Qt::WA_TranslucentBackground);
       setStyleSheet("QWidget { background: transparent; }");


    palette.setColor(QPalette::Background, QColor(12, 132, 132));//设置调色板参数为背景色，RGB颜色为深蓝色
    logoButton = new LabelButton(this);
    logoButton->setFixedSize(40, 40);
    logoButton->setIcon(":/images/network/netease_icon.jpg");
    keyText = new QLineEdit();
    keyText->setFixedSize(300, 40);
    keyText->setStyleSheet("font: bold large \"宋体\";"
                             "font-size:25px;"
                             "selection-color:rgba(244,244,244,100%);"
                             "background-color:rgba(244,244,244,0%);"
                             "border:2px solid rgb(255, 255, 255,50%);"
                             "border-radius:8px;");
    connect(keyText, SIGNAL(returnPressed()), this, SLOT(searchSongs()));

    searchButton = new LabelButton(this);
    searchButton->setFixedSize(40, 40);
    searchButton->setIcon(":/images/network/searchbutton_icon.jpg");
    connect(searchButton, SIGNAL(clicked()), this, SLOT(searchSongs()));

    QHBoxLayout *midLayout = new QHBoxLayout();
    midLayout->addStretch();
    midLayout->addWidget(logoButton);
    midLayout->addWidget(keyText);
    midLayout->addWidget(searchButton);
    midLayout->addStretch();

    searchList = new SearchList(this);
    connect(searchList, SIGNAL(addSongsToCurrentList(QVector<int>&)), this, SLOT(addSongsToCurrentList(QVector<int>&)));
    connect(searchList, SIGNAL(downloadRequested(QString)), this,SLOT(onDownloadRequested(QString)));

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addLayout(midLayout);
    mainLayout->addWidget(searchList);
    //mainLayout->addStretch();
    this->setLayout(mainLayout);

    this->nowPage = 0;
    this->pageCount = 100;
}

void NetworkPage::searchSongs()
{
    QString searchUrl = "http://47.98.240.60:3000/cloudsearch";
    QUrl url(searchUrl);
    QUrlQuery query;
    query.addQueryItem("keywords", keyText->text());
    url.setQuery(query);

    QNetworkRequest request(url);
    accessManager->get(request);  // 使用GET请求
}


void NetworkPage::searchFinished(QNetworkReply *reply)
{
    QByteArray data = reply->readAll();
    QJsonParseError err;
    QJsonDocument json = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError)
    {
        qDebug() << "JSON Parse Error:" << err.errorString();
        return;
    }

    QJsonObject obj = json.object();
    QJsonArray songsArray = obj["result"].toObject()["songs"].toArray();

    searchList->clearSongs();
    songId.clear();

    for (const QJsonValue &value : songsArray)
    {
        QJsonObject songObject = value.toObject();
        QString name = songObject["name"].toString();
        QString artist = songObject["ar"].toArray()[0].toObject()["name"].toString();
        QString albumName = songObject["al"].toObject()["name"].toString(); // 获取专辑名
        QString coverUrl = songObject["al"].toObject()["picUrl"].toString(); // 获取专辑封面url
        int id = songObject["id"].toInt();
        QString songUrl = "http://music.163.com/song/media/outer/url?id=" + QString::number(id) + ".mp3";
        searchList->addSong(name, artist,albumName,coverUrl,songUrl);
        songId.push_back(id);
    }

}



void NetworkPage::addSongsToCurrentList(QVector<int> &songs)
{
    qDebug() << "addSongsToCurrentList called, songs size:" << songs.size();
    for (int i = 0; i < songs.size(); ++i){
        tryAddSongToCurrentList(songs[i], songId[songs[i]]);
        //qDebug() << "2";
    }
}

void NetworkPage::tryAddSongToCurrentList(int which, int id)
{
    // 获取歌曲URL的API地址
    QString songUrl = "http://music.163.com/song/media/outer/url?id=" + QString::number(id) + ".mp3";

    // 更新 MusicInfo 对象
    MusicInfo musicInfo;
    musicInfo.setId(id);
    musicInfo.setName(searchList->item(which, 0)->text());
    musicInfo.setArtist(searchList->item(which, 1)->text());
    musicInfo.setDir(songUrl); // 这里直接设置歌曲的URL
    musicInfo.setCoverUrl(searchList->item(which, 3)->text());
    //qDebug() << "1" << searchList->item(which, 0)->text();
    //qDebug() << "2" << searchList->item(which, 1)->text();

    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, &NetworkPage::getSongFinished);
    // 触发信号，你可能会有一个槽函数来处理这个信号并播放音乐
    emit tryAddSongToCurrentList(musicInfo);

}


void NetworkPage::getSongFinished(QNetworkReply *reply)
{
    qDebug() << "111";
    QByteArray data = reply->readAll();
    QJsonParseError err;
    QJsonDocument json = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError)
    {
        qDebug() << "JSON Parse Error:" << err.errorString();
        return;
    }

    QJsonObject obj = json.object();
    QJsonArray songsArray = obj["songs"].toArray();
    if (!songsArray.isEmpty())
    {
        QJsonObject songObject = songsArray.first().toObject();
        QJsonObject albumObject = songObject["al"].toObject();
        QString albumName = albumObject["name"].toString(); // 提取专辑名称
        QString picUrl = albumObject["picUrl"].toString();
        if (!albumName.isEmpty())
        {
            MusicInfo musicInfo = songHash.value(reply);
            musicInfo.setArtist(albumName); // 设置专辑名为artist属性
            musicInfo.setCoverUrl(picUrl); // 设置封面 URL
            // 如果需要，继续设置MusicInfo对象的其他属性
            // ...
            //emit tryAddSongToCurrentList(musicInfo);
        }
        else
        {
            qDebug() << "Album name not found in the response.";
        }
    }
    else
    {
        qDebug() << "No songs found in the response.";
    }
}

void NetworkPage::fetchLyrics(int songId) {
    QUrl url(QString("http://47.98.240.60:3000/lyric?id=%1").arg(songId));
    QNetworkRequest request(url);
    // 设置请求头，如果API需要的话
    // request.setRawHeader(...);

    // 使用QNetworkAccessManager发送请求
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, &NetworkPage::lyricsFetched);
    manager->get(request);
}



void NetworkPage::lyricsFetched(QNetworkReply* reply) {
    if (!reply->error()) {
        QByteArray response = reply->readAll();
        QJsonDocument jsonResponse = QJsonDocument::fromJson(response);
        if (!jsonResponse.isNull()) {
            QString lyrics = jsonResponse.object().value("lrc").toObject().value("lyric").toString();

            // 打印歌词到调试输出
            qDebug() << "Fetched Lyrics:" << lyrics;

            emit lyricsAvailable(lyrics);
        }
    }
    reply->deleteLater();
}

void NetworkPage::onDownloadRequested(const QString& url) {
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, &NetworkPage::onDownloadFinished);
    manager->get(request);
}

void NetworkPage::onDownloadFinished(QNetworkReply* reply) {
    if (reply->error() == QNetworkReply::NoError) {
        qDebug() << "Download successful";
        qDebug() << "HTTP Status Code:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        QByteArray data = reply->readAll();

        // 处理重定向
        QVariant redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
        if (!redirectionTarget.isNull()) {
            QUrl newUrl = reply->url().resolved(redirectionTarget.toUrl());
            qDebug() << "Redirected to:" << newUrl.toString();
            onDownloadRequested(newUrl.toString());
            return;
        }

        qDebug() << "Received data size:" << data.size();

        // 默认文件名和文件类型设置为MP3
        QString suggestedFileName = QFileInfo(reply->url().path()).fileName();
        QString filePath = QFileDialog::getSaveFileName(this, tr("保存文件"), QDir::homePath() + "/" + suggestedFileName, tr("MP3 文件 (*.mp3)"));

        if (!filePath.isEmpty()) {
            if (!filePath.endsWith(".mp3", Qt::CaseInsensitive)) {
                filePath += ".mp3"; // 确保文件扩展名为.mp3
            }
            QFile file(filePath);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(data);
                file.close();
                qDebug() << "File saved to:" << filePath;
            } else {
                qDebug() << "Failed to open file for writing";
            }
        } else {
            qDebug() << "No file path selected";
        }
    } else {
        qDebug() << "Download error:" << reply->errorString();
    }
    reply->deleteLater();
}



void NetworkPage::enterEvent(QEvent *e)
{
    emit mouseEnter();
}
