#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <opus/opus.h>
#include <portaudio.h>

// Constants
#define FRAME_SIZE 5760
#define MAX_PACKET_SIZE 4000
#define SAMPLE_RATE 48000
#define BUFFER_SIZE_SECONDS 30

// Global flag for stop playback
extern volatile int stop_playback;

// Audio stream data structure
typedef struct {
    short *pcm_buffer;
    int buffer_size;
    volatile int read_position;
    volatile int write_position;
    int channels;
    int bitrate;
    volatile int decoding_finished;
    volatile int playback_finished;
} AudioData;

// Opus header structure (custom format)
typedef struct {
    unsigned char magic[8];
    unsigned char version;
    unsigned char channel_count;
    unsigned short pre_skip;
    unsigned int sample_rate;
    unsigned short gain;
    unsigned char channel_mapping;
} OpusHeader;

// Ogg page header structure
typedef struct {
    char capture_pattern[4];
    unsigned char version;
    unsigned char header_type;
    unsigned long long granule_position;
    unsigned int serial_number;
    unsigned int page_sequence;
    unsigned int checksum;
    unsigned char page_segments;
} __attribute__((packed)) OggPageHeader;

// Packet buffer structure
typedef struct {
    unsigned char *data;
    int size;
    int capacity;
} PacketBuffer;

#endif // COMMON_H
