#ifndef GSTPLAYER_H
#define GSTPLAYER_H

#include <QWidget>
#include <QVideoWidget>
#include <gst/gst.h>
#include <QTimer>

class gstplayer : public QWidget
{
    Q_OBJECT
public:
    enum VideoEncoding {
        H264_SW = 1,
        H264_HW = 2,
        H265_SW = 3,
        H265_HW = 4
    };
    explicit gstplayer(QWidget *parent = nullptr);
    ~gstplayer();
    void start();
    void stop();

    void setUri(const QString &uri);

private:
    void setVideoDecoder(VideoEncoding encoding);
    void _shutdownPipeline();
    static void newPadCB(GstElement* element, GstPad* pad, gpointer data);
    static void _onBusMessage(GstBus* bus, GstMessage* msg, gpointer data);

signals:
    void msgStateChangedReceived();
    void msgErrorReceived();
    void msgEOSReceived();
public slots:
    void play();
private slots:
    void _updateTimer();
    void _restart_timeout();
    void _handleStateChanged();
    void _handleEOS();
    void _handleError();


private:
    QVideoWidget *_playerWidget;
    GstElement *_pipeline;
    GstElement *_lpipeline;
    GstElement *_tee;
    GstBus *bus{};

    QTimer _restart_timer;
    QTimer _frame_timer;
    int _restart_time_ms;
    //-- RTSP UDP reconnect timeout
    uint64_t _udpReconnect_us;
    // encode
    QString _depayName;
    QString _parserName;
    QString _hwDecoderName;
    QString _swDecoderName;
    bool _tryWithHardwareDecoding{true};
    bool _running{false};
    bool _starting{false};
    bool _stop{false};
    bool _stopping{false};
    bool _streaming{false};
    QString _uri;

};

#endif // GSTPLAYER_H
