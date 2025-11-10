#ifndef PACKET_BUFFER_H
#define PACKET_BUFFER_H

#include "common.h"

void packet_buffer_init(PacketBuffer *buf);
void packet_buffer_free(PacketBuffer *buf);
void packet_buffer_append(PacketBuffer *buf, unsigned char *data, int len);
void packet_buffer_reset(PacketBuffer *buf);

#endif // PACKET_BUFFER_H
