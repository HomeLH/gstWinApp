#include "gstplayer.h"
#include <glib.h>
#include <gst/gst.h>
#include <QDebug>
#include <QUrl>
#include <gst/video/videooverlay.h>
#include <QVBoxLayout>
#include <QLabel>

gstplayer::gstplayer(QWidget *parent)
    : QWidget(parent)
    ,_restart_time_ms(1389)
    ,_udpReconnect_us(5000000)
{
    // 创建videoWidget
    _playerWidget = new QVideoWidget(this);
//    _playerWidget->setBackgroundRole(QPalette::Dark);
//    _playerWidget->setGeometry(0, 0, 300, 100);
//    _playerWidget->setStyleSheet("background-color:yellow;");

    //设置背景黑色
    QPalette pal(_playerWidget->palette());
    pal.setColor(QPalette::Background, Qt::black);
    _playerWidget->setAutoFillBackground(true);
    _playerWidget->setPalette(pal);


    auto vlayout = new QVBoxLayout{this};
    vlayout->addWidget(_playerWidget);
    this->setLayout(vlayout);

//    _playerWidget->show();
    auto label = new QLabel(_playerWidget);
    label->setText("视频正在加载...");
    label->setStyleSheet("color:white;");

    // 设置解码形式
    setVideoDecoder(H264_SW);
    // 设置资源地址
    setUri("rtsp://184.72.239.149/vod/mp4://BigBuckBunny_175k.mov");
//    setUri("rtsp://127.0.0.1:8554/vlc");

    // 设置重启定时器，定时器在handleERROR中重启；
    _restart_timer.setSingleShot(true);

    connect(&_restart_timer, &QTimer::timeout, this, &gstplayer::_restart_timeout);
    connect(this, &gstplayer::msgErrorReceived, this, &gstplayer::_handleError);
    connect(this, &gstplayer::msgEOSReceived, this, &gstplayer::_handleEOS);

    connect(this, &gstplayer::msgStateChangedReceived, this, &gstplayer::_handleStateChanged);

    //TODO 增加判断条件，当前用一次定时器
    connect(&_frame_timer, &QTimer::timeout, this, &gstplayer::_updateTimer);
    _frame_timer.setSingleShot(true);
    _frame_timer.start(1000);

}

gstplayer::~gstplayer()
{
#if 0
    if(nullptr!=msg){
        gst_message_unref(msg)
    }
#endif
//    gst_object_unref(bus);
//    gst_element_set_state(_pipeline, GST_STATE_NULL);
//    gst_object_unref(_pipeline);
}

void gstplayer::start()
{
    if(_uri.isEmpty()){
        qDebug() << "URI is not specified!";
        return;
    }
    if(_running){
        qDebug() << "Already running!";
        return;
    }
    bool running{false};
    bool pipelineUp{false};
    _stop = false;
    _starting = true;
    GstElement*     dataSource  = nullptr;
//    GstCaps*        caps        = nullptr;
    GstElement*     demux       = nullptr;
    GstElement*     parser      = nullptr;
    GstElement*     queue       = nullptr;
    GstElement*     decoder     = nullptr;
    GstElement*     queue1      = nullptr;
    GstElement* _playsink{nullptr};


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
        if((_playsink = gst_element_factory_make("glimagesink", nullptr)) == nullptr){
//        if((_playsink = gst_element_factory_make("ximagesink", nullptr)) == nullptr){
//            if((_playsink = gst_element_factory_make("autovideosink", nullptr)) == nullptr){
//            if((_playsink = gst_element_factory_make("playsink", nullptr)) == nullptr){
            qDebug() << "start() failed. Error with gst_element_factory_make(\"playsink\")";
            break;
        }
        // 将sink绑定到QVideoWidget上去；
        guintptr hint = _playerWidget->winId();
        qDebug()<<hint<<endl;

        // 加入 pipeline
        gst_bin_add_many(GST_BIN(_pipeline), dataSource, demux, parser, _tee, queue
                         , decoder, _playsink, nullptr);
        // pipeline 成功
        pipelineUp = true;
        // link


        if(!gst_element_link_many(demux, parser, _tee, queue, decoder, _playsink, nullptr)) {
            qDebug() << "Unable to link RTSP elements.";
            break;
        }
        assert(nullptr!=demux);
        g_signal_connect(dataSource, "pad-added", G_CALLBACK(&gstplayer::newPadCB), demux);

        WId xwinid = _playerWidget->winId();
        gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(_playsink), xwinid);

        dataSource = demux = parser = queue = decoder = queue1 = _playsink = nullptr;
        GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(_pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "pipeline-paused");
        // 注册bus上的事件回调函数
        GstBus* bus = nullptr;
        // bus 上的消息处理，注册回到函数 _onBusMessage, 把this传入
        if ((bus = gst_pipeline_get_bus(GST_PIPELINE(_pipeline))) != nullptr) {
            gst_bus_enable_sync_message_emission(bus);
            g_signal_connect(bus, "sync-message", G_CALLBACK(&gstplayer::_onBusMessage), this);
            gst_object_unref(bus);
            bus = nullptr;
        }
        // 启动播放

        running = gst_element_set_state(_pipeline, GST_STATE_PLAYING) != GST_STATE_CHANGE_FAILURE;
        qDebug() << "Running flag: " << running;
    } while(0);
    // TODO 启动失败后清理资源
    if(!running){
        qDebug() << "start() failed!";
        if(nullptr!=_pipeline){
            gst_object_unref(_pipeline);
            _pipeline = nullptr;
        }

        // If we failed before adding items to the pipeline, then clean up
        if (!pipelineUp) {
            if (decoder != nullptr) {
                gst_object_unref(decoder);
                decoder = nullptr;
            }

            if (parser != nullptr) {
                gst_object_unref(parser);
                parser = nullptr;
            }

            if (demux != nullptr) {
                gst_object_unref(demux);
                demux = nullptr;
            }

            if (dataSource != nullptr) {
                gst_object_unref(dataSource);
                dataSource = nullptr;
            }

            if (_tee != nullptr) {
                gst_object_unref(_tee);
                dataSource = nullptr;
            }

            if (queue != nullptr) {
                gst_object_unref(queue);
                dataSource = nullptr;
            }
            if(_playsink != nullptr){
                gst_object_unref(_playsink);
                _playsink = nullptr;
            }
        }
        _running = false;
    } else {
        GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(_pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "pipeline-playing");
        _running = true;
        qDebug() << "Running";
    }
    _starting = false;

}

void gstplayer::stop()
{
    _stop = true;
    qDebug() <<"stop()";
    // 判断有没有streaming
    if(!_streaming){
        // 清理pipeline
        _shutdownPipeline();
    } else if(_pipeline!=nullptr && !_stopping){
        qDebug() << "Stopping _pipeline!";
        gst_element_send_event(_pipeline, gst_event_new_eos());
        _stopping = true;
        GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(_pipeline));
        // 阻塞等待接收eos信号
        GstMessage* message = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE,
                                (GstMessageType)(GST_MESSAGE_EOS|GST_MESSAGE_ERROR));
        gst_object_unref(bus);
        // error or eos
        if(GST_MESSAGE_TYPE(message) == GST_MESSAGE_ERROR) {
            _shutdownPipeline();
            qCritical() << "Error stopping pipeline!";
        } else if(GST_MESSAGE_TYPE(message) == GST_MESSAGE_EOS) {
            _handleEOS();
        }
        gst_message_unref(message);


    }

}

void gstplayer::newPadCB(GstElement *element, GstPad *pad, gpointer data)
{
    GstPad *sinkpad;
    GstElement *decoder = (GstElement*)data;
    assert(nullptr!=data);
    sinkpad = gst_element_get_static_pad(decoder, "sink");
    assert(nullptr!=sinkpad);
    if(nullptr == sinkpad){
        qDebug() << "NUllptr!";
    }
    gst_pad_link(pad, sinkpad);
    gst_object_unref(sinkpad);
    qDebug() << "linked";
//    gchar* name = gst_pad_get_name(pad);
//    //g_print("A new pad %s was created\n", name);
//    GstCaps* p_caps = gst_pad_get_pad_template_caps (pad);
//    gchar* description = gst_caps_to_string(p_caps);
//    qDebug() << p_caps << ", " << description;
//    g_free(description);
//    GstElement* sink = GST_ELEMENT(data);
////    if(GST_PAD_LINK_FAILED(gst_pad_link(pad, sink))
//    if(gst_element_link_pads(element, name, sink, "sink") == false)
//        qCritical() << "newPadCB : failed to link elements\n";
//    g_free(name);
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

void gstplayer::_shutdownPipeline()
{
    if(!_pipeline) {
        qDebug() << "No pipeline";
        return;
    }
    GstBus* bus = nullptr;
    // gst_bus_disable_sync_message_emission
    if ((bus = gst_pipeline_get_bus(GST_PIPELINE(_pipeline))) != nullptr) {
        gst_bus_disable_sync_message_emission(bus);
        gst_object_unref(bus);
        bus = nullptr;
    }
    gst_element_set_state(_pipeline, GST_STATE_NULL);
//    gst_bin_remove(GST_BIN(_pipeline), _videoSink);
    gst_object_unref(_pipeline);
    _pipeline = nullptr;
    _streaming = false;
//    _recording = false;
    _stopping = false;
    _running = false;
    //    emit recordingChanged();
}

void gstplayer::_onBusMessage(GstBus *bus, GstMessage *msg, gpointer data)
{
    Q_UNUSED(bus)
    Q_ASSERT(msg != nullptr && data != nullptr);
    gstplayer* pThis = static_cast<gstplayer*>(data);

    switch(GST_MESSAGE_TYPE(msg)) {
    case(GST_MESSAGE_ERROR): {
        gchar* debug;
        GError* error;
        gst_message_parse_error(msg, &error, &debug);
        g_free(debug);
        qDebug() << error->message;
        g_error_free(error);
        pThis->msgErrorReceived();
    }
        break;
    case(GST_MESSAGE_EOS):
        pThis->msgEOSReceived();
        break;
    case(GST_MESSAGE_STATE_CHANGED):
        pThis->msgStateChangedReceived();
        break;
    default:
        break;
    }
//    return TRUE;
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
    // 触发条件 重启定时器超时；重启定时器在handleERROR函数中重置
    qDebug() << "Restart video streaming";
//    stopVideo();
    stop();
    start();
//    // seturi
//    _updateSettings();
//    startVideo();
    //    emit aspectRatioChanged();
}

void gstplayer::_handleStateChanged()
{
    if(_pipeline) {
        _streaming = GST_STATE(_pipeline) == GST_STATE_PLAYING;
        qDebug() << "State changed, _streaming:" << _streaming;
    }
}

void gstplayer::_handleEOS()
{
    if(_stopping) {
        _shutdownPipeline();
        qDebug() << "Stopped";
    }  else {
        qDebug() << "VideoReceiver: Unexpected EOS!";
        _handleError();
    }
}

void gstplayer::_handleError()
{
    qDebug() << "Gstreamer error!";
    // If there was an error we switch to software decoding only
    _tryWithHardwareDecoding = false;
    stop();
    _restart_timer.start(_restart_time_ms);
}

void gstplayer::setUri(const QString &uri)
{
    _uri = uri;
}
