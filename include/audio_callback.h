#ifndef AUDIO_CALLBACK_H
#define AUDIO_CALLBACK_H

#include "common.h"

int audio_callback(const void *input, void *output,
                   unsigned long frameCount,
                   const PaStreamCallbackTimeInfo *timeInfo,
                   PaStreamCallbackFlags statusFlags,
                   void *userData);

#endif // AUDIO_CALLBACK_H
