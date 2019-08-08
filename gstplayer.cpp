#include "gstplayer.h"
#include <gst/gst.h>
#include <QDebug>
#include <QUrl>
#include <gst/video/videooverlay.h>
#include <QVBoxLayout>

gstplayer::gstplayer(QWidget *parent)
    : QWidget(parent)
    ,_restart_time_ms(1389)
    ,_udpReconnect_us(5000000)
{
    // 创建videoWidget
    _playerWidget = new QVideoWidget(this);
    _playerWidget->setBackgroundRole(QPalette::Dark);
    auto vlayout = new QVBoxLayout{this};
    vlayout->addWidget(_playerWidget);

    // 设置解码形式
    setVideoDecoder(H264_SW);
    // 设置资源地址
    setUri("rtsp://127.0.0.1:8554/vlc");

    // 设置重启定时器
    _restart_timer.setSingleShot(true);

    connect(&_restart_timer, &QTimer::timeout, this, &gstplayer::_restart_timeout);

    //TODO 增加判断条件，当前用一次定时器
    connect(&_frame_timer, &QTimer::timeout, this, &gstplayer::_updateTimer);
    _frame_timer.setSingleShot(true);
    _frame_timer.start(10);

}

gstplayer::~gstplayer()
{
#if 0
    if(nullptr!=msg){
        gst_message_unref(msg)
    }
#endif
    gst_object_unref(bus);
    gst_element_set_state(_pipeline, GST_STATE_NULL);
    gst_object_unref(_pipeline);
}

void gstplayer::start()
{
    if(_uri.isEmpty()){
        qDebug() << "URI is not specified!";
        return;
    }
    GstElement*     dataSource  = nullptr;
    GstCaps*        caps        = nullptr;
    GstElement*     demux       = nullptr;
    GstElement*     parser      = nullptr;
    GstElement*     queue       = nullptr;
    GstElement*     decoder     = nullptr;
    GstElement*     queue1      = nullptr;
    GstElement* _playsink{nullptr};
    bool running{false};
    do {
        // 创建pipeline
        if((_pipeline = gst_pipeline_new("rtspreceiver")) == nullptr){
            qDebug() << "start() failed. Error with gst_pipeline_new()";
            break;
        }
        // 创建rtspsrc element
        if((dataSource = gst_element_factory_make("rtspsrc", "rtsp-source")) == nullptr){
            qDebug() << "Error with data source for gst_element_factory_make()";
            break;
        }
        // 设置rtsp 源
        g_object_set(static_cast<gpointer>(dataSource), "location", qPrintable(_uri),
                     "latency", 17, "udp-reconnect", 1, "timeout", _udpReconnect_us, NULL);
        // 设置 demux
        if ((demux = gst_element_factory_make(_depayName.toLatin1().data(), "rtp-depacketizer")) == nullptr) {
           qDebug() << "start() failed. Error with gst_element_factory_make('" << _depayName << "')";
            break;
        }
        qDebug() << _depayName;
        // 设置parser
        if ((parser = gst_element_factory_make(_parserName.toLatin1().data(), "parser")) == nullptr) {
            qDebug() << "start() failed. Error with gst_element_factory_make('" << _parserName << "')";
            break;
        }
        // 设置tee
        if((_tee = gst_element_factory_make("tee", nullptr)) == nullptr)  {
            qDebug() << "start() failed. Error with gst_element_factory_make('tee')";
            break;
        }
        // 设置 queue
        if((queue = gst_element_factory_make("queue", nullptr)) == nullptr)  {
            // TODO: We may want to add queue2 max-size-buffers=1 to get lower latency
            //       We should compare gstreamer scripts to QGroundControl to determine the need
            qDebug() << "start() failed. Error with gst_element_factory_make('queue')";
            break;
        }
        // 设置解码器，没有硬解码的情况下用软解码
        if (!_hwDecoderName.isNull() || (decoder = gst_element_factory_make(_hwDecoderName.toLatin1().data(), "decoder")) == nullptr) {
            qWarning() << "VideoReceiver::start() hardware decoding not available "
                       << (_hwDecoderName.isEmpty() ? _hwDecoderName : "");
            if ((decoder = gst_element_factory_make(_swDecoderName.toLatin1().data(), "decoder")) == nullptr) {
                qCritical() << "start() failed. Error with gst_element_factory_make('" << _swDecoderName << "')";
                break;
            }
        }
        // TODO 写文件功能
#if 0
        if ((queue1 = gst_element_factory_make("queue", nullptr)) == nullptr) {
            qCritical() << "start() failed. Error with gst_element_factory_make('queue') [1]";
            break;
        }
#endif
        // 创建sink
        if((_playsink = gst_element_factory_make("playsink", nullptr)) == nullptr){
            qDebug() << "start() failed. Error with gst_element_factory_make(\"playsink\")";
            break;
        }
        // 将sink绑定到QVideoWidget上去；
        guintptr hint = _playerWidget->winId();
        qDebug()<<hint<<endl;

        // 加入 pipeline
        gst_bin_add_many(GST_BIN(_pipeline), dataSource, demux, parser, _tee, queue
                         , decoder, _playsink, nullptr);

        gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(_playsink), hint);
        // link

        g_signal_connect(dataSource, "pad-added", G_CALLBACK(&gstplayer::newPadCB), demux);
        if(!gst_element_link_many(demux, parser, _tee, queue, decoder, _playsink, nullptr)) {
            qDebug() << "Unable to link RTSP elements.";
            break;
        }
        dataSource = demux = parser = queue = decoder = queue1 = _playsink = nullptr;
        GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(_pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "pipeline-paused");
        // 启动播放

        running = gst_element_set_state(_pipeline, GST_STATE_PLAYING) != GST_STATE_CHANGE_FAILURE;
        qDebug() << "Running flag: " << running;
    } while(0);
    // TODO 启动失败后清理资源

}

void gstplayer::newPadCB(GstElement *element, GstPad *pad, gpointer data)
{
    gchar* name = gst_pad_get_name(pad);
    //g_print("A new pad %s was created\n", name);
    GstCaps* p_caps = gst_pad_get_pad_template_caps (pad);
    gchar* description = gst_caps_to_string(p_caps);
    qDebug() << p_caps << ", " << description;
    g_free(description);
    GstElement* sink = GST_ELEMENT(data);
//    if(GST_PAD_LINK_FAILED(gst_pad_link(pad, sink))
    if(gst_element_link_pads(element, name, sink, "sink") == false)
        qCritical() << "newPadCB : failed to link elements\n";
    g_free(name);
}

void gstplayer::setVideoDecoder(gstplayer::VideoEncoding encoding)
{
    if (encoding == H265_HW || encoding == H265_SW) {
        _depayName = "rtph265depay";
        _parserName = "h265parse";
        _swDecoderName = "avdec_h265";
    } else {
        _depayName = "rtph264depay";
        _parserName = "h264parse";
        _swDecoderName = "avdec_h264";
    }

    if (!_tryWithHardwareDecoding) {
        _hwDecoderName.clear();
    }
}

void gstplayer::play()
{
    _lpipeline = gst_element_factory_make ("playbin", "playbin");
    GstElement *_sink = gst_element_factory_make("qtquick2videosink",nullptr);
    if(nullptr == _sink){
        qCritical("Failed to create qtquick2videosink. Make sure it is installed correctly");
    }
//    QString filename{"C:/Users/li_ha/Desktop/620x252_3DModels.mp4"};
//    QString uri = "file:///" + filename;
    QString uri{"rtsp://127.0.0.1:8554/vlc"};
    qDebug()<<"uri is :"<<uri;
    g_object_set (_lpipeline, "uri", uri.toStdString().c_str(), NULL);

//    pipeline =
//         gst_parse_launch
//         ("playbin uri=https://www.freedesktop.org/software/gstreamer-sdk/data/media/sintel_trailer-480p.webm",
//         NULL);

    guintptr hint = _playerWidget->winId();
    qDebug()<<hint<<endl;
    gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(_lpipeline), hint);
    if(!gst_element_set_state (_lpipeline, GST_STATE_PLAYING))
    {
        qDebug()<<"set state playing error"<<endl;
    }

    /* Wait until error or EOS */
    bus = gst_element_get_bus (_lpipeline);
    GstMessage *msg{};
    // block
//    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE,(GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

}

void gstplayer::_updateTimer()
{
    // TODO 增加判断条件 超时判断
    // 开启播放
    start();

}

void gstplayer::_restart_timeout()
{
//    qCDebug(VideoManagerLog) << "Restart video streaming";
//    stopVideo();
//    // seturi
//    _updateSettings();
//    startVideo();
//    emit aspectRatioChanged();
}

void gstplayer::setUri(const QString &uri)
{
    _uri = uri;
}
