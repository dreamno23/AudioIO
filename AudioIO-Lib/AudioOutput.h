//
// Created by zhangyu on 2017/9/30.
//

#ifndef AUDIOIODEMO_AUDIOOUTPUT_H
#define AUDIOIODEMO_AUDIOOUTPUT_H

#include "AudioIOStream.h"

namespace AUDIOENGINENAMESPACE {
    class AudioOutput : public AudioIOStream {
    public:
        AudioOutput(AudioOpenSLEngine *engine, int sampleCount = 1024, int sampleRate = 44100, int channel = 2);
        ~AudioOutput();

        void start();

        void stop();

        void pause();

//called by internal callback
        void process();
    private:

        AudioOpenSLEngine  *m_engine;
        SLObjectItf bqPlayerObject = NULL;
        SLPlayItf  bqPlayerPlay = NULL;
        SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue = NULL;

        bool m_needFillBuffer = false;

        bool isEnginePlaying();
        SLuint32 enginePlayingState();
        void prepare();
    };
}



#endif //AUDIOIODEMO_AUDIOOUTPUT_H
