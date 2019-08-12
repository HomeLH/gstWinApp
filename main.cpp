#include "mainwindow.h"
#include <QApplication>
#include <gst/gst.h>

static void gstputenv(const QString& key, const QString& root, const QString& path)
{
    QString value = root + path;
    qputenv(key.toStdString().c_str(), QByteArray(value.toStdString().c_str()));
}

int main(int argc, char *argv[])
{
#if (QT_VERSION >= QT_VERSION_CHECK(5,9,0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QApplication a(argc, argv);
    QString currentDir = QCoreApplication::applicationDirPath();
    gstputenv("GST_PLUGIN_PATH", currentDir, "/gstreamer-plugins");  // register gstreamer-plugins
    gst_init(nullptr, nullptr);
    MainWindow w;
    w.show();

    return a.exec();
}
