## 软件介绍
本软件是一个基于Qt和FFmpeg的多功能视频处理工具，支持视频
- 系统环境 : Ubuntu 22.04
- qt版本 : 6.9.3
- ffmpeg版本 : 8.0
- 构建工具 : CMake


## 编译
- 命令行编译
```bash
mkdir build
cd build
cmake ..
make -j4
```
- Qt Creator编译
  - 打开Qt Creator，选择“打开项目”，选择项目根目录下的CMakeLists.txt文件。
  - 配置构建套件，选择合适的Qt版本和编译器。
  - 点击“构建”按钮进行编译。

## 运行
- 命令行运行
```bash
./VideoProcessor
```
- Qt Creator运行
点击“运行”按钮即可启动应用程序。
