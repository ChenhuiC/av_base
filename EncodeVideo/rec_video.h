/**
* @file: rec_video.h
* @brief: 
* @author: chenhui
* @created: 2023-11-17 22:09:45
* 
* @copyright (C), 2008-2023, 浙江德施曼科技智能股份有限公司
* 
*/
#ifndef __REC_VIDEO_H__
#define __REC_VIDEO_H__

#ifdef __cplusplus
extern "C"
{
#endif

/* includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "libavutil/avutil.h"
#include "libavdevice/avdevice.h"
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"

/* export marcos -------------------------------------------------------------*/
#define REC_START 1
#define REC_STOP  0
/* export types --------------------------------------------------------------*/
/* export variables ----------------------------------------------------------*/
/* export functions ----------------------------------------------------------*/
void set_status(int status);

void rec_video(void);
#ifdef __cplusplus
}
#endif

#endif
