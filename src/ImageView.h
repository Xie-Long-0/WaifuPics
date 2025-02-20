#pragma once

#include <QWidget>
#include <QImage>
#include <QMovie>

class ImageView : public QWidget
{
    Q_OBJECT
public:
    explicit ImageView(QWidget *parent = nullptr);

    QImage image() const
    {
        return m_img;
    }
    // Note: 请勿在外部将该QMovie对象删除
    QMovie *const movie() const
    {
        return m_movie;
    }
    double scaleFactor() const
    {
        return m_factor;
    }
    void reset();
    void setMaxFactor(double factor)
    {
        m_maxFactor = factor;
    }
    void setMinFactor(double factor)
    {
        m_minFactor = factor;
    }

public slots:
    void setImage(const QImage &img);
    // Note: 请勿在外部将该QMovie对象删除
    void setMovie(QMovie *mov);
    // 缩小
    void zoomIn();
    // 放大
    void zoomOut();
    // 缩放至100%
    void zoom100();
    // 图像自适应窗口大小
    void zoomAuto();
    // 图像居中
    void centerImage();
    void setScaleFactor(double factor);

protected:
    virtual void mousePressEvent(QMouseEvent *e) override;
    virtual void mouseReleaseEvent(QMouseEvent *e) override;
    virtual void mouseMoveEvent(QMouseEvent *e) override;
    virtual void wheelEvent(QWheelEvent *e) override;
    virtual void paintEvent(QPaintEvent *e) override;
    virtual void resizeEvent(QResizeEvent *e) override;

    virtual void zoomAtPos(const QPointF &pos, double factor);
    // 计算自适应窗口时的缩放比例
    void adaptFactor();
    // 调整图像位置
    void adjustImage();

signals:
    void factorChanged(double factor);

protected:
    double m_x = 0.0;   // 图像左上角坐标X
    double m_y = 0.0;   // 图像左上角坐标Y
    double m_w = 0.0;   // 图像缩放后的宽
    double m_h = 0.0;   // 图像缩放后的高
    double m_factor = 1.0;  // 图像缩放比例
    double m_maxFactor = 10.0;
    double m_minFactor = 0.1;

private:
    QImage m_img;
    QMovie *m_movie = nullptr;
    QPointF m_pos;
    bool m_pressed = false;
    bool m_firstUpdate = true;
};

