#include "gstplayer.h"
#include <gst/gst.h>
#include <QDebug>
#include <QUrl>
#include <gst/video/videooverlay.h>
#include <QVBoxLayout>

gstplayer::gstplayer(QWidget *parent) : QWidget(parent)
{
//    gst_init(nullptr, nullptr);
    _playerWidget = new QVideoWidget(this);
//    _playerWidget->resize(200,400);
    _playerWidget->setBackgroundRole(QPalette::Dark);
    auto vlayout = new QVBoxLayout{this};
    vlayout->addWidget(_playerWidget);
    qDebug() << "run here";
    // build pipeline
}

gstplayer::~gstplayer()
{
#if 0
    if(nullptr!=msg){
        gst_message_unref(msg)
    }
#endif
    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
}

void gstplayer::play()
{
    pipeline = gst_element_factory_make ("playbin", "playbin");
//    QString filename{"C:/Users/li_ha/Desktop/620x252_3DModels.mp4"};
//    QString uri = "file:///" + filename;
    QString uri{"rtsp://127.0.0.1:8554/vlc"};
    qDebug()<<"uri is :"<<uri;
    g_object_set (pipeline, "uri", uri.toStdString().c_str(), NULL);

//    pipeline =
//         gst_parse_launch
//         ("playbin uri=https://www.freedesktop.org/software/gstreamer-sdk/data/media/sintel_trailer-480p.webm",
//         NULL);
    guintptr hint = _playerWidget->winId();
    qDebug()<<hint<<endl;
    gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(pipeline), hint);

    if(!gst_element_set_state (pipeline, GST_STATE_PLAYING))
    {
        qDebug()<<"set state playing error"<<endl;
    }

    /* Wait until error or EOS */
    bus = gst_element_get_bus (pipeline);
    GstMessage *msg{};
    // block
//    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE,(GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

}
