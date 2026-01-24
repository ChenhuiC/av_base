#pragma once
#include <QObject>
#include <QString>
#include <QStringList>
#include "Public.h"

extern "C" {
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
}

class AudioDeviceManager : public QObject
{
    Q_OBJECT
    DECLARE_SINGLETON(AudioDeviceManager);
    Q_PROPERTY(QStringList deviceList READ deviceList NOTIFY deviceListChanged)
    Q_PROPERTY(QString currentDevice READ currentDevice WRITE setCurrentDevice NOTIFY currentDeviceChanged)

public:
    explicit AudioDeviceManager(QObject *parent = nullptr);

    Q_INVOKABLE void refreshDevices();

    QStringList deviceList() const { return m_deviceList; }
    QString currentDevice() const { return m_currentDevice; }
    Q_INVOKABLE void setCurrentDevice(const QString &dev);

signals:
    void deviceListChanged();
    void currentDeviceChanged();

private:
    QStringList m_deviceList;
    QString m_currentDevice;
};

// 引用版本
inline AudioDeviceManager& g_AudioDeviceManager()
{
    return AudioDeviceManager::instance();
}

// 指针版本
inline AudioDeviceManager* g_AudioDeviceManagerPtr()
{
    return &AudioDeviceManager::instance();
}
