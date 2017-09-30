//
// Created by zhangyu on 2017/9/30.
//

#ifndef AUDIOIODEMO_AUDIOOPENSLENGINE_H
#define AUDIOIODEMO_AUDIOOPENSLENGINE_H

#include "AudioIOCommon.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

namespace AUDIOENGINENAMESPACE {
    class AudioOpenSLEngine {
    public:
        AudioOpenSLEngine();
        ~AudioOpenSLEngine();

        SLEngineItf getEngine();
        SLObjectItf getOuputMixObj();
    private:
        void _createOpenslEngine();

        SLEngineItf m_engine = NULL;

        SLObjectItf m_engineObj= NULL;
        SLObjectItf m_outputMixObj = NULL;
    };
}


#endif //AUDIOIODEMO_AUDIOOPENSLENGINE_H
