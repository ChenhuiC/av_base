#pragma once
#include <QQmlApplicationEngine>
#include <QObject>
#include "Public.h"


class AppModuleManager : public QObject
{
    Q_OBJECT
    DECLARE_SINGLETON(AppModuleManager);
public:
    explicit AppModuleManager(QObject *parent = nullptr);
    ~AppModuleManager();

    // 注册子模块给QML使用
    void registerQMLObjects(QQmlApplicationEngine& engine);
};

inline AppModuleManager& g_AppModuleManager()
{
    return AppModuleManager::instance();
}

inline AppModuleManager* g_AppModuleManagerPtr()
{
    return &AppModuleManager::instance();
}
