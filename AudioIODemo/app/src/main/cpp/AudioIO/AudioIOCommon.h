//
// Created by zhangyu on 2017/9/30.
//

#ifndef AUDIOIODEMO_AUDIOIOCOMMON_H
#define AUDIOIODEMO_AUDIOIOCOMMON_H

#include <android/log.h>
#include <malloc.h>
#include <string.h>
#include <time.h>

#define AUDIOENGINENAMESPACE AUDIOIO

#define LONG_MILLISECOND unsigned long long

//log flag, user can define or not
#define NDK_DEBUG


#ifdef NDK_DEBUG
#define LOG_TAG "AUDIOIOENGINE"
#define  LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#else
#define  LOGE(...)
#endif

typedef struct audio_buffer_ {
    //
    int numberOfSamples;

    int numberOfChannel;

    //if numberOfChannel is 2, the data is interact
    short *data;
} audio_buffer;

typedef void (*IOStreamCallback)(const audio_buffer *buffer, void *ref);


//init audio_buffer
__inline void alloc_audio_buffer(audio_buffer **buffer, int sampleCount, int channel) {
    *buffer = (audio_buffer *)malloc(sizeof(audio_buffer));
    (*buffer)->numberOfSamples = sampleCount;
    (*buffer)->numberOfChannel = channel;
    (*buffer)->data = (short *)malloc(sizeof(short) * channel * sampleCount);
    memset((*buffer)->data, 0, sizeof(short) * channel * sampleCount);
}

//free audio_buffer
__inline void release_audio_buffer(audio_buffer * &buffer) {
    if (buffer != nullptr) {
        if (buffer->data != nullptr) {
            free(buffer->data);
            buffer->data = nullptr;
        }
        free(buffer);
        buffer = nullptr;
    }
}

static LONG_MILLISECOND getCurrentSystemTime()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return (LONG_MILLISECOND)(tv.tv_sec) * 1000 +
           (LONG_MILLISECOND)(tv.tv_usec) / 1000;
}
#endif //AUDIOIODEMO_AUDIOIOCOMMON_H
