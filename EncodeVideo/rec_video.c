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
    // av_dict_set(&options, "pixel_format", "nv12", 0);        // 指定貌似不生效 

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
        av_log(NULL,AV_LOG_DEBUG,"Failed to find encoder!\n");
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
    (*enc_ctx)->bit_rate = 1000000;      // 600kbps
    // 设置帧率
    (*enc_ctx)->time_base = (AVRational){1,25};        //帧与帧之间的间隔是time_base
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

static void encode(AVCodecContext *enc_ctx,AVFrame *frame,AVPacket *newpkt,FILE *outfile)
{
    int ret = 0;
    if(frame)
        printf("send frame to encoder, pts=%lld\n",frame->pts);

    /*1.将数据给编码器*/
    ret = avcodec_send_frame(enc_ctx,frame);
    if(ret < 0){
        av_log(NULL,AV_LOG_DEBUG,"Failed to send frame to encoder!\n");
        exit(-1);
    }

    while (ret > 0)
    {
        /*2.获取编码后的数据*/
        ret = avcodec_receive_packet(enc_ctx,newpkt);
        if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF){   // 如果编码器数据不足时，返回EAGAIN,  如果data已经全部输出，返回EOF
            return;
        }else if(ret < 0){
            av_log(NULL,AV_LOG_DEBUG,"Failed to receive packet from encoder!\n");
            exit(-1);
        }
        fwrite(newpkt->data,1,newpkt->size,outfile);
        fflush(outfile);
        av_packet_unref(newpkt);            // 减少引用计数
    }
}


void rec_video(void)
{
    int ret;
    int base = 0;
    int i,j;
    AVFormatContext *fmt_ctx = NULL;

    av_log_set_level(AV_LOG_DEBUG);

    /*1.打开设备*/
    fmt_ctx = open_dev();
    if(!fmt_ctx){
        return;
    }

    rec_status = 1;
    // char *out = "video.h264";
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
    uint8_t ubuff[VIDEO_WIDTH*VIDEO_HEIGHT/2] = {0};
    uint8_t vbuff[VIDEO_WIDTH*VIDEO_HEIGHT/2] = {0};
    /*2.read data from device*/
    while((rec_status) && ((ret = av_read_frame(fmt_ctx,&pkt)) == 0)){
        /*3.write to file */
        // av_log(NULL,AV_LOG_DEBUG,"pkt.size:%d\n",pkt.size);                   // pkt.size:614400 = 640x480x2   (YU422)
        // fwrite((void*)pkt.data, 1, pkt.size, outfile);      /* 原始格式 YUYV422  播放时候要指定 -pixel_format yuyv422 */
        // fflush(outfile);

        /*原始格式 YUYV422 :  YUYVYUYV*/
        /*YUV420P: YYYYYYYYUUVV*/
        // memcpy(frame->data[0],pkt.data,VIDEO_WIDTH*VIDEO_HEIGHT);   // Y

        #if 1   // 手动转
        /* 转yuv420p */
        for(i=0;i<VIDEO_WIDTH*VIDEO_HEIGHT;i++){
            frame->data[0][i] = pkt.data[2*i];          // copy Y data
        }
        // UV   422 --> 420 可以丢弃一半数据   丢弃数据时，要考虑到分辨率（uv分布情况）
        /* 没考虑分辨率，色彩不太对 */
        for(int i=0;i<VIDEO_WIDTH*VIDEO_HEIGHT/2;i++){
            ubuff[i] = pkt.data[4*i + 1];    // copy U data  U的下标索引为 1,5,9,13...
            vbuff[i] = pkt.data[4*i + 3];    // copy V data  V的下标索引为 3,7,11,15...
        }

        /*考虑到分辨率 ，以及转换后的uv分布情况，*/
        for(i=0;i<VIDEO_HEIGHT;i++){
            if((i%2) != 0)
                continue;
            for(j=0;j<VIDEO_WIDTH/2;j++){
                frame->data[1][(i*VIDEO_WIDTH)/4+j] = ubuff[i*VIDEO_WIDTH/2 + j];
            }
        }
        for(i =0;i<VIDEO_HEIGHT;i++){
            if((i%2) != 0)
                continue;
            for(j=0;j<VIDEO_WIDTH/2;j++){
                frame->data[2][(i*VIDEO_WIDTH)/4+j] = vbuff[i*VIDEO_WIDTH/2 + j];
            }
        }

        fwrite(frame->data[0],1,VIDEO_WIDTH*VIDEO_HEIGHT,outfile);
        fwrite(frame->data[1],1,VIDEO_WIDTH*VIDEO_HEIGHT/4,outfile);
        fwrite(frame->data[2],1,VIDEO_WIDTH*VIDEO_HEIGHT/4,outfile);
        fflush(outfile);
        #endif
        

        // frame->pts = base++;
        // encode(enc_ctx,frame,newpkt,outfile);
        // avcodec_send_frame();
        // avcodec_receive_packet();
        // av_packet_unref(&pkt);
    }
    // encode(enc_ctx,NULL,newpkt,outfile);

    av_log(NULL,AV_LOG_DEBUG,"rec_video end!\n");
    fclose(outfile);

__ERROR:
    if(frame){
        av_frame_free(&frame);
    }

}
