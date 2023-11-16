
# 采集音频
1. 命令方式
ffmpeg -f alsa -i hw:0,0 try.wav

2. 代码方式
默认采集率 44100 通道数 2  采样大小16bit

# 播放音频

1. 播放pcm
ffplay -ar 44100 -ac 2 -f s16le test.pcm


# 重采样
## 用到的数据
1. 上下文  SwrContext
    初始化该上下文 swr_alloc_set_opts(参数...)
## 用到的函数
1.    int swr_convert(struct SwrContext *s, uint8_t **out, int out_count,
                                const uint8_t **in , int in_count);



# 问题及解决方法

1. -alsa-调节音频音量大小
获取各参数当前的值：
amixer contents
设置录音音频参数
amixer cset numid=20,iface=MIXER,name='Capture Volume' 13

2. 请教一下，例子中输入是 f32，但输出是 s16，f32 是4字节所以 输入单通道采样个数是 4096/4/2 = 512，但输出单通道采样个数为什么也是 512 ？ s16 不是应该是 2字节，4096/2/2 = 1024 吗 ？
要保持单通道下输入与输出采样个数一样，变量是采样位深

# 参考
## 重采样参考：https://blog.csdn.net/wanggao_1990/article/details/115731502


# 音频处理流程

1. 采集音频
    流程
    1. 打开输入设备
        输入设备名称：

    2. 数据包
    3. 输出文件（如果无后续操作的话）

2. 重采样

3. 编码

4. 本地存储/传输



# 视频

## 视频录制

播放测试：
ffplay -pixel_format nv12  -video_size 640x480 video.yuv

