#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "AppModuleManager.h"
#include "AudioDeviceManager.h"
#include "AudioRecorder.h"

AppModuleManager::AppModuleManager(QObject *parent)
    : QObject(parent)
{

}

AppModuleManager::~AppModuleManager()
{

}

void AppModuleManager::registerQMLObjects(QQmlApplicationEngine& engine)
{
    // 在这里注册子模块的 QML 对象
    // 例如：
    qmlRegisterSingletonInstance("App.Core", 1, 0, "AudioDeviceManager", &g_AudioDeviceManager());
    qmlRegisterSingletonInstance("App.Core", 1, 0, "AudioRecorder", &g_AudioRecorder());
}
