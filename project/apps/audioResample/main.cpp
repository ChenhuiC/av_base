#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QCoreApplication>
#include <QtCore/qurl.h>
#include "AppModuleManager.h"


int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    g_AppModuleManager().registerQMLObjects(engine);

    engine.loadFromModule("AudioResample", "Main");

    if (engine.rootObjects().isEmpty())
        return -1;

    int ret = app.exec();
    return ret;
}
