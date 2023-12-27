#include "lyric.h"
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QRegularExpression>
#include <QRegularExpressionMatchIterator>
#include <QDebug>

Lyric::Lyric()
{

}

int Lyric::getIndex(qint64 pos)
{
    //采用二分查找
    //时间复杂度O(logn)
    //qDebug() << "Searching index for position:" << pos;
    //qDebug() << "Lyric timestamps:" << position;
    int lt, rt, mid;
    lt = 0; rt = position.count();
    while (lt < rt-1)
    {
        mid = (lt+rt)>>1;
        if (position[mid] > pos) rt = mid;
        else lt = mid;
    }
    return lt;
}

qint64 Lyric::getPostion(int index)
{
    if (index < position.size())
        return position[index];
    else
        return 0;
}


void Lyric::getFromFile(QString dir)
{
    qDebug() << "Lyric dir:" << dir << endl;
    this->filedir = dir;
    //this->offset
    this->line.clear();
    this->position.clear();

    QFile file(dir);
    if (!file.exists()) return;
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream read(&file);
    QChar ch;
    bool flag;
    qint64 mm;
    double ss;
    QVector<qint64> muli;
    QMap<qint64, QString> ans;
    QMap<qint64, QString>::iterator it;
    QString sign;
    QString val;
    QString str;
    while (!read.atEnd())
    {
        for (;;)
        {
            read >> ch;
            if (ch == '[' || read.atEnd())
            {
                if (muli.count())
                {
                    for (int i = 0; i < muli.count(); ++i)
                        ans.insert(muli[i], str);
                }
                str = "";
                muli.clear();
                break;
            }
            str = str+ch;
        }
        if (read.atEnd()) break;
        for (;;)
        {
            read >> ch;
            if (ch == ':' || read.atEnd()) break;
            sign = sign+ch;
        }
        if (read.atEnd()) break;
        for (;;)
        {
            read >> ch;
            if (ch == ']' || read.atEnd())
            {
                mm = sign.toLongLong(&flag, 10);
                //判断sign是否是整数
                if (flag)
                {
                    ss = val.toDouble(&flag);
                    if (flag)
                        muli.push_back((qint64)(ss*1000)+mm*60*1000);
                }
                break;
            }
            val = val+ch;
        }
        sign = "";
        val = "";
    }
    for (it = ans.begin(); it != ans.end(); ++it)
    {
        this->position.push_back(it.key());
        this->line.push_back(it.value());
    }
}

QString Lyric::getLineAt(int index)
{
    return line[index];
}

int Lyric::getCount()
{
    return line.count();
}

void Lyric::parseFromString(const QString &lyricData)
{
    // 清除旧数据
    line.clear();
    position.clear();

    // 正则表达式解析歌词和时间标签
    QRegularExpression regex(R"(\[(\d+):(\d+(?:\.\d+)?)\](.*))");
    QRegularExpressionMatchIterator it = regex.globalMatch(lyricData);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        qint64 minutes = match.captured(1).toInt();
        double seconds = match.captured(2).toDouble();
        QString lyricText = match.captured(3).trimmed();

        qint64 time = (minutes * 60 + seconds) * 1000; // 转换时间为毫秒
        position.push_back(time);
        line.push_back(lyricText);
    }
}
