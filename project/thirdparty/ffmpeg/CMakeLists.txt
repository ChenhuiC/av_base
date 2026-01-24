add_library(ffmpeg INTERFACE)

# include
target_include_directories(ffmpeg INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# link library（使用完整路径更安全）
target_link_libraries(ffmpeg INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/libavdevice.so
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/libavformat.so
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/libavcodec.so
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/libavutil.so
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/libswresample.so
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/libswscale.so
)


install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/libavdevice.so
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/libavformat.so
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/libavcodec.so
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/libavutil.so
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/libswresample.so
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/libswscale.so
    DESTINATION lib
)
