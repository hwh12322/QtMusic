#ifndef MUSICINFO_H
#define MUSICINFO_H

#include <QString>

class MusicInfo
{
    public:
        MusicInfo();
        void setId(int idVal) { id = idVal; }
        void setDir(QString dirStr);
        void setName(QString nameStr) {name = nameStr;}
        void setArtist(QString artistStr) {artist = artistStr;}
        void setCoverUrl(QString url) { coverUrl = url; } // 设置封面 URL
        QString getDir() {return dir;}
        QString getName() {return name;}
        QString getArtist() {return artist;}
        QString getCoverUrl() const { return coverUrl; } // 获取封面 URL
        int getId() const { return id; }
    private:
        int id;
        QString dir;
        QString name;
        QString artist;
        QString coverUrl; // 专辑封面 URL
};

#endif // MUSICINFO_H
