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


static SwrContext *create_swr_ctx_t(int i_rate,enum AVSampleFormat i_sfmt,uint64_t i_channel_layout,int o_rate,enum AVSampleFormat o_sfmt,uint64_t o_channel_layout)
{
    SwrContext *swr_ctx;
    /*1. 创建重采样上下文； 2.设置参数*/
    swr_ctx = swr_alloc_set_opts(NULL,                  /*ctx*/
                                o_channel_layout,     /*输出channel布局*/
                                o_sfmt,               /*输出采样格式*/
                                o_rate,        /*采样率*/
                                i_channel_layout,      /*输入channel布局*/
                                i_sfmt,                /*输入的采样格式*/
                                i_rate,         /*采样率*/
                                0,
                                NULL);
    if(swr_ctx == NULL){
        av_log(NULL, AV_LOG_DEBUG, "swr_alloc_set_opts err\n");
        return NULL;
    }
    /* 3. 重采样初始化  */
    if(swr_init(swr_ctx) < 0){
        av_log(NULL, AV_LOG_DEBUG, "swr_init err\n");
        // TODO 释放其他资源 
        return NULL;
    }
    return swr_ctx;
}

/**
 * @brief 打开AAC编码器
 * 
 * @return AVCodecContext* 
 */
static AVCodecContext *open_coder(void)
{
    /*1. 创建编码器*/
//    AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
     AVCodec *codec = (AVCodec *)avcodec_find_encoder_by_name("libfdk_aac");
    if(!codec){
        av_log(NULL, AV_LOG_ERROR, "can't find encoder\n");
        return NULL;
    }
    /*2. 创建codec上下文*/
    AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
    if(!codec_ctx){
        av_log(NULL, AV_LOG_ERROR, "can't alloc context\n");
        return NULL;
    }
    /* 这里应该对应重采样输出的三元组信息 */
    codec_ctx->sample_fmt = AV_SAMPLE_FMT_S16;          /*输入音频的采样大小*/
    codec_ctx->channel_layout = AV_CH_LAYOUT_STEREO;    /*输入音频的channel layout*/
    codec_ctx->channels = 2;                            /*输入音频channel个数*/
    codec_ctx->sample_rate = 48000;                     /*输入音频的采样率*/
    codec_ctx->bit_rate = 0;                            /* 码流，当设置为0的时候 通过profile来进行设置 */
    codec_ctx->profile = FF_PROFILE_AAC_HE_V2;          /* AAC_LC:128K, AAC_HE:64K, AAC_HE_V2:32K  */

    /*3. 打开编码器*/
    if(avcodec_open2(codec_ctx,codec,NULL) < 0){

        return NULL;
    }
    return codec_ctx;
}

/**
 * @brief 
 * 
 * @param ctx 
 * @param frame 
 * @param pkt 
 * @param outfile 
 */
void encode(AVCodecContext *ctx, AVFrame *frame, AVPacket *pkt, FILE *outfile)
{
    int ret = 0;
    /* 将数据发送到编码器 */
    ret = avcodec_send_frame(ctx, frame);
    while(ret >= 0){
        /*获取编码后的音频数据，如果成功，需要重复获取，直到失败为止*/
        ret = avcodec_receive_packet(ctx, pkt);
        if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF){
            return;
        }else if(ret < 0){
            av_log(NULL, AV_LOG_DEBUG, "Error, encoding audio frame\n");
            exit(-1);
        }
        /* write file */
        fwrite(pkt->data, 1, pkt->size, outfile);
        fflush(outfile);
    }

    return;
}

static AVFormatContext* open_dev()
{
    AVFormatContext *fmt_ctx = NULL;
    const char *devicename = "hw:0,0";
    AVDictionary *options = NULL;
    AVInputFormat *iformat = NULL;
    int ret;
    char errbuf[1024];

    /*1.打开输入设备*/
    /* 1.1. 注册音频设备 */
    avdevice_register_all();

    /* 1.2. 设置采集方式 (avfoundation/dshow/alsa,分别对应mac/window/linux)*/
    iformat = av_find_input_format("alsa");

    /* 1.3. 打开音频设备 */
    if((ret = avformat_open_input(&fmt_ctx, devicename,iformat, &options)) != 0){
        av_strerror(ret, errbuf, 1024);
        av_log(NULL, AV_LOG_DEBUG, "Failed to open audio device, err code: [%d],err: %s\n", ret,errbuf);
        return NULL;
    }
    av_log(NULL, AV_LOG_DEBUG, "Open audio device success\n");

    return fmt_ctx;
}


static AVFrame* createFrame(int nb_samples,enum AVSampleFormat sfmt,uint64_t channel_layout)
{
    AVFrame *frame = av_frame_alloc();
    if(frame == NULL){
        av_log(NULL, AV_LOG_DEBUG, "av_frame_alloc err\n");
        return NULL;
    }
    frame->nb_samples = nb_samples;           /*单通道一个音频帧的采样数*/
    frame->format = sfmt;    /*每个采样的大小*/
    frame->channel_layout = channel_layout;
    av_frame_get_buffer(frame, 0);      /*创建buffer大小 三元组相乘*/
    if(!frame->buf[0]){
        av_log(NULL, AV_LOG_DEBUG, "av_frame_get_buffer err\n");
        /*释放frame*/
        return NULL;
    }
    return frame;
}

/**
 * @brief 录制音频
 * 
 */
void rec_audio(void)
{
    int ret = 0;
    AVFormatContext *fmt_ctx = NULL;

    AVPacket pkt;

    SwrContext *swr_ctx = NULL;

    av_log_set_level(AV_LOG_DEBUG);
    av_log(NULL, AV_LOG_DEBUG, "this is a log\n");

    /* create file */
    char *filename = "audio.aac";
    FILE *outfile = fopen(filename,"wb+");
    if(outfile == NULL){
        av_log(NULL, AV_LOG_DEBUG, "fopen err\n");
        return;
    }

    /* open dev */
    fmt_ctx = open_dev();
    if(fmt_ctx == NULL){
        av_log(NULL, AV_LOG_DEBUG, "open dev err\n");
        return;
    }

    /* 重采样输入输出三元组相关信息 */
    const int in_sample_rate = 48000;
    enum AVSampleFormat in_sfmt = AV_SAMPLE_FMT_S16;
    uint64_t in_channel_layout = AV_CH_LAYOUT_STEREO;
    int in_channels = av_get_channel_layout_nb_channels(in_channel_layout);
    const int out_sample_rate = 48000;
    enum AVSampleFormat out_sfmt = AV_SAMPLE_FMT_S16;
    uint64_t out_channel_layout = AV_CH_LAYOUT_STEREO;
    int out_channels = av_get_channel_layout_nb_channels(out_channel_layout);
    /*创建重采样上下文*/
    swr_ctx = create_swr_ctx_t(in_sample_rate,in_sfmt,in_channel_layout,out_sample_rate,out_sfmt,out_channel_layout);
    if(swr_ctx == NULL){
        av_log(NULL, AV_LOG_DEBUG, "create swr_ctx_t err\n");
        return;
    }

    int in_nb_samples = sizeof(rec_buf)/2/2;       /* 输入单通道采样个数 = 采样总数/通道数/采样大小/8 */
    int out_nb_samples = av_rescale_rnd(in_nb_samples,out_sample_rate,in_sample_rate,AV_ROUND_UP);      /* 输出单通道采样个数  */
    av_log(NULL,AV_LOG_DEBUG,"out nb samples is %d\n",out_nb_samples);

    /* 打开编码器 */
    AVCodecContext *c_ctx = open_coder();
    if(c_ctx == NULL){
        av_log(NULL, AV_LOG_DEBUG, "open coder err\n");
        return;
    }

    /* 音频输入数据  AVFrame 保存未编码的数据 */
    AVFrame* frame = createFrame(out_nb_samples,out_sfmt,out_channel_layout);
    if(frame == NULL){
        av_log(NULL, AV_LOG_DEBUG, "create frame err\n");
        return;
    }

    /* 分配编码后的数据空间 AVPacket基本都是 存放编码后的数据*/
    AVPacket *newpkt = av_packet_alloc();
    if(newpkt == NULL){
        av_log(NULL, AV_LOG_DEBUG, "av_packet_alloc err\n");
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
    /* TODO 优化：读取失败或不能立即读到数据的处理，超时处理，以及退出后主线程中状态的同步 */
    while(((ret = av_read_frame(fmt_ctx,&pkt) ) == 0) && ( rec_status == REC_START )){
        // av_log(NULL, AV_LOG_INFO, "pkt size:%d(%p)\n",pkt.size,pkt.data);
        if(wPointer + pkt.size > sizeof(rec_buf)){
            // av_log(NULL,AV_LOG_DEBUG,"wPointer:%d\n",wPointer);
            // fwrite(rec_buf, wPointer, 1, outfile);
            memcpy(src_data[0],rec_buf,wPointer);
            /* 4. 进行重采样 */
            swr_convert(swr_ctx,                        /*重采样上下文*/
                        dst_data,                       /*输出缓冲区*/
                        out_nb_samples,                 /*输出每个通道的采样数*/
                        (const uint8_t **)src_data,     /*输入缓冲区*/
                        in_nb_samples);                 /*输入每个通道的采样数*/

            /* 将重采样的数据拷贝到 frame中 */
            memcpy((void *)frame->data[0], (void *)dst_data[0], dst_linesize);

            // /* 编码 */
            encode(c_ctx, frame, newpkt,outfile);

            /* write file 测试重采样 */
            // fwrite((void *)dst_data[0], 1, dst_linesize, outfile);
            // fflush(outfile);

            wPointer = 0;
        }
        memcpy(rec_buf + wPointer, pkt.data, pkt.size);
        wPointer += pkt.size;
        av_packet_unref(&pkt);

    }
    /* 强制将编码器中的数据进行编码 */
    encode(c_ctx, NULL, newpkt,outfile);    /* 让编码器吐出最后一点数据 */

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

    /* 释放重采样的上下文 */
    swr_free(&swr_ctx);
    /* 释放 AVFrame AVPacket */

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
