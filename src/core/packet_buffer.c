#include "packet_buffer.h"

void packet_buffer_init(PacketBuffer *buf) {
    buf->capacity = MAX_PACKET_SIZE;
    buf->data = (unsigned char*)malloc(buf->capacity);
    buf->size = 0;
}

void packet_buffer_free(PacketBuffer *buf) {
    free(buf->data);
}

void packet_buffer_append(PacketBuffer *buf, unsigned char *data, int len) {
    if (buf->size + len > buf->capacity) {
        buf->capacity = (buf->size + len) * 2;
        buf->data = (unsigned char*)realloc(buf->data, buf->capacity);
    }
    memcpy(buf->data + buf->size, data, len);
    buf->size += len;
}

void packet_buffer_reset(PacketBuffer *buf) {
    buf->size = 0;
}
