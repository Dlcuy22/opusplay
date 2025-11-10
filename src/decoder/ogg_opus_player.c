#include "player.h"
#include "common.h"
#include "audio_callback.h"
#include "ogg_reader.h"

int play_ogg_opus(const char *filename) {
    FILE *fin = fopen(filename, "rb");
    if (!fin) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
        return 1;
    }

    OggPageHeader header;
    unsigned char *packets[256];
    int packet_sizes[256];
    int num_packets;

    // Read OpusHead
    if (read_ogg_page(fin, &header, packets, packet_sizes, &num_packets) <= 0) {
        fprintf(stderr, "Error: Failed to read OpusHead page\n");
        fclose(fin);
        return 1;
    }

    int sample_rate, channels;
    
    if (!parse_opus_head_ogg(packets[0], packet_sizes[0], &sample_rate, &channels)) {
        fprintf(stderr, "Error: Invalid OpusHead\n");
        for (int i = 0; i < num_packets; i++) free(packets[i]);
        fclose(fin);
        return 1;
    }
    for (int i = 0; i < num_packets; i++) free(packets[i]);

    // Skip OpusTags
    if (read_ogg_page(fin, &header, packets, packet_sizes, &num_packets) <= 0) {
        fprintf(stderr, "Error: Failed to read OpusTags page\n");
        fclose(fin);
        return 1;
    }
    for (int i = 0; i < num_packets; i++) free(packets[i]);

    int decode_sample_rate = 48000;
    
    printf("\n=== Playing Ogg Opus ===\n");
    printf("Channels: %d\n", channels);
    printf("Original Sample Rate: %d Hz\n", sample_rate);
    printf("Decode Sample Rate: %d Hz\n", decode_sample_rate);
    printf("\nPress Ctrl+C to stop\n\n");

    // Create decoder
    int err;
    OpusDecoder *decoder = opus_decoder_create(decode_sample_rate, channels, &err);
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
    audio_data.buffer_size = decode_sample_rate * channels * BUFFER_SIZE_SECONDS;
    audio_data.pcm_buffer = (short*)calloc(audio_data.buffer_size, sizeof(short));
    audio_data.read_position = 0;
    audio_data.write_position = 0;
    audio_data.channels = channels;
    audio_data.decoding_finished = 0;
    audio_data.playback_finished = 0;

    // Open audio stream
    PaStreamParameters outputParameters;
    outputParameters.device = Pa_GetDefaultOutputDevice();
    
    if (outputParameters.device == paNoDevice) {
        fprintf(stderr, "Error: No default output device.\n");
        free(audio_data.pcm_buffer);
        Pa_Terminate();
        opus_decoder_destroy(decoder);
        fclose(fin);
        return 1;
    }
    
    outputParameters.channelCount = channels;
    outputParameters.sampleFormat = paInt16;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    printf("Using audio device: %s\n", Pa_GetDeviceInfo(outputParameters.device)->name);

    PaStream *stream;
    err = Pa_OpenStream(&stream, NULL, &outputParameters, decode_sample_rate,
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
    err = Pa_StartStream(stream);
    if (err != paNoError) {
        fprintf(stderr, "PortAudio start error: %s\n", Pa_GetErrorText(err));
        Pa_CloseStream(stream);
        free(audio_data.pcm_buffer);
        Pa_Terminate();
        opus_decoder_destroy(decoder);
        fclose(fin);
        return 1;
    }
    
    printf("Audio stream started\n\n");

    short *pcm = (short*)malloc(FRAME_SIZE * channels * sizeof(short));
    int frame_count = 0;

    // Decode pages
    while (!stop_playback) {
        int result = read_ogg_page(fin, &header, packets, packet_sizes, &num_packets);
        if (result == 0) break;
        if (result < 0) {
            fprintf(stderr, "Error reading Ogg page\n");
            break;
        }

        for (int i = 0; i < num_packets; i++) {
            if (packet_sizes[i] == 0) {
                free(packets[i]);
                continue;
            }

            int num_samples = opus_decode(decoder, packets[i], packet_sizes[i], 
                                         pcm, FRAME_SIZE, 0);
            
            if (num_samples > 0) {
                // Wait if buffer is getting too full (keep 5 seconds max buffered)
                while ((audio_data.write_position - audio_data.read_position) > 
                       (decode_sample_rate * channels * 5) && !stop_playback) {
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
                    float decoded_duration = (float)audio_data.write_position / (decode_sample_rate * channels);
                    int buffered_samples = audio_data.write_position - audio_data.read_position;
                    float buffered_duration = (float)buffered_samples / (decode_sample_rate * channels);
                    printf("\rDecoded: %.2f sec | Buffered: %.2f sec | Playing...", 
                           decoded_duration, buffered_duration);
                    fflush(stdout);
                }
            } else if (num_samples < 0) {
                fprintf(stderr, "\nDecode error: %s\n", opus_strerror(num_samples));
            }

            free(packets[i]);
        }

        if (header.header_type & 0x04) break; // EOS
    }

    // Mark decoding as finished
    audio_data.decoding_finished = 1;
    printf("\n\nDecoding finished, waiting for playback to complete...\n");
    
    // Wait for playback to finish
    while (!audio_data.playback_finished && !stop_playback) {
        Pa_Sleep(100);
        int remaining = audio_data.write_position - audio_data.read_position;
        if (remaining > 0) {
            printf("\rRemaining: %.2f seconds", (float)remaining / (decode_sample_rate * channels));
            fflush(stdout);
        }
    }

    printf("\n✓ Playback finished\n");

    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
    free(pcm);
    free(audio_data.pcm_buffer);
    opus_decoder_destroy(decoder);
    fclose(fin);

    return 0;
}
