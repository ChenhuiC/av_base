#pragma once
#include <QObject>
#include <QString>
#include <QThread>
#include "Public.h"
extern "C" 
{
#include "arecord.h"
}


/**
 * @brief 简单封装 AudioRecorder 单例
 */
class AudioRecorder : public QObject
{
    Q_OBJECT
    DECLARE_SINGLETON(AudioRecorder);
public:

    Q_INVOKABLE void startRecording(const QString& filename = "audio.aac");
    Q_INVOKABLE void stopRecording();
    Q_INVOKABLE bool isRecording() const;

signals:
    void recordingStarted();
    void recordingStopped();

private:
    explicit AudioRecorder(QObject* parent = nullptr);
    ~AudioRecorder();

    bool m_running = false;
    QThread* m_thread = nullptr;
};

// 引用版本
inline AudioRecorder& g_AudioRecorder()
{
    return AudioRecorder::instance();
}

// 指针版本
inline AudioRecorder* g_AudioRecorderPtr()
{
    return &AudioRecorder::instance();
}

