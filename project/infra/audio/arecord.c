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
/**
 * @file arecord.c
 * @brief Audio record + resample + AAC encode (FFmpeg 8.0)
 */

#include "arecord.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libavutil/avutil.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libswresample/swresample.h>

/* -------------------------------------------------------------------------- */

static int rec_status = REC_STOP;
static uint8_t rec_buf[4096];
static uint32_t wPointer = 0;

/* -------------------------------------------------------------------------- */
/* SwrContext */

static SwrContext *create_swr_ctx_t(
        int in_rate, enum AVSampleFormat in_fmt, uint64_t in_ch_mask,
        int out_rate, enum AVSampleFormat out_fmt, uint64_t out_ch_mask)
{
    SwrContext *swr_ctx = swr_alloc();
    if (!swr_ctx) {
        return NULL;
    }

    AVChannelLayout in_layout;
    AVChannelLayout out_layout;

    av_channel_layout_from_mask(&in_layout, in_ch_mask);
    av_channel_layout_from_mask(&out_layout, out_ch_mask);

    av_opt_set_chlayout(swr_ctx, "in_chlayout",  &in_layout,  0);
    av_opt_set_chlayout(swr_ctx, "out_chlayout", &out_layout, 0);
    av_opt_set_int(swr_ctx, "in_sample_rate",    in_rate,     0);
    av_opt_set_int(swr_ctx, "out_sample_rate",   out_rate,    0);
    av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt",  in_fmt,  0);
    av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", out_fmt, 0);

    if (swr_init(swr_ctx) < 0) {
        swr_free(&swr_ctx);
        return NULL;
    }

    return swr_ctx;
}

/* -------------------------------------------------------------------------- */
/* AAC encoder */

static AVCodecContext *open_coder(void)
{
    const AVCodec *codec = avcodec_find_encoder_by_name("libfdk_aac");
    if (!codec) {
        av_log(NULL, AV_LOG_ERROR, "libfdk_aac not found\n");
        return NULL;
    }

    AVCodecContext *ctx = avcodec_alloc_context3(codec);
    if (!ctx) {
        av_log(NULL, AV_LOG_ERROR, "avcodec_alloc_context3 failed\n");
        return NULL;
    }

    ctx->codec_id   = AV_CODEC_ID_AAC;
    ctx->sample_fmt = AV_SAMPLE_FMT_S16;
    ctx->sample_rate = 48000;
    ctx->bit_rate   = 128000;
    ctx->profile    = AV_PROFILE_AAC_LOW;

    /* FFmpeg 8.x channel layout */
    av_channel_layout_default(&ctx->ch_layout, 2);

    if (avcodec_open2(ctx, codec, NULL) < 0) {
        avcodec_free_context(&ctx);
        return NULL;
    }

    return ctx;
}

/* -------------------------------------------------------------------------- */

static AVFrame *create_frame(int nb_samples,
                             enum AVSampleFormat fmt,
                             uint64_t ch_mask)
{
    AVFrame *frame = av_frame_alloc();
    if (!frame) {
        return NULL;
    }

    frame->nb_samples = nb_samples;
    frame->format     = fmt;
    frame->sample_rate = 48000;

    av_channel_layout_from_mask(&frame->ch_layout, ch_mask);

    if (av_frame_get_buffer(frame, 0) < 0) {
        av_frame_free(&frame);
        return NULL;
    }

    return frame;
}

/* -------------------------------------------------------------------------- */

static void encode(AVCodecContext *ctx,
                   AVFrame *frame,
                   AVPacket *pkt,
                   FILE *outfile)
{
    int ret = avcodec_send_frame(ctx, frame);
    if (ret < 0) {
        return;
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        }
        fwrite(pkt->data, 1, pkt->size, outfile);
        av_packet_unref(pkt);
    }
}

/* -------------------------------------------------------------------------- */
/* ALSA input */

static AVFormatContext *open_dev(void)
{
    AVFormatContext *fmt_ctx = NULL;
    const char *device = "hw:0,0";

    const AVInputFormat *iformat = av_find_input_format("alsa");
    if (!iformat) {
        return NULL;
    }

    if (avformat_open_input(&fmt_ctx, device, iformat, NULL) < 0) {
        return NULL;
    }

    return fmt_ctx;
}

/* -------------------------------------------------------------------------- */

void rec_audio(void)
{
    AVFormatContext *fmt_ctx = NULL;
    SwrContext *swr_ctx = NULL;
    AVCodecContext *c_ctx = NULL;
    AVFrame *frame = NULL;
    AVPacket *pkt = NULL;

    FILE *outfile = fopen("audio.aac", "wb");
    if (!outfile) {
        return;
    }

    fmt_ctx = open_dev();
    if (!fmt_ctx) {
        goto end;
    }

    /* input / output params */
    const int in_rate  = 48000;
    const int out_rate = 48000;

    const enum AVSampleFormat in_fmt  = AV_SAMPLE_FMT_S16;
    const enum AVSampleFormat out_fmt = AV_SAMPLE_FMT_S16;

    const uint64_t in_ch_mask  = AV_CH_LAYOUT_STEREO;
    const uint64_t out_ch_mask = AV_CH_LAYOUT_STEREO;

    AVChannelLayout in_layout;
    av_channel_layout_from_mask(&in_layout, in_ch_mask);
    int in_channels = in_layout.nb_channels;

    AVChannelLayout out_layout;
    av_channel_layout_from_mask(&out_layout, out_ch_mask);
    int out_channels = out_layout.nb_channels;

    swr_ctx = create_swr_ctx_t(
        in_rate, in_fmt, in_ch_mask,
        out_rate, out_fmt, out_ch_mask);

    if (!swr_ctx) {
        goto end;
    }

    c_ctx = open_coder();
    if (!c_ctx) {
        printf("open coder failed\n");
        goto end;
    }

    int in_nb_samples  = sizeof(rec_buf) / (2 * in_channels);
    int out_nb_samples = av_rescale_rnd(
        in_nb_samples, out_rate, in_rate, AV_ROUND_UP);

    frame = create_frame(out_nb_samples, out_fmt, out_ch_mask);
    if (!frame) {
        goto end;
    }

    pkt = av_packet_alloc();
    if (!pkt) {
        goto end;
    }

    uint8_t **src_data = NULL;
    uint8_t **dst_data = NULL;
    int src_linesize = 0;
    int dst_linesize = 0;

    av_samples_alloc_array_and_samples(
        &src_data, &src_linesize,
        in_channels, in_nb_samples, in_fmt, 0);

    av_samples_alloc_array_and_samples(
        &dst_data, &dst_linesize,
        out_channels, out_nb_samples, out_fmt, 0);

    rec_status = REC_START;
    wPointer = 0;

    AVPacket in_pkt;

    while (rec_status == REC_START &&
           av_read_frame(fmt_ctx, &in_pkt) == 0) {

        if (wPointer + in_pkt.size > sizeof(rec_buf)) {

            memcpy(src_data[0], rec_buf, wPointer);

            swr_convert(swr_ctx,
                        dst_data, out_nb_samples,
                        (const uint8_t **)src_data, in_nb_samples);

            memcpy(frame->data[0], dst_data[0], dst_linesize);

            encode(c_ctx, frame, pkt, outfile);
            wPointer = 0;
        }

        memcpy(rec_buf + wPointer, in_pkt.data, in_pkt.size);
        wPointer += in_pkt.size;
        av_packet_unref(&in_pkt);
    }

    encode(c_ctx, NULL, pkt, outfile);

end:
    if (pkt) av_packet_free(&pkt);
    if (frame) av_frame_free(&frame);
    if (c_ctx) avcodec_free_context(&c_ctx);
    if (swr_ctx) swr_free(&swr_ctx);
    if (fmt_ctx) avformat_close_input(&fmt_ctx);
    if (outfile) fclose(outfile);
    printf("Recording stopped.\n");
}

/* -------------------------------------------------------------------------- */

void set_status(int status)
{
    rec_status = status;
}

