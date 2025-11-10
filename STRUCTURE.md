# Project Structure Documentation

## Overview
This document explains the modular organization of the OpusPlay project.

## Directory Layout

```
opusplay/
├── include/              # Public header files
├── src/                 # Source code
│   ├── core/           # Core utilities
│   ├── decoder/        # Decoding logic
│   ├── audio/          # Audio processing
│   └── main.c          # Entry point
├── obj/                # Compiled object files (generated)
├── songs/              # Test audio files
├── .vscode/            # IDE configuration
├── Makefile            # Build system
└── README.md           # User documentation
```

## Module Breakdown

### Header Files (`include/`)

| File | Purpose |
|------|---------|
| `common.h` | Shared types, constants, and includes |
| `packet_buffer.h` | Packet buffer API |
| `audio_callback.h` | PortAudio callback declaration |
| `ogg_reader.h` | Ogg file parsing API |
| `format_detector.h` | File format detection |
| `player.h` | Player function declarations |
| `signal_handler.h` | Signal handling API |

### Core Module (`src/core/`)

**Purpose**: Fundamental utilities used across the application

| File | Functions | Description |
|------|-----------|-------------|
| `packet_buffer.c` | `packet_buffer_init()`<br>`packet_buffer_free()`<br>`packet_buffer_append()`<br>`packet_buffer_reset()` | Dynamic buffer for Ogg packet assembly |
| `signal_handler.c` | `signal_handler()` | Handles Ctrl+C for graceful shutdown |
| `format_detector.c` | `detect_format()` | Detects Ogg Opus vs Custom format |

### Decoder Module (`src/decoder/`)

**Purpose**: Audio decoding and playback logic

| File | Functions | Description |
|------|-----------|-------------|
| `ogg_reader.c` | `read_ogg_page()`<br>`parse_opus_head_ogg()` | Parses Ogg container format |
| `custom_opus_player.c` | `play_custom_opus()` | Plays custom raw Opus files |
| `ogg_opus_player.c` | `play_ogg_opus()` | Plays standard Ogg Opus files |

### Audio Module (`src/audio/`)

**Purpose**: Real-time audio processing

| File | Functions | Description |
|------|-----------|-------------|
| `audio_callback.c` | `audio_callback()` | PortAudio callback for playback |

### Main (`src/main.c`)

**Purpose**: Application entry point and orchestration

- Parses command-line arguments
- Sets up signal handlers
- Detects file format
- Dispatches to appropriate player

## Data Flow

```
1. main.c
   ↓
2. detect_format() → Identifies file type
   ↓
3a. play_ogg_opus()          3b. play_custom_opus()
    ↓                             ↓
4. read_ogg_page()           4. Direct packet reading
   ↓                             ↓
5. opus_decode()             5. opus_decode()
   ↓                             ↓
6. Circular buffer           6. Circular buffer
   ↓                             ↓
7. audio_callback() → PortAudio → Speakers
```

## Build Process

### Compilation Steps

1. **Create object directories**: `obj/`, `obj/core/`, `obj/decoder/`, `obj/audio/`
2. **Compile core modules**: `src/core/*.c` → `obj/core/*.o`
3. **Compile decoder modules**: `src/decoder/*.c` → `obj/decoder/*.o`
4. **Compile audio module**: `src/audio/*.c` → `obj/audio/*.o`
5. **Compile main**: `src/main.c` → `obj/main.o`
6. **Link**: All `.o` files → `opusplay.exe`

### Compiler Flags

- `-Wall -Wextra`: Enable all warnings
- `-O2`: Optimization level 2
- `-Iinclude`: Add include directory to search path
- `-lopus -lportaudio -lm`: Link required libraries

## Adding New Features

### Adding a New Core Utility

1. Create `include/new_utility.h`
2. Create `src/core/new_utility.c`
3. Update `CORE_SRC` in Makefile
4. Include header where needed

### Adding a New Decoder

1. Create `include/new_decoder.h`
2. Create `src/decoder/new_decoder.c`
3. Update `DECODER_SRC` in Makefile
4. Add dispatch logic in `main.c`

### Adding a New Audio Processor

1. Create `include/new_processor.h`
2. Create `src/audio/new_processor.c`
3. Update `AUDIO_SRC` in Makefile
4. Integrate into audio pipeline

## Design Principles

1. **Separation of Concerns**: Each module has a single, well-defined responsibility
2. **Modularity**: Functions are independent and reusable
3. **Clear Interfaces**: Header files define clean APIs
4. **Minimal Dependencies**: Modules depend only on what they need
5. **Easy Testing**: Each module can be tested independently

## Dependencies

### External Libraries
- **libopus**: Opus codec implementation
- **libportaudio**: Cross-platform audio I/O
- **libc**: Standard C library (stdio, stdlib, string, signal)

### Internal Dependencies
- All modules depend on `common.h`
- Decoder modules depend on core utilities
- Audio module is independent
- Main depends on all modules

## Memory Management

- **Buffers**: Allocated once at startup, freed at shutdown
- **Packets**: Dynamically allocated per-page, freed after processing
- **No memory leaks**: All allocations have corresponding frees
- **Circular buffer**: Fixed-size, no dynamic resizing during playback

## Thread Safety

- **Single-threaded design**: Main thread for decoding
- **Callback thread**: PortAudio callback runs in separate thread
- **Synchronization**: Volatile flags for thread communication
- **Lock-free**: No mutexes, uses atomic operations via volatile

## Performance Considerations

- **Buffering**: 30-second buffer prevents underruns
- **Optimization**: `-O2` compiler flag
- **Minimal copying**: Direct buffer access where possible
- **Efficient I/O**: Buffered file reading
