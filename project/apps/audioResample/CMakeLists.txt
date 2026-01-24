qt_add_executable(audioResample
    main.cpp
    AppModuleManager.cpp
    AppModuleManager.h
    # videotest.c
    # videotest.h
)

# QML
qt_add_qml_module(audioResample
    URI AudioResample
    VERSION 1.0
    QML_FILES
        qml/Main.qml
)

target_link_libraries(audioResample
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        infra_audio
        ffmpeg
)

target_include_directories(audioResample
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(audioResample PROPERTIES
    INSTALL_RPATH "$ORIGIN/../lib"
)

# 安装规则
install(TARGETS audioResample
    RUNTIME DESTINATION bin
)