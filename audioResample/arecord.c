/**
* @file: arecord.c
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
#include <string.h>
#include "libswresample/swresample.h"

/* private marcos ------------------------------------------------------------*/
/* private types -------------------------------------------------------------*/
/* private variables ---------------------------------------------------------*/
static int rec_status = REC_STOP;
static uint8_t rec_buf[4096];
static uint32_t wPointer = 0;
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

    SwrContext *swr_ctx = NULL;

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

    const int in_sample_rate = 44100;
    enum AVSampleFormat in_sfmt = AV_SAMPLE_FMT_S16;
    uint64_t in_channel_layout = AV_CH_LAYOUT_STEREO;
    int in_channels = av_get_channel_layout_nb_channels(in_channel_layout);
    const int out_sample_rate = 8000;
    enum AVSampleFormat out_sfmt = AV_SAMPLE_FMT_S32;
    uint64_t out_channel_layout = AV_CH_LAYOUT_STEREO;
    int out_channels = av_get_channel_layout_nb_channels(out_channel_layout);

    int in_nb_samples = sizeof(rec_buf)/2/2;       /* 输入单通道采样个数 */
    int out_nb_samples = av_rescale_rnd(in_nb_samples,out_sample_rate,in_sample_rate,AV_ROUND_UP);      /* 输出单通道采样个数  */

    swr_ctx = swr_alloc_set_opts(NULL,                  /*ctx*/
                                out_channel_layout,     /*输出channel布局*/
                                out_sfmt,               /*输出采样格式*/
                                out_sample_rate,        /*采样率*/
                                in_channel_layout,      /*输入channel布局*/
                                in_sfmt,                /*输入的采样格式*/
                                in_sample_rate,         /*采样率*/
                                0,
                                NULL);
    if(swr_ctx == NULL){
        av_log(NULL, AV_LOG_DEBUG, "swr_alloc_set_opts err\n");
        return;
    }

    if(swr_init(swr_ctx) < 0){
    
    }

    unsigned char **src_data;
    int src_linesize = 0;
    unsigned char **dst_data;
    int dst_linesize = 0;


    /* 创建输入缓冲区 */
    av_samples_alloc_array_and_samples(&src_data,           /*输入缓冲区地址*/
                                        &src_linesize,      /*缓冲区大小*/
                                        in_channels,        /*通道个数*/
                                        in_nb_samples,      /*单通道采样个数*/
                                        in_sfmt,            /*采样格式*/
                                        0);
    /* 创建输出缓冲区 */
    av_samples_alloc_array_and_samples(&dst_data,           /*输出缓冲区地址*/
                                        &dst_linesize,      /*缓冲区大小*/
                                        out_channels,                  /*通道个数*/
                                        out_nb_samples,         /*单通道采样个数*/
                                        out_sfmt,  /*采样格式*/
                                        0);

    rec_status = REC_START;
//    av_init_packet(&pkt);     /* 新的ffmpeg 都不需要初始化 */
    memset(rec_buf,0,sizeof(rec_buf));
    while(((ret = av_read_frame(fmt_ctx,&pkt) ) == 0) && ( rec_status == REC_START )){
        // av_log(NULL, AV_LOG_INFO, "pkt size:%d(%p)\n",pkt.size,pkt.data);
        if(wPointer + pkt.size > sizeof(rec_buf)){
            av_log(NULL,AV_LOG_DEBUG,"wPointer:%d\n",wPointer);
            // fwrite(rec_buf, wPointer, 1, outfile);
            memcpy(src_data[0],rec_buf,wPointer);
            swr_convert(swr_ctx,                        /*重采样上下文*/
                        dst_data,                       /*输出缓冲区*/
                        out_nb_samples,                 /*输出每个通道的采样数*/
                        (const uint8_t **)src_data,     /*输入缓冲区*/
                        in_nb_samples);                 /*输入每个通道的采样数*/
            fwrite(dst_data[0], 1, dst_linesize, outfile);
            fflush(outfile);
            wPointer = 0;
        }
        memcpy(rec_buf + wPointer, pkt.data, pkt.size);
        wPointer += pkt.size;
        av_packet_unref(&pkt);

    }

    av_log(NULL, AV_LOG_DEBUG, "rec_audio end\n");
    /*释放资源*/
    fclose(outfile);
    if(src_data){
        av_freep(&src_data[0]);
    }
    av_freep(&src_data);
    if(dst_data){
        av_freep(&dst_data[0]);
    }
    av_freep(&dst_data);

    avformat_close_input(&fmt_ctx);
    return;
}



/**
 * @brief 设置录制状态
 * 
 * @param status 
 */
void set_status(int status)
{
    rec_status = status;
}
