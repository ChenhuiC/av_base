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
#define VIDEO_WIDTH 640
#define VIDEO_HEIGHT 480
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

/**
 * @brief 
 * 
 * @param width 
 * @param heigh 
 * @param[out] enc_ctx 
 * @return int 
 */
static int open_encoder(int width,int heigh,AVCodecContext **enc_ctx)
{
    AVCodec *codec = NULL;
    int ret;

    /*1. 查找编码器*/
    codec = avcodec_find_encoder_by_name("libx264");
    if(!codec){

        return -1;
    }
    /*2. 获取上下文*/
    *enc_ctx = avcodec_alloc_context3(codec);
    if(!(*enc_ctx)){
        av_log(NULL,AV_LOG_DEBUG,"Failed to alloc context!\n");
        return -1;
    }
    /*3. 设置参数*/
    /*SPS/PPS*/
    (*enc_ctx)->profile = FF_PROFILE_H264_HIGH_444;
    (*enc_ctx)->level = 50; /*表示LEVEL是5.0*/
    /*设置分辨率*/
    (*enc_ctx)->width = width;
    (*enc_ctx)->height = heigh;
    /*GOP*/
    (*enc_ctx)->gop_size = 250;
    (*enc_ctx)->keyint_min = 25;
    /*设置B帧数据*/
    (*enc_ctx)->max_b_frames = 3;
    (*enc_ctx)->has_b_frames = 1;
    /*参考帧的数量*/
    (*enc_ctx)->refs = 3;
    // 设置输入YUV格式
    (*enc_ctx)->pix_fmt = AV_PIX_FMT_YUV420P;
    // 设置码率
    (*enc_ctx)->bit_rate = 600000;
    // 设置帧率
    (*enc_ctx)->time_base = (AVRational){1,25};
    (*enc_ctx)->framerate = (AVRational){25,1};     // 帧率， 每秒 25 帧

    ret = avcodec_open2(*enc_ctx,codec,NULL);
    if(ret < 0){
        av_log(NULL,AV_LOG_DEBUG,"Failed to open encoder!\n");
        return -1;
    }
    return 0;
}

static AVFrame *create_frame(int width,int height)
{
    int ret = 0;
    AVFrame *frame = NULL;
    frame = av_frame_alloc();
    if(!frame){
        av_log(NULL,AV_LOG_DEBUG,"Failed to alloc frame!\n");
        return NULL;
    }

    frame->width = width;
    frame->height = height;
    frame->format = AV_PIX_FMT_YUV420P;

    // alloc inner memory
    ret = av_frame_get_buffer(frame, 32);   // 按 32 位对齐
    if(ret < 0){
        av_log(NULL,AV_LOG_DEBUG,"Failed to get buffer!\n");
        goto __ERROR;
    }
    return frame;

__ERROR:
    if(frame){
        av_frame_free(&frame);
    }
    return NULL;
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

    AVCodecContext *enc_ctx = NULL;
    ret = open_encoder(VIDEO_WIDTH,VIDEO_HEIGHT,&enc_ctx);
    if(-1 == ret){
        return;
    }
    av_log(NULL,AV_LOG_DEBUG,"open encoder success!\n");

    /*创建AVFrame*/
    AVFrame *frame = create_frame(VIDEO_WIDTH,VIDEO_HEIGHT);
    if(!frame){
        av_log(NULL,AV_LOG_DEBUG,"Failed to alloc frame!\n");
        goto __ERROR;
    }

    /*创建AVPakcet*/
    AVPacket *newpkt = av_packet_alloc();
    if(!newpkt){
        av_log(NULL,AV_LOG_DEBUG,"Failed to alloc packet!\n");
        goto __ERROR;
    }

    AVPacket pkt;

    /*2.read data from device*/
    while((rec_status) && ((ret = av_read_frame(fmt_ctx,&pkt)) == 0)){


        /*3.write to file */
        // av_log(NULL,AV_LOG_DEBUG,"pkt.size:%d\n",pkt.size);                   // pkt.size:614400 = 640x480x2   (YU422)
        // fwrite((void*)pkt.data, 1, pkt.size, outfile);
        // fflush(outfile);

        /*原始格式 YUYV422 :  YUYVYUYV*/
        /*YUV420P: YYYYYYYYUUVV*/
        // memcpy(frame->data[0],pkt.data,VIDEO_WIDTH*VIDEO_HEIGHT);   // Y

        for(int i=0;i<VIDEO_WIDTH*VIDEO_HEIGHT;i++){
            frame->data[0][i] = pkt.data[2*i];
        }
        // UV   422 --> 420 可以丢弃一般数据
        for(int i=0;i<VIDEO_WIDTH*VIDEO_HEIGHT/4;i++){
            frame->data[1][i] = pkt.data[2*4*i + 1];
            frame->data[2][i] = pkt.data[2*4*i + 3];
        }

        fwrite(frame->data[0],1,VIDEO_WIDTH*VIDEO_HEIGHT,outfile);
        fwrite(frame->data[1],1,VIDEO_WIDTH*VIDEO_HEIGHT/4,outfile);
        fwrite(frame->data[2],1,VIDEO_WIDTH*VIDEO_HEIGHT/4,outfile);
        fflush(outfile);
        // avcodec_send_frame();
        // avcodec_receive_packet();
    }
    av_log(NULL,AV_LOG_DEBUG,"rec_video end!\n");
    fclose(outfile);

__ERROR:
    if(frame){
        av_frame_free(&frame);
    }

}
