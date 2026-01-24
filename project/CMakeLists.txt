cmake_minimum_required(VERSION 3.16)

project(AudioResample
    VERSION 0.1.0
    LANGUAGES C CXX
)

# ========================
# 基本编译设置
# ========================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

# 生成 compile_commands.json（给 clangd / vscode 用）
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ========================
# Qt6
# ========================
find_package(Qt6 6.4 REQUIRED COMPONENTS
    Core
    Gui
    Qml
    Quick
)

qt_standard_project_setup()



# ========================
# 子模块
# ========================
# add_subdirectory(platform)
add_subdirectory(thirdparty)
add_subdirectory(utils)
add_subdirectory(infra)
add_subdirectory(apps)
