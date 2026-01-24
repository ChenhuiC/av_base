add_library(infra_audio STATIC)

# 关键：开启 Qt 的 moc / rcc / uic
set_target_properties(infra_audio PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
)

target_sources(infra_audio
    PRIVATE
        arecord.c
        AudioDeviceManager.cpp
        AudioRecorder.cpp
    PUBLIC
        arecord.h
        AudioDeviceManager.h
        AudioRecorder.h
)

target_include_directories(infra_audio
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(infra_audio
    PUBLIC
        Qt6::Core
        ffmpeg
        utils
)
