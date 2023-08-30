/**
* @file: test.c
* @brief: 
        默认采样率 44100 通道数 2 采样位数 f32lebit
* @author: chenhui
* @created: 2023-08-29 21:24:49
* 
* @copyright (C), 2008-2023
* 
*/
/* includes ------------------------------------------------------------------*/
#include "arecord.h"

/* private marcos ------------------------------------------------------------*/
/* private types -------------------------------------------------------------*/
/* private variables ---------------------------------------------------------*/
static int rec_status = REC_STOP;

/* private functions ---------------------------------------------------------*/

void rec_audio(void)
{
    int ret = 0;
    char errbuf[1024];
    AVFormatContext *fmt_ctx = NULL;
    const char *devicename = "hw:0,0";
    AVDictionary *options = NULL;
    AVInputFormat *iformat = NULL;
    AVPacket pkt;

    av_log_set_level(AV_LOG_DEBUG);
    av_log(NULL, AV_LOG_DEBUG, "this is a log\n");

    /* 注册音频设备 */
    avdevice_register_all();

    /* 设置采集方式 */
    iformat = av_find_input_format("alsa");

    /* 打开音频设备 */
    if((ret = avformat_open_input(&fmt_ctx, devicename,iformat, &options)) != 0){
        av_strerror(ret, errbuf, 1024);
        av_log(NULL, AV_LOG_DEBUG, "avformat_open_input err:%s\n", errbuf);
    }else{
        av_log(NULL, AV_LOG_DEBUG, "avformat_open_input success\n");
    }

    /* create file */
    char *filename = "test.pcm";
    FILE *outfile = fopen(filename,"wb+");
    if(outfile == NULL){
        av_log(NULL, AV_LOG_DEBUG, "fopen err\n");
        return;
    }
    rec_status = REC_START;
//    av_init_packet(&pkt);     /* 新的ffmpeg 都不需要初始化 */
    while(((ret = av_read_frame(fmt_ctx,&pkt) ) == 0) && ( rec_status == REC_START )){
        av_log(NULL, AV_LOG_INFO, "pkt size:%d(%p)\n",pkt.size,pkt.data);
        fwrite(pkt.data, pkt.size, 1, outfile);
        fflush(outfile);
        av_packet_unref(&pkt);

    }

    av_log(NULL, AV_LOG_DEBUG, "rec_audio end\n");
    /*释放资源*/
    fclose(outfile);
    avformat_close_input(&fmt_ctx);
    return;
}

void set_status(int status)
{
    rec_status = status;
}
