#include "widget.h"
#include "ui_widget.h"
#include <QFileDialog>
#include <QBoxLayout>
#include <QNetworkReply>
#include <QStandardPaths>
#include <QBuffer>
#include <QMessageBox>
#include <QDebug>

#include "ImageView.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , m_settings("Configs.ini", QSettings::IniFormat)
{
    ui->setupUi(this);
    setWindowTitle(tr("看图"));

    m_viewer = new ImageView(ui->frame);
    QHBoxLayout *layout = new QHBoxLayout(ui->frame);
    layout->addWidget(m_viewer);
    layout->setContentsMargins(0, 0, 0, 0);
    ui->frame->setLayout(layout);

    connect(m_viewer, &ImageView::factorChanged, this, &Widget::onScaleChanged);

    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, &QNetworkAccessManager::finished, this, &Widget::onReplyFinished);
    connect(m_manager, &QNetworkAccessManager::sslErrors, this,
        [=](QNetworkReply *reply, const QList<QSslError> &errors) {
            QString text;
            for (auto &e : errors)
                text.append(e.errorString() + '\n');
            QMessageBox::critical(this, tr("网络错误"), text);
            qWarning() << errors;
        });

    // 获取支持的图片格式
    m_supportedImageFmts = QImageReader::supportedImageFormats();

    // 获取支持的动图格式
    m_supportedMovieFmts = QMovie::supportedFormats();

    // 使用`https://waifu.pics/docs`接口获取图片
    // categories for sfw and nsfw
    m_sfwList = {
        "waifu", "neko",     "shinobu",  "megumin", "bully", "cuddle", "cry",  "hug",
        "awoo",  "kiss",     "lick",     "pat",     "smug",  "bonk",   "yeet", "blush",
        "wave",  "highfive", "handhold", "nom",     "bite",  "glomp",  "slap", "kill",
        "kick",  "happy",    "wink",     "poke",    "dance", "cringe", "smile"
    };

    m_nsfwList = {
        "waifu", "neko", "trap", "blowjob"
    };

    // add waifupics type
    ui->wf_typeComBox->addItem("sfw");
    ui->wf_typeComBox->addItem("nsfw");
    ui->wf_typeComBox->setCurrentIndex(0);

    for (auto &v : m_sfwList)
    {
        ui->wf_categoryComBox->addItem(v);
    }
    ui->wf_categoryComBox->setCurrentIndex(0);

    // 恢复窗口状态
    if (bool max = m_settings.value("WindowMaximize", false).toBool(); max == true)
    {
        showMaximized();
    }
    else
    {
        if (auto size = m_settings.value("WindowSize").toSize(); size.isValid())
        {
            resize(size);
        }
        if (auto pos = m_settings.value("WindowPos").toPoint(); !pos.isNull())
        {
            move(pos);
        }
    }
    m_lastPath = m_settings.value("LastSavePath", QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)).toString();

    connect(ui->adjustImgBtn, &QPushButton::clicked, this, &Widget::onAdjustImgBtnClicked);
    connect(ui->centerImgBtn, &QPushButton::clicked, this, &Widget::onCenterImgBtnClicked);
    connect(ui->openImgBtn, &QPushButton::clicked, this, &Widget::onOpenImgBtnClicked);
    connect(ui->originalBtn, &QPushButton::clicked, this, &Widget::onOriginalBtnClicked);
    connect(ui->zoomInBtn, &QPushButton::clicked, this, &Widget::onZoomInBtnClicked);
    connect(ui->zoomOutBtn, &QPushButton::clicked, this, &Widget::onZoomOutBtnClicked);
    connect(ui->wf_takeButton, &QPushButton::clicked, this, &Widget::onWfTakePicsBtnClicked);
    connect(ui->wf_typeComBox, &QComboBox::currentTextChanged, this, &Widget::onWfTypeComboxChanged);

    connect(ui->savePicBtn, &QPushButton::clicked, this, [=] {
        QUrl url(m_picUrl);
        auto name = url.path();

        if (isSupportedMovie(name))
        {
            auto mov = m_viewer->movie();
            if (mov == nullptr)
                return;

            QString suffix = "*." + m_supportedMovieFmts.join(" *.");
            QFileDialog dlg(this, tr("保存图片"), m_lastPath + name, tr("动画 (%1)").arg(suffix));
            dlg.setFileMode(QFileDialog::FileMode::AnyFile);
            dlg.setAcceptMode(QFileDialog::AcceptSave);

            if (dlg.exec() == QFileDialog::Accepted)
            {
                name = dlg.selectedFiles().first();
                mov->stop();
                auto buf = mov->device();
                // NOTE: 在读取完整数据时需要将位置设置为0
                buf->seek(0);
                auto data = buf->readAll();
                QFile file(name);
                file.open(QIODevice::WriteOnly);
                file.write(data);
                file.close();
                mov->start();
                QUrl url = dlg.selectedUrls().first();
                m_lastPath = url.toLocalFile().remove(url.fileName());
                QMessageBox::information(this, tr("保存成功"), tr("图片已保存到 %1").arg(name));
            }
        }
        else
        {
            auto img = m_viewer->image();
            if (img.isNull())
                return;

            QString suffix = "*." + m_supportedImageFmts.join(" *.");
            QFileDialog dlg(this, tr("保存图片"), m_lastPath + name, tr("图片 (%1)").arg(suffix));
            dlg.setFileMode(QFileDialog::FileMode::AnyFile);
            dlg.setAcceptMode(QFileDialog::AcceptSave);

            if (dlg.exec() == QFileDialog::Accepted)
            {
                name = dlg.selectedFiles().first();
                img.save(name);
                QUrl url = dlg.selectedUrls().first();
                m_lastPath = url.toLocalFile().remove(url.fileName());
                QMessageBox::information(this, tr("保存成功"), tr("图片已保存到 %1").arg(name));
            }
        }
        });

    ui->zoomInBtn->setEnabled(false);
    ui->zoomOutBtn->setEnabled(false);
    ui->originalBtn->setEnabled(false);
    ui->centerImgBtn->setEnabled(false);
    ui->adjustImgBtn->setEnabled(false);
    ui->savePicBtn->setEnabled(false);

#ifdef QT_DEBUG
    QDir::current().mkdir("tmp");
#endif
}

Widget::~Widget()
{
    m_settings.setValue("WindowMaximize", isMaximized());
    if (!isMaximized() && !isMinimized())
    {
        m_settings.setValue("WindowSize", size());
        m_settings.setValue("WindowPos", pos());
    }
    m_settings.setValue("LastSavePath", m_lastPath);
    delete ui;
}

void Widget::onOpenImgBtnClicked()
{
    QString imgName = QFileDialog::getOpenFileName(this, tr("打开图片"),
        QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
        tr("图片 (%1)").arg(m_supportedImageFmts.join(' ')));
    if (imgName.isEmpty())
        return;

    if (isSupportedMovie(imgName))
    {
        if (readMovie(imgName) == false)
            return;
    }
    else
    {
        if (readImage(imgName) == false)
            return;
    }

    ui->imgPathLineEdit->setText(imgName);

    ui->zoomInBtn->setEnabled(true);
    ui->zoomOutBtn->setEnabled(true);
    ui->originalBtn->setEnabled(true);
    ui->centerImgBtn->setEnabled(true);
    ui->adjustImgBtn->setEnabled(true);
}

void Widget::onZoomInBtnClicked()
{
    m_viewer->zoomIn();
}

void Widget::onZoomOutBtnClicked()
{
    m_viewer->zoomOut();
}

void Widget::onOriginalBtnClicked()
{
    m_viewer->zoom100();
}

void Widget::onCenterImgBtnClicked()
{
    m_viewer->centerImage();
}

void Widget::onAdjustImgBtnClicked()
{
    m_viewer->zoomAuto();
}

void Widget::onWfTakePicsBtnClicked()
{
    m_requestPic = false;
    ui->wf_takeButton->setEnabled(false);
    ui->openImgBtn->setEnabled(false);

    QUrl url(QString("https://api.waifu.pics/%1/%2")
        .arg(ui->wf_typeComBox->currentText())
        .arg(ui->wf_categoryComBox->currentText()));

    QNetworkRequest rqst(url);
#if QT_VERSION >= QT_VERSION_CHECK(5,15,0)
    rqst.setTransferTimeout(10000);
#endif
    m_manager->get(rqst);
}

void Widget::onReplyFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError || !reply->isReadable())
    {
        QMessageBox::critical(this, tr("发生错误"), reply->errorString());
        qWarning() << "Reply error:" << reply->errorString();
        ui->wf_takeButton->setEnabled(true);
        ui->openImgBtn->setEnabled(true);
        ui->progressBar->reset();
        return;
    }

    auto data = reply->readAll();

    // 获取图片地址的响应报文
    if (m_requestPic == false)
    {
        // 使用正则表达式匹配模板 `{"url":"urltext"}\n`
        QRegularExpression pattern("{\"url\":\"([^\"]*)\"}");
        QRegularExpressionMatch match = pattern.match(data);
        if (!match.hasMatch())
        {
            qDebug() << data;
            qWarning() << "Content does not match";
            ui->wf_takeButton->setEnabled(true);
            ui->openImgBtn->setEnabled(true);
            return;
        }

        m_picUrl = match.captured(1);
        qDebug() << m_picUrl;

        m_requestPic = true;

        QNetworkRequest rqst(m_picUrl);
#if QT_VERSION >= QT_VERSION_CHECK(5,15,0)
        rqst.setTransferTimeout(10000);
#endif
        auto p_reply = m_manager->get(rqst);
        connect(p_reply, &QNetworkReply::downloadProgress, this, [=](qint64 rec, qint64 total) {
            ui->progressBar->setValue(rec * 100 / total);
            });
    }
    else // 获取图片的响应报文
    {
        if (isSupportedMovie(m_picUrl))
        {
            if (readMovie(data) == false)
                return;
        }
        else
        {
            if (readImage(data) == false)
                return;
        }

        ui->imgPathLineEdit->setText(m_picUrl);

        ui->zoomInBtn->setEnabled(true);
        ui->zoomOutBtn->setEnabled(true);
        ui->originalBtn->setEnabled(true);
        ui->centerImgBtn->setEnabled(true);
        ui->adjustImgBtn->setEnabled(true);
        ui->savePicBtn->setEnabled(true);
        ui->wf_takeButton->setEnabled(true);
        ui->openImgBtn->setEnabled(true);
    }
}

bool Widget::isSupportedMovie(const QString &fileName) const
{
    if (fileName.size() >= 3)
    {
        QString suffix = fileName.split('.').last();
        for (auto &fmt : m_supportedMovieFmts)
        {
            if (suffix == fmt)
                return true;
        }
    }
    return false;
}

bool Widget::readImage(const QString &name)
{
    QImage img(name);
    if (img.isNull())
        return false;

    m_viewer->setImage(img);
    return true;
}

bool Widget::readImage(const QByteArray &data)
{
    QImage img;
    img.loadFromData(data);
    if (img.isNull())
    {
        qWarning() << "Image data is invalid";
        ui->wf_takeButton->setEnabled(true);
        ui->openImgBtn->setEnabled(true);
        return false;
    }

    m_viewer->setImage(img);
#ifdef QT_DEBUG
    img.save("tmp/" + QUrl(m_picUrl).fileName());
#endif
    return true;
}

bool Widget::readMovie(const QString &name)
{
    auto mov = new QMovie(this);
    mov->setFileName(name);
    if (!mov->isValid())
        return false;

    m_viewer->setMovie(mov);
    return true;
}

bool Widget::readMovie(const QByteArray &data)
{
    // 将数据存到QBuffer中以供QMovie读取
    auto buf = new QBuffer(this);
    buf->setData(data);
    if (!buf->open(QIODevice::ReadOnly))
    {
        qWarning() << "Can not read data for QBuffer";
        ui->wf_takeButton->setEnabled(true);
        ui->openImgBtn->setEnabled(true);
        buf->deleteLater();
        return false;
    }

    auto mov = new QMovie(this);
    mov->setDevice(buf);
    if (!mov->isValid())
    {
        qWarning() << "Data is invalid for QMovie";
        ui->wf_takeButton->setEnabled(true);
        ui->openImgBtn->setEnabled(true);
        buf->deleteLater();
        mov->deleteLater();
        return false;
    }

    m_viewer->setMovie(mov);
#ifdef QT_DEBUG
    QFile file("tmp/" + QUrl(m_picUrl).fileName());
    file.open(QIODevice::WriteOnly);
    file.write(buf->data());
#endif
    return true;
}

void Widget::onWfTypeComboxChanged(const QString &text)
{
    ui->wf_categoryComBox->clear();
    if (text == "sfw")
    {
        for (auto &v : m_sfwList)
        {
            ui->wf_categoryComBox->addItem(v);
        }
    }
    else if (text == "nsfw")
    {
        for (auto &v : m_nsfwList)
        {
            ui->wf_categoryComBox->addItem(v);
        }
    }
    ui->wf_categoryComBox->setCurrentIndex(0);
}

void Widget::onScaleChanged(double factor)
{
    ui->scaledLabel->setText(QString::number((int)(factor * 100)) + '%');
}
