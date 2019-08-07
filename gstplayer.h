#ifndef GSTPLAYER_H
#define GSTPLAYER_H

#include <QWidget>
#include <QVideoWidget>
#include <gst/gst.h>

class gstplayer : public QWidget
{
    Q_OBJECT
public:
    explicit gstplayer(QWidget *parent = nullptr);
    ~gstplayer();

signals:

public slots:
    void play();
private:
    QVideoWidget *_playerWidget;
    GstElement *pipeline;
    GstBus *bus{};
};

#endif // GSTPLAYER_H
