import QtQuick
import QtQuick.Window
import QtQuick.Controls
import App.Core 1.0

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("audioResample")

    Rectangle {
        anchors.fill: parent
        color: "#f0f0f0"

        Column {
            anchors.centerIn: parent
            spacing: 40

            // 音频设备列表
            Column {
                spacing: 20

                ComboBox {
                    id: deviceCombo
                    width: 240
                    model: AudioDeviceManager.deviceList
                    onCurrentTextChanged: {
                        console.log("Selected device:", currentText)
                        AudioDeviceManager.setCurrentDevice(currentText)
                    }
                }

                Button {
                    text: "Refresh Devices"
                    onClicked: AudioDeviceManager.refreshDevices()
                }
            }


            // 录音按钮
            Button {
                id: recordButton
                text: "Start Recording"
                width: 200
                height: 50

                property bool isRecording: false

                onClicked: {
                    if (!isRecording) {
                        console.log("Recording started")
                        recordButton.text = "Stop Recording"
                        isRecording = true
                        AudioRecorder.startRecording()
                    } else {
                        console.log("Recording stopped")
                        recordButton.text = "Start Recording"
                        AudioRecorder.stopRecording()
                        isRecording = false
                    }
                }
            }
        }
    }
}


