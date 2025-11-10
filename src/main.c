#include "common.h"
#include "signal_handler.h"
#include "format_detector.h"
#include "player.h"

// Global flag definition
volatile int stop_playback = 0;

void print_usage(const char *prog_name) {
    printf("opusplay - Opus Audio Player\n");
    printf("=============================\n\n");
    printf("Usage:\n");
    printf("  %s <audio.opus>\n\n", prog_name);
    printf("Examples:\n");
    printf("  %s music.opus\n", prog_name);
    printf("  %s recording.opus\n\n", prog_name);
    printf("Supported formats:\n");
    printf("  ✓ Ogg Opus (universal format)\n");
    printf("  ✓ Custom Raw Opus (from eopus)\n\n");
    printf("Controls:\n");
    printf("  Ctrl+C : Stop playback\n");
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        print_usage(argv[0]);
        return 1;
    }

    const char *filename = argv[1];

    // Setup signal handler
    signal(SIGINT, signal_handler);

    // Detect format
    int is_ogg;
    if (!detect_format(filename, &is_ogg)) {
        fprintf(stderr, "Error: Unable to detect file format or file not found\n");
        return 1;
    }

    // Play based on format
    if (is_ogg) {
        return play_ogg_opus(filename);
    } else {
        return play_custom_opus(filename);
    }
}
