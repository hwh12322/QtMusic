#include "musicinfo.h"
#include <QMediaPlayer>
#include <QMediaContent>
#include <QMediaMetaData>

MusicInfo::MusicInfo()
{

}

void MusicInfo::setDir(QString dirStr)
{
    this->dir = dirStr;
}
