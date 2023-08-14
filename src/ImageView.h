#pragma once

#include <QWidget>
#include <QImage>
#include <QMovie>

class ImageView : public QWidget
{
    Q_OBJECT
public:
    explicit ImageView(QWidget *parent = nullptr);

    QImage image() const { return m_img; }
    // Note: 请勿在外部将该QMovie对象删除
    QMovie* movie() const { return m_movie; }
    double scaleFactor() const { return m_factor; }

public slots:
    void setImage(const QImage &img);
    void setMovie(QMovie *mov);
    void zoomIn();      // 缩小
    void zoomOut();     // 放大
    void zoom100();     // 缩放比例100%
    void zoomAuto();    // 图像自适应窗口大小
    void centerImage(); // 图像居中

protected:
    virtual void mousePressEvent(QMouseEvent *e) override;
    virtual void mouseReleaseEvent(QMouseEvent *e) override;
    virtual void mouseMoveEvent(QMouseEvent *e) override;
    virtual void wheelEvent(QWheelEvent *e) override;       // 滚轮事件
    virtual void paintEvent(QPaintEvent *e) override;       // 绘画事件
    virtual void resizeEvent(QResizeEvent *e) override;     // 窗口大小改变

    virtual void zoomInAtPos(const QPointF &pos);
    virtual void zoomOutAtPos(const QPointF &pos);
    void adaptFactor();     // 计算适应窗口时的缩放比例
    void adjustImage();     // 调整图像位置

signals:
    void factorChanged(double factor);

protected:
    double m_x = 0.0;   // 图像左上角坐标X
    double m_y = 0.0;   // 图像左上角坐标Y
    double m_w = 0.0;   // 图像缩放后的宽
    double m_h = 0.0;   // 图像缩放后的高
    double m_factor = 1.0;  // 图像缩放比例

private:
    QImage m_img;
    QMovie *m_movie = nullptr;
    QPointF m_pos;
    bool m_pressed = false;
};

