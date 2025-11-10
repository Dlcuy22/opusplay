#ifndef OGG_READER_H
#define OGG_READER_H

#include "common.h"

int read_ogg_page(FILE *fin, OggPageHeader *header, unsigned char **packets, 
                  int *packet_sizes, int *num_packets);
int parse_opus_head_ogg(unsigned char *packet, int size, int *sample_rate, int *channels);

#endif // OGG_READER_H
