#include "ogg_reader.h"
#include "packet_buffer.h"

int read_ogg_page(FILE *fin, OggPageHeader *header, unsigned char **packets, 
                  int *packet_sizes, int *num_packets) {
    if (fread(header, 27, 1, fin) != 1) {
        return 0;
    }

    if (memcmp(header->capture_pattern, "OggS", 4) != 0) {
        return -1;
    }

    unsigned char segments[255];
    if (fread(segments, 1, header->page_segments, fin) != header->page_segments) {
        return -1;
    }

    int payload_size = 0;
    for (int i = 0; i < header->page_segments; i++) {
        payload_size += segments[i];
    }

    unsigned char *payload = (unsigned char*)malloc(payload_size);
    if (fread(payload, 1, payload_size, fin) != (size_t)payload_size) {
        free(payload);
        return -1;
    }

    *num_packets = 0;
    int pos = 0;
    PacketBuffer current_packet;
    packet_buffer_init(&current_packet);

    for (int i = 0; i < header->page_segments; i++) {
        packet_buffer_append(&current_packet, payload + pos, segments[i]);
        pos += segments[i];

        if (segments[i] < 255) {
            packets[*num_packets] = (unsigned char*)malloc(current_packet.size);
            memcpy(packets[*num_packets], current_packet.data, current_packet.size);
            packet_sizes[*num_packets] = current_packet.size;
            (*num_packets)++;
            packet_buffer_reset(&current_packet);
        }
    }

    packet_buffer_free(&current_packet);
    free(payload);
    return 1;
}

int parse_opus_head_ogg(unsigned char *packet, int size, int *sample_rate, int *channels) {
    if (size < 19 || memcmp(packet, "OpusHead", 8) != 0) {
        return 0;
    }
    *channels = packet[9];
    *sample_rate = packet[12] | (packet[13] << 8) | (packet[14] << 16) | (packet[15] << 24);
    return 1;
}
