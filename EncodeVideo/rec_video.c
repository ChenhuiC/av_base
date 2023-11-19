/**
* @file: rec_video.c
* @brief: 
* @author: chenhui
* @created: 2023-11-17 22:09:32
* 
* @copyright (C), 2008-2023
* 
*/
/* includes ------------------------------------------------------------------*/
#include "rec_video.h"

/* private marcos ------------------------------------------------------------*/
/* private types -------------------------------------------------------------*/
/* private variables ---------------------------------------------------------*/
/* private functions ---------------------------------------------------------*/

static int rec_status = 0;

void set_status(int status)
{
    rec_status = status;
}

AVFormatContext* open_dev()
{
    int ret;
    AVFormatContext *fmt_ctx = NULL;
    const char *devicename = "/dev/video0";
    AVDictionary *options = NULL;
    char errbuf[1024];

    /*设置options*/
    av_dict_set(&options,"video_size","640x480",0);
    av_dict_set(&options,"framerate","30",0);
    av_dict_set(&options, "pixel_format", "nv12", 0);

    /*1.1注册所有设备*/
    avdevice_register_all();

    /*1.2打开视频设备*/
    if(ret = avformat_open_input(&fmt_ctx,devicename,NULL,&options)){
        av_strerror(ret,errbuf,1024);
        av_log(NULL,AV_LOG_DEBUG,"Failed to opoen video, err code:%s\n",errbuf);
        return NULL;
    }
    av_log(NULL,AV_LOG_DEBUG,"Open video device success!\n");
    return fmt_ctx;

}

static int open_encoder(int width,int heigh,AVFormatContext **fmt)
{


    return 0;
}


void rec_video(void)
{
    int ret;
    AVFormatContext *fmt_ctx = NULL;

    av_log_set_level(AV_LOG_DEBUG);

    /*1.打开设备*/
    fmt_ctx = open_dev();
    if(!fmt_ctx){
        return;
    }

    rec_status = 1;
    char *out = "video.yuv";
    FILE *outfile = fopen(out, "wb+");

    // ret = open_encoder(640,480,&fmt_ctx);
    // if()

    AVPacket pkt;

    /*2.read data from device*/
    while((rec_status) && ((ret = av_read_frame(fmt_ctx,&pkt)) == 0)){


        /*3.write to file */
        fwrite((void*)pkt.data, 1, pkt.size, outfile);
    }
    av_log(NULL,AV_LOG_DEBUG,"rec_video end!\n");
    fclose(outfile);


}
