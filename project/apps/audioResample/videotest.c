/**
* @file: videotest.c
* @brief: 
* @author: chenhui
* @created: 2023-11-15 21:29:13
* 
* @copyright (C), 2008-2023
* 
*/
/* includes ------------------------------------------------------------------*/
#include "videotest.h"

/* private marcos ------------------------------------------------------------*/
/* private types -------------------------------------------------------------*/
/* private variables ---------------------------------------------------------*/
/* private functions ---------------------------------------------------------*/

static int rec_status = 0;

void set_status(int status)
{
    rec_status = status;
}

static AVFormatContext* open_dev()
{
    int ret;
    AVFormatContext *fmt_ctx = NULL;
    const char *devicename = "/dev/video0";
    AVDictionary *options = NULL;
    AVInputFormat *iformat = NULL;
    char errbuf[1024];

    /*设置options*/
    av_dict_set(&options, "video_size", "640x480", 0);
    av_dict_set(&options, "framerate", "30", 0);
    // av_dict_set(&options, "pixel_format", "nv12", 0);

    /*1.打开输入设备*/
    /* 1.1. 注册所有设备 */
    avdevice_register_all();

    /* 1.2. 设置采集方式 (avfoundation/dshow/alsa,分别对应mac/window/linux)*/
    // iformat = av_find_input_format("alsa");

    /* 1.3. 打开音频设备 */
    if((ret = avformat_open_input(&fmt_ctx, devicename,iformat, &options)) != 0){
        av_strerror(ret, errbuf, 1024);
        av_log(NULL, AV_LOG_DEBUG, "Failed to open video device, err code: [%d],err: %s\n", ret,errbuf);
        return NULL;
    }
    av_log(NULL, AV_LOG_DEBUG, "Open video device success\n");

    return fmt_ctx;
}

void rec_video(void)
{
    int ret;
    AVFormatContext *fmt_ctx = NULL;
    AVPacket pkt;

    //set log level
    av_log_set_level(AV_LOG_DEBUG);
    
    //start record
    rec_status = 1;
    char *out = "video.yuv";
    FILE *outfile = fopen(out, "wb+");

    //打开设备
    fmt_ctx = open_dev();

    //read data from device
    while((ret = av_read_frame(fmt_ctx, &pkt)) == 0 && rec_status) {
        
        int i = 0;
        
        av_log(NULL, AV_LOG_INFO,"packet size is %d(%p)\n",pkt.size, pkt.data);
        
        fwrite((void*)pkt.data, 1, pkt.size, outfile);

    }

    fclose(outfile);

    return;
}