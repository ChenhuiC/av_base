#include "AudioDeviceManager.h"
#include <QDebug>

AudioDeviceManager::AudioDeviceManager(QObject *parent)
    : QObject(parent)
{
    avdevice_register_all();
    // FFmpeg >=4 已经自动注册设备，不需要调用 avdevice_register_all()
    refreshDevices();
}

/**
 * @brief 刷新音频输入设备列表
*/
void AudioDeviceManager::refreshDevices()
{
    m_deviceList.clear();

    // 1. 查找音频输入设备
#if defined(_WIN32)
    const char *fmt_name = "dshow";
#elif defined(__APPLE__)
    const char *fmt_name = "avfoundation";
#else
    const char *fmt_name = "alsa";
#endif
    // 1.1 获取输入格式
    const AVInputFormat* iformat = av_find_input_format(fmt_name);
    if (!iformat) {
        qWarning() << "Can't find input format for" << fmt_name;
        return;
    }

    // 1.2 列出音频输入设备
    AVDeviceInfoList* dev_list = nullptr;
    if (avdevice_list_input_sources(iformat, nullptr, nullptr, &dev_list) < 0) {
        qWarning() << "Failed to list audio input devices";
        return;
    }

    // 1.3 遍历设备列表
    for (int i = 0; i < dev_list->nb_devices; ++i) {
        QString devName = QString::fromUtf8(dev_list->devices[i]->device_name);
        // 过滤可音频输入的设备
        if (devName.startsWith("hw:") ||
            devName.startsWith("plughw:") ||
            devName == "default")
        {
            m_deviceList << devName;
            qDebug() << "Found device:" << devName;
        }
        // m_deviceList << devName;
        // qDebug() << "Found device:" << devName;
    }

    avdevice_free_list_devices(&dev_list);
    emit deviceListChanged();
}


void AudioDeviceManager::setCurrentDevice(const QString &dev)
{
    if (m_currentDevice != dev) {
        m_currentDevice = dev;
        emit currentDeviceChanged();
        qDebug() << "Selected audio device:" << m_currentDevice;
    }
}
