#include "lyriclabel.h"
#include "Core/lyric.h"
#include "UI/ToolWidget/mymenu.h"
#include <QScroller>
#include <QEvent>
#include <QScrollPrepareEvent>
#include <QPainter>
#include <QApplication>
#include <QFontDialog>
#include <QColorDialog>
#include <QTime>
#include <QTimer>
#define WHEEL_SCROLL_OFFSET 50000.0

#include <QDebug>

LyricLabel::LyricLabel(bool touch, QWidget *parent)
    :AbstractWheelWidget(touch, parent)
{
    lyric = new Lyric();
    lyricFont = new QFont("宋体", 12, QFont::Bold);
    lyricNormal = new QColor(255, 255, 255);
    lyricHighlight = new QColor(255, 220, 26);
    connect(this, SIGNAL(changeTo(int)), this, SLOT(changeToEvent(int)));

    lyricTimer = new QTimer(this);
       connect(lyricTimer, &QTimer::timeout, this, &LyricLabel::updateLyricPosition);

    MyMenu *menu = new MyMenu(this);
    //QAction *selectLyric = new QAction("关联本地歌词", menu);
    QAction *fontSelect = new QAction("字体设置", menu);
    connect(fontSelect, SIGNAL(triggered(bool)), this, SLOT(changeFont()));
    QAction *colorNormal = new QAction("普通颜色", menu);
    connect(colorNormal, SIGNAL(triggered(bool)), this, SLOT(changeNormalColor()));
    QAction *colorHighLight = new QAction("高亮颜色", menu);
    connect(colorHighLight, SIGNAL(triggered(bool)), this, SLOT(changeHightLightColor()));
    //menu->addAction(selectLyric);
    menu->addSeparator();
    menu->addAction(fontSelect);
    menu->addAction(colorNormal);
    menu->addAction(colorHighLight);
    connect(this, SIGNAL(rightClicked()), menu, SLOT(menuVisiable()));
}

void LyricLabel::getFromFile(QString dir)
{
    lyric->getFromFile(dir);
    this->update();
}

void LyricLabel::setLyrics(const QString &lyrics) {
    if (lyric) {
        lyric->parseFromString(lyrics); // 使用Lyric类的新方法解析字符串
        this->update();  // 重绘界面以显示新歌词
    }
}

void LyricLabel::paintItem(QPainter* painter, int index, const QRect &rect)
{
    if (index == this->m_currentItem)
    {
        painter->setPen(*lyricHighlight);
        QFont font(*lyricFont);
        font.setPointSize(font.pointSize()+5);
        painter->setFont(font);
    }
    else
    {
        QPen pen = painter->pen();
        QColor color = pen.color();
        color.setRed(lyricNormal->red());
        color.setGreen(lyricNormal->green());
        color.setBlue(lyricNormal->blue());
        painter->setPen(color);
        painter->setFont(*lyricFont);
    }
    painter->drawText(rect, Qt::AlignCenter, lyric->getLineAt(index));
}

int LyricLabel::itemHeight() const
{
    QFontMetrics fm(*lyricFont);
    //qDebug() << "itemheight" << fm.height()*2.8;
    return fm.height()*2.8;
    //return 45;
}

int LyricLabel::itemCount() const
{
    return lyric->getCount();
}

void LyricLabel::postionChanged(qint64 pos)
{
    if (this->isScrolled) return;
    pos = pos+500;//歌词滚动需要500ms
    int index = lyric->getIndex(pos);
    if (index != m_currentItem)
        this->scrollTo(index);
}

void LyricLabel::setPostion(qint64 pos)
{
    int index = lyric->getIndex(pos);
    this->setCurrentIndex(index);
}

void LyricLabel::changeToEvent(int index)
{
    emit changeTo(lyric->getPostion(index));
}

void LyricLabel::changeFont()
{
    bool flag;
    *lyricFont = QFontDialog::getFont(&flag, *lyricFont, this);
    if (flag)
    {
        // the user clicked OK and font is set to the font the user selected
    }
    else
    {
        // the user canceled the dialog; font is set to the initial value
        lyricFont = new QFont("宋体", 12, QFont::Bold);
    }
}

void LyricLabel::changeNormalColor()
{
    *lyricNormal =  QColorDialog::getColor(*lyricNormal, this);
}

void LyricLabel::changeHightLightColor()
{
    *lyricHighlight =  QColorDialog::getColor(*lyricHighlight, this);
}

void LyricLabel::contextMenuEvent(QContextMenuEvent *event)
{
    emit rightClicked();
}

void LyricLabel::enterEvent(QEvent *e)
{
    emit mouseEnter();
}

void LyricLabel::startLyricScroll(qint64 totalTime)
{
    totalSongTime = totalTime;
    lyricTimer->start(1000); // 每秒更新一次歌词位置
}

void LyricLabel::stopLyricScroll() {
    lyricTimer->stop();
}

void LyricLabel::setMyPlayer(MyPlayer *player) {
    myPlayer = player;
}

void LyricLabel::updateLyricPosition() {
    if (myPlayer != nullptr) {
        qint64 currentTime = myPlayer->getCurrentPosition();
        int index = lyric->getIndex(currentTime+1000);//调整更新延迟
        //qDebug() << "currentTime index" <<index;
        if (index != m_currentItem) {
            scrollTo(index);
        }
    }
}


AbstractWheelWidget::AbstractWheelWidget(bool touch, QWidget *parent)
    : QWidget(parent), m_currentItem(0), m_itemOffset(0)
{
    lyric = new Lyric();
// ![0]
    QScroller::grabGesture(this, touch ? QScroller::TouchGesture : QScroller::LeftMouseButtonGesture);
// ![0]
    this->isScrolled = false;
    this->dosignal = true;
}

AbstractWheelWidget::~AbstractWheelWidget()
{ }

int AbstractWheelWidget::currentIndex() const
{
    return m_currentItem;
}

void AbstractWheelWidget::setCurrentIndex(int index)
{
    if (index >= 0 && index < itemCount()) {
        m_currentItem = index;
        m_itemOffset = 0;
        update();
    }
}

bool AbstractWheelWidget::event(QEvent *e)
{
    switch (e->type())
    {
// ![1]
        case QEvent::ScrollPrepare:
        {
            // We set the snap positions as late as possible so that we are sure
            // we get the correct itemHeight
            QScroller *scroller = QScroller::scroller(this);
            scroller->setSnapPositionsY( WHEEL_SCROLL_OFFSET, itemHeight() );

            QScrollPrepareEvent *se = static_cast<QScrollPrepareEvent *>(e);
            se->setViewportSize(QSizeF(size()));
            // we claim a huge scrolling area and a huge content position and
            // hope that the user doesn't notice that the scroll area is restricted
            se->setContentPosRange(QRectF(0.0, 0.0, 0.0, WHEEL_SCROLL_OFFSET * 2));
            se->setContentPos(QPointF(0.0, WHEEL_SCROLL_OFFSET + m_currentItem * itemHeight() + m_itemOffset));
            se->accept();
            return true;
        }
// ![1]
// ![2]
        case QEvent::Scroll:
        {
            QScrollEvent *se = static_cast<QScrollEvent *>(e);
            if (dosignal)//只有触发信号的滚动才进行,而且这种是人为滚动
            {
                //开始滚动
                if (se->scrollState() == QScrollEvent::ScrollStarted)
                {
                    qDebug() << "start scroll lyric" << endl;
                    this->isScrolled = true;
                }
            }
            //滚动结束
            if (se->scrollState() == QScrollEvent::ScrollFinished)
            {

                if (dosignal)
                {
                    qDebug() << "滚动到第" << m_currentItem << endl;
                    qint64 position = lyric->getPostion(m_currentItem);
                    qDebug()  << "position" << position;
                    // 发送信号
                    //emit changeTo(position);
                }
                this->isScrolled = false;
                dosignal = true;
            }

            qreal y = se->contentPos().y();
            int iy = y - WHEEL_SCROLL_OFFSET;
            int ih = itemHeight();

// ![2]
// ![3]
            // -- calculate the current item position and offset and redraw the widget
            int ic = itemCount();
            if (ic>0)
            {
                m_currentItem = iy / ih;
                //m_currentItem = iy / ih % ic;
                m_itemOffset = iy % ih;

                // take care when scrolling backwards. Modulo returns negative numbers
                /*
                if (m_itemOffset < 0) {
                    m_itemOffset += ih;
                    m_currentItem--;
                }
                */
                //if (m_currentItem < 0)
                    //m_currentItem += ic;
                    //m_currentItem = 0;
                if (m_currentItem >= ic) m_currentItem = ic-1;
            }
            // -- repaint
            update();

            se->accept();
            return true;
// ![3]
        }
        case QEvent::MouseButtonPress:
            return true;
        default:
            return QWidget::event(e);
    }
    return true;
}

void AbstractWheelWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED( event );

    // -- first calculate size and position.
    int w = width();
    int h = height();

    QPainter painter(this);
    QPalette palette = QApplication::palette();
    QPalette::ColorGroup colorGroup = isEnabled() ? QPalette::Active : QPalette::Disabled;

    // linear gradient brush
    QLinearGradient grad(0.5, 0, 0.5, 1.0);
    grad.setColorAt(0, palette.color(colorGroup, QPalette::ButtonText));
    grad.setColorAt(0.2, palette.color(colorGroup, QPalette::Button));
    grad.setColorAt(0.8, palette.color(colorGroup, QPalette::Button));
    grad.setColorAt(1.0, palette.color(colorGroup, QPalette::ButtonText));
    grad.setCoordinateMode( QGradient::ObjectBoundingMode );
    QBrush gBrush( grad );
/*
    // paint a border and background
    painter.setPen(palette.color(colorGroup, QPalette::ButtonText));
    painter.setBrush(gBrush);
    // painter.setBrushOrigin( QPointF( 0.0, 0.0 ) );
    //painter.drawRect( 0, 0, w-1, h-1 );

    // paint inner border
    painter.setPen(palette.color(colorGroup, QPalette::Button));
    painter.setBrush(Qt::NoBrush);
    //painter.drawRect( 1, 1, w-3, h-3 );
*/
    // paint the items
    painter.setClipRect( QRect( 3, 3, w-6, h-6 ) );
    painter.setPen(palette.color(colorGroup, QPalette::ButtonText));

    int iH = itemHeight();
    int iC = itemCount();
    if (iC > 0)
    {

        m_itemOffset = m_itemOffset % iH;

        for (int i = -h/2/iH; i <= h/2/iH+1; i++)
        {

            int itemNum = m_currentItem + i;
            /*
            while (itemNum < 0)
                itemNum += iC;
            while (itemNum >= iC)
                itemNum -= iC;
            */
            if (itemNum >= 0 && itemNum < iC)
            {
                int len = h/2/iH;
                /*线性衰减的方法
                int t = len-abs(i);
                t = (t+8)*255/(len+8);
                */
                //抛物线衰减的方法
                int t = abs(i);
                t = 255-t*t*220/len/len;//220是255-y得到,y为边界透明度
                if (t < 0) t = 0;
                //qDebug() << "a值:" << t << endl;
                painter.setPen(QColor(255, 255, 255, t));
                paintItem(&painter, itemNum, QRect(6, h/2 +i*iH - m_itemOffset - iH/2, w-6, iH ));
            }
        }
    }
/*
    // draw a transparent bar over the center
    QColor highlight = palette.color(colorGroup, QPalette::Highlight);
    highlight.setAlpha(150);

    QLinearGradient grad2(0.5, 0, 0.5, 1.0);
    grad2.setColorAt(0, highlight);
    grad2.setColorAt(1.0, highlight.lighter());
    grad2.setCoordinateMode( QGradient::ObjectBoundingMode );
    QBrush gBrush2( grad2 );

    QLinearGradient grad3(0.5, 0, 0.5, 1.0);
    grad3.setColorAt(0, highlight);
    grad3.setColorAt(1.0, highlight.darker());
    grad3.setCoordinateMode( QGradient::ObjectBoundingMode );
    QBrush gBrush3( grad3 );

    painter.fillRect( QRect( 0, h/2 - iH/2, w, iH/2 ), gBrush2 );
    painter.fillRect( QRect( 0, h/2,        w, iH/2 ), gBrush3 );
*/
}

/*!
    Rotates the wheel widget to a given index.
    You can also give an index greater than itemCount or less than zero in which
    case the wheel widget will scroll in the given direction and end up with
    (index % itemCount)
*/
void AbstractWheelWidget::scrollTo(int index) {
    if (index < 0 || index >= itemCount()) {
        return; // 保护，防止非法索引
    }

    this->dosignal = false; // 禁用滚动信号
    QScroller *scroller = QScroller::scroller(this);
    qreal yPosition = WHEEL_SCROLL_OFFSET + index * itemHeight();
    scroller->scrollTo(QPointF(0, yPosition), 500); // 滚动到指定位置
}


