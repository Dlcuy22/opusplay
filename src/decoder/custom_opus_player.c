#include "player.h"
#include "common.h"
#include "audio_callback.h"

int play_custom_opus(const char *filename) {
    FILE *fin = fopen(filename, "rb");
    if (!fin) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
        return 1;
    }

    // Read header
    OpusHeader header;
    if (fread(&header, sizeof(OpusHeader), 1, fin) != 1) {
        fprintf(stderr, "Error: Failed to read Opus header\n");
        fclose(fin);
        return 1;
    }

    if (memcmp(header.magic, "OpusHead", 8) != 0) {
        fprintf(stderr, "Error: Invalid Opus header\n");
        fclose(fin);
        return 1;
    }

    int channels = header.channel_count;
    int sample_rate = header.sample_rate;

    printf("\n=== Playing Custom Opus ===\n");
    printf("Channels: %d\n", channels);
    printf("Sample Rate: %d Hz\n", sample_rate);
    printf("\nPress Ctrl+C to stop\n\n");

    // Create decoder
    int err;
    OpusDecoder *decoder = opus_decoder_create(sample_rate, channels, &err);
    if (err != OPUS_OK) {
        fprintf(stderr, "Error: Failed to create decoder: %s\n", opus_strerror(err));
        fclose(fin);
        return 1;
    }

    // Initialize PortAudio
    err = Pa_Initialize();
    if (err != paNoError) {
        fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(err));
        opus_decoder_destroy(decoder);
        fclose(fin);
        return 1;
    }

    // Setup audio data
    AudioData audio_data;
    audio_data.buffer_size = sample_rate * channels * BUFFER_SIZE_SECONDS;
    audio_data.pcm_buffer = (short*)calloc(audio_data.buffer_size, sizeof(short));
    audio_data.read_position = 0;
    audio_data.write_position = 0;
    audio_data.channels = channels;
    audio_data.decoding_finished = 0;
    audio_data.playback_finished = 0;

    // Open audio stream
    PaStreamParameters outputParameters;
    outputParameters.device = Pa_GetDefaultOutputDevice();
    outputParameters.channelCount = channels;
    outputParameters.sampleFormat = paInt16;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    PaStream *stream;
    err = Pa_OpenStream(&stream, NULL, &outputParameters, sample_rate,
                        256, paClipOff, audio_callback, &audio_data);
    
    if (err != paNoError) {
        fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(err));
        free(audio_data.pcm_buffer);
        Pa_Terminate();
        opus_decoder_destroy(decoder);
        fclose(fin);
        return 1;
    }

    // Start stream
    Pa_StartStream(stream);
    printf("Audio stream started\n");

    // Decode and buffer
    unsigned char *opus_data = (unsigned char*)malloc(MAX_PACKET_SIZE);
    short *pcm = (short*)malloc(FRAME_SIZE * channels * sizeof(short));
    int frame_count = 0;

    while (!stop_playback) {
        unsigned int packet_size;
        if (fread(&packet_size, sizeof(unsigned int), 1, fin) != 1) {
            break;
        }

        if (packet_size > MAX_PACKET_SIZE) break;

        if (fread(opus_data, 1, packet_size, fin) != packet_size) break;

        int num_samples = opus_decode(decoder, opus_data, packet_size, pcm, FRAME_SIZE, 0);
        if (num_samples < 0) {
            fprintf(stderr, "Decode error: %s\n", opus_strerror(num_samples));
            break;
        }

        // Wait if buffer is getting full
        while ((audio_data.write_position - audio_data.read_position) > 
               (sample_rate * channels * 5) && !stop_playback) {
            Pa_Sleep(10);
        }

        // Copy to buffer with circular buffering
        int samples_to_write = num_samples * channels;
        for (int j = 0; j < samples_to_write; j++) {
            audio_data.pcm_buffer[(audio_data.write_position + j) % audio_data.buffer_size] = pcm[j];
        }
        audio_data.write_position += samples_to_write;

        frame_count++;
        
        if (frame_count % 50 == 0) {
            float duration = (float)audio_data.write_position / (sample_rate * channels);
            int buffered = audio_data.write_position - audio_data.read_position;
            printf("\rDecoded: %.2f sec | Buffered: %.2f sec", 
                   duration, (float)buffered / (sample_rate * channels));
            fflush(stdout);
        }
    }

    // Mark decoding as finished
    audio_data.decoding_finished = 1;
    printf("\n\nDecoding finished, waiting for playback...\n");

    // Wait for playback to finish
    while (!audio_data.playback_finished && !stop_playback) {
        Pa_Sleep(100);
        int remaining = audio_data.write_position - audio_data.read_position;
        printf("\rRemaining: %.2f seconds", (float)remaining / (sample_rate * channels));
        fflush(stdout);
    }

    printf("\n✓ Playback finished\n");

    // Cleanup
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
    free(opus_data);
    free(pcm);
    free(audio_data.pcm_buffer);
    opus_decoder_destroy(decoder);
    fclose(fin);

    return 0;
}
