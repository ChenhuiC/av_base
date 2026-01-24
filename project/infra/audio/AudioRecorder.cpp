#include "AudioRecorder.h"
#include "AudioDeviceManager.h"
#include <QString>
#include <QDebug>
#include <QByteArray>

AudioRecorder::AudioRecorder(QObject* parent)
    : QObject(parent)
{
}

AudioRecorder::~AudioRecorder()
{
    stopRecording();
}

bool AudioRecorder::isRecording() const
{
    return m_running;
}

void AudioRecorder::startRecording(const QString& filename)
{
    if (m_running) return;

    m_running = true;

    rec_audio();

    emit recordingStarted();
    printf("Recording started: %s\n", filename.toStdString().c_str());
}

void AudioRecorder::stopRecording()
{
    if (!m_running) return;

    set_status(REC_STOP); // 停止录音

    m_running = false;
    emit recordingStopped();
}
