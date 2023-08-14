#pragma once

#include <QWidget>
#include <QMap>
#include <QStringList>
#include <QtNetwork/QNetworkAccessManager>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class ImageView;

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void onOpenImgBtnClicked();
    void onZoomInBtnClicked();
    void onZoomOutBtnClicked();
    void onOriginalBtnClicked();
    void onCenterImgBtnClicked();
    void onAdjustImgBtnClicked();
    void onWfTakePicsBtnClicked();
    void onWfTypeComboxChanged(const QString &text);

    void onScaleChanged(double factor);
    void onReplyFinished(QNetworkReply *reply);

private:
    bool isSupportedMovie(const QString &fileName) const;
    bool readImage(const QString &name);
    bool readImage(const QByteArray &data);
    bool readMovie(const QString &name);
    bool readMovie(const QByteArray &data);

private:
    Ui::Widget *ui;
    ImageView *m_viewer = nullptr;

    QNetworkAccessManager *m_manager = nullptr;
    bool m_requestPic = false;
    QString m_picUrl;

    // categories for sfw and nsfw
    QStringList m_sfwList;
    QStringList m_nsfwList;
};
