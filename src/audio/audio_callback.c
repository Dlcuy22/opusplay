#include "audio_callback.h"

int audio_callback(const void *input, void *output,
                   unsigned long frameCount,
                   const PaStreamCallbackTimeInfo *timeInfo,
                   PaStreamCallbackFlags statusFlags,
                   void *userData) {
    AudioData *data = (AudioData*)userData;
    short *out = (short*)output;
    unsigned long i;
    
    (void)input;
    (void)timeInfo;
    (void)statusFlags;

    // Check for stop signal
    if (stop_playback) {
        return paComplete;
    }

    // Calculate available samples
    int available = data->write_position - data->read_position;
    
    // If no data available
    if (available <= 0) {
        // If decoding is finished and no data left, we're done
        if (data->decoding_finished) {
            data->playback_finished = 1;
            return paComplete;
        }
        // Otherwise output silence and wait for more data
        memset(out, 0, frameCount * data->channels * sizeof(short));
        return paContinue;
    }

    // Calculate how many samples we can actually play
    unsigned long samples_needed = frameCount * data->channels;
    unsigned long samples_to_play = (available < (int)samples_needed) ? (unsigned long)available : samples_needed;
    
    // Copy audio data to output (with circular buffer support)
    for (i = 0; i < samples_to_play; i++) {
        out[i] = data->pcm_buffer[(data->read_position + i) % data->buffer_size];
    }
    
    // Update read position
    data->read_position += samples_to_play;
    
    // Fill remaining with silence if needed
    if (samples_to_play < samples_needed) {
        memset(&out[samples_to_play], 0, (samples_needed - samples_to_play) * sizeof(short));
        
        // If we couldn't fill the buffer and decoding is done, we're finishing
        if (data->decoding_finished) {
            data->playback_finished = 1;
            return paComplete;
        }
    }

    return paContinue;
}
