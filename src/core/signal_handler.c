#include "signal_handler.h"
#include "common.h"
#include <stdio.h>

void signal_handler(int sig) {
    (void)sig;
    printf("\n\nStopping playback...\n");
    stop_playback = 1;
}
