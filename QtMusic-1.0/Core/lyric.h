#ifndef LYRIC_H
#define LYRIC_H

#include <QVector>

class Lyric
{
    public:
        Lyric();
        void getFromFile(QString dir);
        QString getLineAt(int index);
        int getCount();
        int getIndex(qint64 pos);
        qint64 getPostion(int index);
        void parseFromString(const QString &lyricData);
    private:
        QString filedir;
        double offset;
        QVector<QString> line;
        QVector<qint64> position;
};

#endif // LYRIC_H
