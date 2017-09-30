//
// Created by zhangyu on 2017/9/30.
//

#include "AudioInput.h"
#include <unistd.h>

static void bqRecorderCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
    AUDIOIO::AudioInput *recorder = (AUDIOIO::AudioInput *) context;
    recorder->process();
}

AUDIOIO::AudioInput::AudioInput(AUDIOIO::AudioOpenSLEngine *engine, int sampleCount, int sampleRate,
                                int channel) : AudioIOStream(engine, sampleCount, sampleRate,
                                                             channel) {
    prepare();
}

AUDIOIO::AudioInput::~AudioInput() {
    if (recorderObject != NULL) {
        //stop
        SLuint32 soundPlayerState;
        (*recorderObject)->GetState(recorderObject, &soundPlayerState);
        if (soundPlayerState == SL_OBJECT_STATE_REALIZED) {
            SLuint32 state = SL_RECORDSTATE_RECORDING;
            (*recorderRecord)->SetRecordState(recorderRecord, SL_RECORDSTATE_STOPPED);
            while(state != SL_RECORDSTATE_STOPPED) {
                usleep(10000);
                (*recorderRecord)->GetRecordState(recorderRecord, &state);
            }

            (*recorderBufferQueue)->Clear(recorderBufferQueue);
            (*recorderObject)->AbortAsyncOperation(recorderObject);
            (*recorderObject)->Destroy(recorderObject);
        }
    }
    m_engine = NULL;
    recorderObject      = NULL;
    recorderRecord      = NULL;
    recorderBufferQueue = NULL;
}

void AUDIOIO::AudioInput::start() {
    AudioIOStream::start();
    if (m_isStart) {
        return;
    }
    //reset
    m_isStart = true;
    m_startRecordTimeMS = 0;
    m_recorderLanetcyTimeMS = 0;

    memset(m_buffer->data, 0 , m_sampleCount * sizeof(short) * m_channel);

    m_startRecordTimeMS = getCurrentSystemTime();
    m_totalRecordSize = 0;

    SLresult result;
    if (recorderRecord) {
        // start recording
        result    = (*recorderRecord)->SetRecordState(recorderRecord, SL_RECORDSTATE_RECORDING);
        if (result != SL_RESULT_SUCCESS) {
            LOGE("opensl SetRecordState SL_RECORDSTATE_RECORDING failed (error:%d)", result);
        }

        //send buffer
        result = (*recorderBufferQueue)->Enqueue(recorderBufferQueue, m_buffer->data,
                                                 m_sampleCount * sizeof(short) * m_channel);
        if (SL_RESULT_SUCCESS != result) {
            LOGE("%s（%d） : recorderBufferQueue Enqueue error .result is : %d", __FUNCTION__, __LINE__, result);
            return ;
        }
    }
}

void AUDIOIO::AudioInput::stop() {
    AudioIOStream::stop();
    if (!m_isStart) {
        return;
    }
    m_isStart = false;

    SLresult result;
    if (recorderRecord) {
        if (isRecording()) {
            result = (*recorderRecord)->SetRecordState(recorderRecord, SL_RECORDSTATE_STOPPED);
            if (result != SL_RESULT_SUCCESS) {
                LOGE("McAudioRecorder::stop() SetRecordState SL_RECORDSTATE_STOPPED failed(error:%d) ", result);
            }
        }
    }
}

void AUDIOIO::AudioInput::pause() {
    AudioIOStream::pause();
    SLresult result;
    if (recorderRecord) {
        result = (*recorderRecord)->SetRecordState(recorderRecord, SL_RECORDSTATE_PAUSED);
    }
}

void AUDIOIO::AudioInput::process() {
    if (m_recorderLanetcyTimeMS == 0) { //首次进入，计算延迟
        m_recorderLanetcyTimeMS = (int) (getCurrentSystemTime() - m_startRecordTimeMS);
    }
    if (!m_isStart) {
        return;
    }
    //voice data callback
    m_totalRecordSize ++;

    //callback
    int bufferSize = m_sampleCount * m_channel * sizeof(short);

    if (m_callback != nullptr) {
        m_callback(m_buffer, m_callbackRef);
    }

    //回调可能改变状态
    if (!m_isStart)
        return;

    //Enqueue buffer
    if (isRecording()) {
        // enqueue an empty buffer to be filled by the recorder
        // (for streaming recording, we would enqueue at least 2 empty buffers to start things off)
        SLresult result;
        result = (*recorderBufferQueue)->Enqueue(recorderBufferQueue, m_buffer->data, bufferSize);
        if (result != SL_RESULT_SUCCESS) {
            LOGE("mMcAudioRcorder Enqueue error (%d)", result);
        }
    }
}

int AUDIOIO::AudioInput::getRecordLatency() {
    return m_recorderLanetcyTimeMS;
}

bool AUDIOIO::AudioInput::isRecording() {
    SLresult result;
    if (recorderRecord) {
        SLuint32 isRecording;
        result = (*recorderRecord)->GetRecordState(recorderRecord, &isRecording);
        if (SL_RESULT_SUCCESS != result) {
            LOGE("opensl GetRecordState failed (error:%d)", result);
            return false;
        }
        (void) result;
        if (isRecording == SL_RECORDSTATE_RECORDING) {
            return true;
        }
    }
    return false;
}

void AUDIOIO::AudioInput::prepare() {
    AudioIOStream::prepare();
    SLresult result;

    // configure audio source
    SLDataLocator_IODevice loc_dev  = {SL_DATALOCATOR_IODEVICE, SL_IODEVICE_AUDIOINPUT,
                                       SL_DEFAULTDEVICEID_AUDIOINPUT, NULL};
    SLDataSource           audioSrc = {&loc_dev, NULL};

    // configure audio sink
    SLDataLocator_AndroidSimpleBufferQueue loc_bq     = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 1};
    SLDataFormat_PCM                       format_pcm = {SL_DATAFORMAT_PCM, (SLuint32)m_channel, (SLuint32)m_sampleRate * 1000,
                                                         SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                                         m_channel == 2 ? (SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT) : SL_SPEAKER_FRONT_CENTER,
                                                         SL_BYTEORDER_LITTLEENDIAN};

    SLDataSink                             audioSnk   = {&loc_bq, &format_pcm};

    // create audio recorder
    // (requires the RECORD_AUDIO permission)
    const SLInterfaceID id[1]  = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
    const SLboolean     req[1] = {SL_BOOLEAN_TRUE};
    result = (*m_engine->getEngine())->CreateAudioRecorder(m_engine->getEngine(), &recorderObject, &audioSrc,
                                                           &audioSnk, 1, id, req);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("AudioInput CreateAudioRecorder failed (error:%d)", result);
        return;
    }

    // realize the audio recorder
    result = (*recorderObject)->Realize(recorderObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("AudioInput Realize failed (error:%d)", result);
        return;
    }

    // get the record interface
    result = (*recorderObject)->GetInterface(recorderObject, SL_IID_RECORD, &recorderRecord);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("AudioInput GetInterface SL_IID_RECORD failed (error:%d)", result);
        return;
    }

    // get the buffer queue interface
    result = (*recorderObject)->GetInterface(recorderObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                                             &recorderBufferQueue);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("AudioInput GetInterface SL_IID_ANDROIDSIMPLEBUFFERQUEUE failed (error:%d)", result);
        return;
    }

    // register callback on the buffer queue
    result = (*recorderBufferQueue)->RegisterCallback(recorderBufferQueue, bqRecorderCallback, this);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("AudioInput RegisterCallback failed (error:%d)", result);
        return;
    }
}
