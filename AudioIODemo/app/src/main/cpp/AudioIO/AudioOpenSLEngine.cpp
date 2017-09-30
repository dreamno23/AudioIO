//
// Created by zhangyu on 2017/9/30.
//

#include "AudioOpenSLEngine.h"

void AUDIOIO::AudioOpenSLEngine::_createOpenslEngine() {
    SLresult result;

    // create engine
    result = slCreateEngine(&m_engineObj, 0, NULL, 0, NULL, NULL);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("%s: slCreateEngine failed(error:%d)",__FUNCTION__, result);
        return;
    }

    // realize the engine
    result = (*m_engineObj)->Realize(m_engineObj, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("%s: Realize engine failed(error:%d)",__FUNCTION__, result);
        return;
    }

    // get the engine interface, which is needed in order to create other objects
    result = (*m_engineObj)->GetInterface(m_engineObj, SL_IID_ENGINE, &m_engine);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("%s: GetInterface  SL_IID_ENGINE failed(error:%d)",__FUNCTION__, result);
        return;
    }

    // create output mix, with environmental reverb specified as a non-required interface
    const SLInterfaceID ids[2] = {SL_IID_EQUALIZER,SL_IID_BASSBOOST};
    const SLboolean req[2] = {SL_BOOLEAN_TRUE,SL_BOOLEAN_TRUE};
    result = (*m_engine)->CreateOutputMix(m_engine, &m_outputMixObj, 2, ids, req);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("%s: CreateOutputMix  failed(error:%d)",__FUNCTION__, result);
        return;
    }

    // realize the output mix
    result = (*m_outputMixObj)->Realize(m_outputMixObj, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("%s:Realize m_output_itf  failed(error:%d)",__FUNCTION__, result);
        return;
    }
}

AUDIOIO::AudioOpenSLEngine::AudioOpenSLEngine() {
    _createOpenslEngine();
}

AUDIOIO::AudioOpenSLEngine::~AudioOpenSLEngine() {
    if (m_outputMixObj != NULL) {
        (*m_outputMixObj)->Destroy(m_outputMixObj);
        m_outputMixObj = NULL;
    }
    // destroy engine object, and invalidate all associated interfaces
    if (m_engineObj != NULL) {
        (*m_engineObj)->Destroy(m_engineObj);
        m_engineObj = NULL;
        m_engine = NULL;
    }
}

SLEngineItf AUDIOIO::AudioOpenSLEngine::getEngine() {
    return m_engine;
}

SLObjectItf AUDIOIO::AudioOpenSLEngine::getOuputMixObj() {
    return m_outputMixObj;
}

