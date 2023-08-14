#include "ImageView.h"
#include <QPainter>
#include <QTransform>
#include <QMouseEvent>
#include <QDebug>

ImageView::ImageView(QWidget *parent) : QWidget(parent)
{
}

void ImageView::setImage(const QImage &img)
{
    if (m_movie)
    {
        m_movie->stop();
        m_movie->device()->deleteLater();
        // 防止内部device二次删除
        m_movie->setDevice(nullptr);
        m_movie->deleteLater();
        m_movie = nullptr;
    }
    m_img = img;
    zoomAuto();
}

void ImageView::setMovie(QMovie *mov)
{
    if (mov == nullptr)
        return;

    if (m_movie != nullptr)
        m_movie->deleteLater();

    m_movie = mov;
    
    connect(m_movie, &QMovie::frameChanged, this, [=] {
        m_img = m_movie->currentImage();
        update();
        });
    m_movie->start();
    m_img = m_movie->currentImage();
    zoomAuto();
}

void ImageView::zoomIn()
{
    QPointF pos(width() / 2.0, height() / 2.0);
    zoomInAtPos(pos);
}

void ImageView::zoomOut()
{
    QPointF pos(width() / 2.0, height() / 2.0);
    zoomOutAtPos(pos);
}

void ImageView::zoom100()
{
    m_factor = 1.0;
    m_w = m_img.width();
    m_h = m_img.height();
    adjustImage();
    update();
    emit factorChanged(m_factor);
}

void ImageView::zoomAuto()
{
    adaptFactor();
    centerImage();
    update();
}

void ImageView::centerImage()
{
    m_x = (width() - m_w) / 2.0;
    m_y = (height() - m_h) / 2.0;
    update();
}

void ImageView::mousePressEvent(QMouseEvent *e)
{
    if (!m_img.isNull())
    {
        m_pressed = true;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        m_pos = e->position();
#else
        m_pos = e->localPos();
#endif
    }
}

void ImageView::mouseReleaseEvent(QMouseEvent *)
{
    m_pressed = false;
}

void ImageView::mouseMoveEvent(QMouseEvent *e)
{
    if (m_pressed)
    {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        auto offset = e->position() - m_pos;
#else
        auto offset = e->localPos() - m_pos;
#endif

        if (m_x <= 0 && m_x + m_w >= width())
            m_x += offset.x();
        if (m_y <= 0 && m_y + m_h >= height())
            m_y += offset.y();

        adjustImage();

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        m_pos = e->position();
#else
        m_pos = e->localPos();
#endif
        update();
    }
}

void ImageView::wheelEvent(QWheelEvent *e)
{
    if (m_img.isNull()) return;

    // 滚轮放大
    if (e->angleDelta().y() > 0)
    {
        zoomOutAtPos(e->position());
    }
    // 滚轮缩小
    else if (e->angleDelta().y() < 0)
    {
        zoomInAtPos(e->position());
    }
}

void ImageView::paintEvent(QPaintEvent *e)
{
    if (m_img.isNull()) return QWidget::paintEvent(e);

    QPainter pt(this);
    pt.setRenderHint(QPainter::Antialiasing);
    QTransform tf;
    // 设置绘图左上角坐标
    tf.translate(m_x, m_y);
    // 设置绘图缩放
    tf.scale(m_factor, m_factor);
    // 应用转换
    pt.setTransform(tf);
    // 从绘图坐开始绘制图片
    pt.drawImage(0, 0, m_img);
    pt.end();

    QWidget::paintEvent(e);
}

void ImageView::resizeEvent(QResizeEvent *e)
{
    if (m_img.isNull()) return QWidget::resizeEvent(e);

    if (m_w < width() && m_h < height())
    {
        adaptFactor();
    }
    adjustImage();
    QWidget::resizeEvent(e);
}

void ImageView::zoomInAtPos(const QPointF &pos)
{
    if (m_factor > 0.25 || m_w > width() || m_h > height())
    {
        m_factor /= 1.25;
        // 限制最小缩放倍数
        if (m_factor < 0.25 && m_w < width() && m_h < height()) m_factor = 0.25;
        
        m_x = pos.x() - (pos.x() - m_x) / 1.25;
        m_y = pos.y() - (pos.y() - m_y) / 1.25;
        m_w = m_img.width() * m_factor;
        m_h = m_img.height() * m_factor;
        adjustImage();
        update();
        emit factorChanged(m_factor);
    }
}

void ImageView::zoomOutAtPos(const QPointF &pos)
{
    if (m_factor < 100)
    {
        m_factor *= 1.25;
        if (m_factor > 100)
            m_factor = 100;
        m_x = pos.x() - (pos.x() - m_x) * 1.25;
        m_y = pos.y() - (pos.y() - m_y) * 1.25;
        m_w = m_img.width() * m_factor;
        m_h = m_img.height() * m_factor;
        adjustImage();
        update();
        emit factorChanged(m_factor);
    }
}

void ImageView::adaptFactor()
{
    double w = (double)width() / m_img.width();
    double h = (double)height() / m_img.height();
    // 选择长的一边填充窗口
    if (w < h)
    {
        m_factor = w;
        m_w = width();
        m_h = m_img.height() * m_factor;
    }
    else
    {
        m_factor = h;
        m_w = m_img.width() * m_factor;
        m_h = height();
    }
    emit factorChanged(m_factor);
}

void ImageView::adjustImage()
{
    // 当图像缩放倍率小于25%，且图像大小小于窗口大小时，图像限制在窗口大小
    if (m_w <= width() && m_h <= height() && m_factor < 0.25)
    {
        adaptFactor();
        centerImage();
        return;
    }

    // 图像左边小于窗口左边
    if (m_x > 0 && m_w >= width())
        m_x = 0;

    // 图像顶部小于窗口顶部
    if (m_y > 0 && m_h >= height())
        m_y = 0;

    // 图像右边小于窗口右边
    if (m_x + m_w < width() && m_w >= width())
        m_x = width() - m_w;

    // 图像底部小于窗口底部
    if (m_y + m_h < height() && m_h >= height())
        m_y = height() - m_h;

    if (m_w < width())
        m_x = (width() - m_w) / 2.0;

    if (m_h < height())
        m_y = (height() - m_h) / 2.0;
}
