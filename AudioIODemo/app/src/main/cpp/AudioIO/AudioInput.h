//
// Created by zhangyu on 2017/9/30.
//

#ifndef AUDIOIODEMO_AUDIOINPUT_H
#define AUDIOIODEMO_AUDIOINPUT_H

#include "AudioIOStream.h"

namespace AUDIOENGINENAMESPACE {
    class AudioInput : public AudioIOStream {
    public:
        AudioInput(AudioOpenSLEngine *engine, int sampleCount = 1024, int sampleRate = 44100, int channel = 2);

        ~AudioInput();
//

        void start();

        void stop();

        void pause();

        //called by internal callback
        void process();

        int getRecordLatency();

    private:

        //internal is recording
        bool isRecording();

        //runing flag
        bool m_isStart = false;

        //
        unsigned long long m_startRecordTimeMS = 0;

        //record latenetcy in ms
        int m_recorderLanetcyTimeMS = 0;

        unsigned int m_totalRecordSize = 0;

        AudioOpenSLEngine *m_engine = NULL;
        SLObjectItf                   recorderObject = NULL;
        SLRecordItf                   recorderRecord = NULL;
        SLAndroidSimpleBufferQueueItf recorderBufferQueue = NULL;
        void prepare();
    };
}



#endif //AUDIOIODEMO_AUDIOINPUT_H
