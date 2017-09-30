//
// Created by zhangyu on 2017/9/30.
//

#include "AudioOutput.h"
#include "unistd.h"

static void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
    AUDIOIO::AudioOutput *player = (AUDIOIO::AudioOutput *) context;
    player->process();
}

AUDIOIO::AudioOutput::AudioOutput(AUDIOIO::AudioOpenSLEngine *engine, int sampleCount,
                                  int sampleRate, int channel) : AudioIOStream(engine, sampleCount,
                                                                               sampleRate,
                                                                               channel) {
    bqPlayerObject = NULL;
    bqPlayerPlay = NULL;
    bqPlayerBufferQueue = NULL;

    m_needFillBuffer = false;

    prepare();
}

AUDIOIO::AudioOutput::~AudioOutput() {
    stop();

    if (bqPlayerObject != NULL) {
        //stop
        SLuint32 soundPlayerState;
        SLuint32 recordState;
        (*bqPlayerObject)->GetState(bqPlayerObject, &soundPlayerState);
        if (soundPlayerState == SL_OBJECT_STATE_REALIZED) {
            SLuint32 state = SL_PLAYSTATE_PLAYING;
            (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_STOPPED);
            while(state != SL_PLAYSTATE_STOPPED) {
                usleep(10000);
                (*bqPlayerPlay)->GetPlayState(bqPlayerPlay, &state);
            }
            (*bqPlayerBufferQueue)->Clear(bqPlayerBufferQueue);
            (*bqPlayerObject)->AbortAsyncOperation(bqPlayerObject);
            (*bqPlayerObject)->Destroy(bqPlayerObject);
        }
    }
    m_engine = NULL;
    bqPlayerObject      = NULL;
    bqPlayerPlay        = NULL;
    bqPlayerBufferQueue = NULL;
}

void AUDIOIO::AudioOutput::start() {
    AudioIOStream::start();
    if (bqPlayerPlay && enginePlayingState() != SL_PLAYSTATE_PLAYING) {
        // set the player's state to playing
        SLresult result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
        if (result != SL_RESULT_SUCCESS) {
            LOGE("PlayerRender::play() failed.SetPlayState SL_PLAYSTATE_PLAYING error(%d)", result);
            return;
        }
        m_needFillBuffer = true;

        //填充数据
        process();
    }
}

void AUDIOIO::AudioOutput::stop() {
    AudioIOStream::stop();
    if (bqPlayerPlay && enginePlayingState() != SL_PLAYSTATE_STOPPED) {
        SLresult result;
        // set the player's state to playing
        result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_STOPPED);
        if (result != SL_RESULT_SUCCESS) {
            LOGE("PlayerRender::stop() failed.SetPlayState SL_PLAYSTATE_PAUSED error(%d)", result);
            return;
        }
        m_needFillBuffer = false;
        //回调停止
    }
}

void AUDIOIO::AudioOutput::pause() {
    AudioIOStream::pause();
    if (bqPlayerPlay && enginePlayingState() != SL_PLAYSTATE_PAUSED) {
        SLresult result;
        // set the player's state to playing
        result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PAUSED);
        if (result != SL_RESULT_SUCCESS) {
            LOGE("PlayerRender::pause() failed.SetPlayState SL_PLAYSTATE_PAUSED error(%d)", result);
            return;
        }
        m_needFillBuffer = false;
    }
}

void AUDIOIO::AudioOutput::process() {
    if (!m_needFillBuffer) {
        return;
    }
    //callback
    int enqueueSize = m_buffer->numberOfSamples * m_buffer->numberOfChannel * sizeof(short);
    memset(m_buffer->data, 0 ,enqueueSize);
    if (m_callback != nullptr) {
        m_callback(m_buffer, m_callbackRef);
    }

    if (!m_needFillBuffer || bqPlayerBufferQueue == NULL) {
        return;
    }
    //Enqueue buffer
    SLresult result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, m_buffer->data, enqueueSize);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("bufferQueueItf Enqueue error; result is : %d", result);
        stop();
        return;
    }
}

bool AUDIOIO::AudioOutput::isEnginePlaying() {
    return (enginePlayingState() == SL_PLAYSTATE_PLAYING);
}

SLuint32 AUDIOIO::AudioOutput::enginePlayingState() {
    if (bqPlayerPlay) {
        SLuint32 state;
        (*bqPlayerPlay)->GetPlayState(bqPlayerPlay, &state);
        return state;
    }
    return SL_PLAYSTATE_STOPPED;
}

void AUDIOIO::AudioOutput::prepare() {
    AudioIOStream::prepare();
    SLresult result;

    SLmilliHertz bqPlayerSampleRate = 0;
    if (m_sampleRate >= 0) {
        bqPlayerSampleRate = (SLmilliHertz) (m_sampleRate * 1000);
    }

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq   = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM                       format_pcm = {SL_DATAFORMAT_PCM, (SLuint32)m_channel, (SLuint32)bqPlayerSampleRate,
                                                         SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                                         m_channel == 2 ? (SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT) : SL_SPEAKER_BACK_CENTER,
                                                         SL_BYTEORDER_LITTLEENDIAN};

    if (bqPlayerSampleRate) {
        format_pcm.samplesPerSec = bqPlayerSampleRate;       //sample rate in mili second
    }
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, m_engine->getOuputMixObj()};
    SLDataSink              audioSnk   = {&loc_outmix, NULL};


    const SLInterfaceID ids[2]       = {SL_IID_PLAY, SL_IID_BUFFERQUEUE};
    const SLboolean     req[2]       = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    SLEngineItf         engineEngine = m_engine->getEngine();
    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &bqPlayerObject, &audioSrc, &audioSnk,
                                                2, ids, req);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("PlayerRender::_createPlayer() failed.CreateAudioPlayer error(%d)", result);
        return;
    }

    // realize the player
    result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("PlayerRender::_createPlayer() failed.Realize error(%d)", result);
        return;
    }

    // get the play interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("PlayerRender::_createPlayer() failed.GetInterface SL_IID_PLAY error(%d)", result);
        return;
    }

    // get the buffer queue interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,
                                             &bqPlayerBufferQueue);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("PlayerRender::_createPlayer() failed.GetInterface SL_IID_BUFFERQUEUE error(%d)", result);
        return;
    }

    // register callback on the buffer queue
    result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, this);

    if (result != SL_RESULT_SUCCESS) {
        LOGE("PlayerRender::_createPlayer() failed.RegisterCallback SL_IID_BUFFERQUEUE error(%d)", result);
        return;
    }
}
