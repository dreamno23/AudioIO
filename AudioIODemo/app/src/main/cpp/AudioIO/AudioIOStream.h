//
// Created by zhangyu on 2017/9/30.
//

#ifndef AUDIOIODEMO_AUDIOIOSTREAM_H
#define AUDIOIODEMO_AUDIOIOSTREAM_H

#include "AudioOpenSLEngine.h"

namespace AUDIOENGINENAMESPACE {
    class AudioIOStream {
    public:
        AudioIOStream(AudioOpenSLEngine *engine,int numberOfSamples, int sampleRate, int numberOfChannel);
        virtual ~AudioIOStream();

        void setStreamCallback(IOStreamCallback callback, void *callbackRef);


        virtual void start();

        virtual void stop();

        virtual void pause();

    protected:
        audio_buffer *m_buffer = nullptr;

        IOStreamCallback m_callback = NULL;
        void *m_callbackRef = nullptr;

        int m_sampleRate;
        int m_channel;
        int m_sampleCount;

        void prepare();

        AudioOpenSLEngine *m_slengine = nullptr;
    };
}

#endif //AUDIOIODEMO_AUDIOIOSTREAM_H
