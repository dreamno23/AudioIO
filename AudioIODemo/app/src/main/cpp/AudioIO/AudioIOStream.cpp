//
// Created by zhangyu on 2017/9/30.
//

#include "AudioIOStream.h"

AUDIOIO::AudioIOStream::AudioIOStream(AUDIOIO::AudioOpenSLEngine *engine, int numberOfSamples,
                                      int sampleRate, int numberOfChannel) {
    m_sampleCount = numberOfSamples;
    m_channel = numberOfChannel;
    m_sampleRate = sampleRate;
    alloc_audio_buffer(&m_buffer,numberOfSamples, numberOfChannel);

    m_slengine = engine;
}

AUDIOIO::AudioIOStream::~AudioIOStream() {
    m_callback = nullptr;
    m_callbackRef = nullptr;

    audio_buffer *&m_bufferRef = m_buffer;
    release_audio_buffer(m_bufferRef);
    m_buffer = nullptr;

    m_slengine = nullptr;
}

void AUDIOIO::AudioIOStream::start() {

}

void AUDIOIO::AudioIOStream::stop() {

}

void AUDIOIO::AudioIOStream::pause() {

}

void AUDIOIO::AudioIOStream::prepare() {

}

void AUDIOIO::AudioIOStream::setStreamCallback(IOStreamCallback callback, void *callbackRef) {
    m_callback = callback;
    m_callbackRef = callbackRef;
}
