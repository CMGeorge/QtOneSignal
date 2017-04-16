#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "onesignal.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    OneSignal *os = new OneSignal();
    os->Init("db7b05f1-857d-423d-b1e8-1e8d5e982f13");
    engine.load(QUrl(QLatin1String("qrc:/main.qml")));
    return app.exec();
}
