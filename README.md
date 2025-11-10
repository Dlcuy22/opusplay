# OpusPlay - Modular Opus Audio Player

A well-organized, modular Opus audio player written in C that supports both Ogg Opus and custom raw Opus formats.

## Project Structure

```
opusplay/
├── include/              # Header files
│   ├── common.h         # Common types and constants
│   ├── packet_buffer.h  # Packet buffer management
│   ├── audio_callback.h # PortAudio callback
│   ├── ogg_reader.h     # Ogg file parsing
│   ├── format_detector.h # Format detection
│   ├── player.h         # Player functions
│   └── signal_handler.h # Signal handling
├── src/                 # Source files
│   ├── core/           # Core functionality
│   │   ├── packet_buffer.c    # Packet buffer implementation
│   │   ├── signal_handler.c   # Ctrl+C handler
│   │   └── format_detector.c  # File format detection
│   ├── decoder/        # Decoder modules
│   │   ├── ogg_reader.c       # Ogg page reading
│   │   ├── custom_opus_player.c # Custom format player
│   │   └── ogg_opus_player.c    # Ogg Opus player
│   ├── audio/          # Audio processing
│   │   └── audio_callback.c   # PortAudio callback
│   └── main.c          # Main entry point
├── obj/                # Object files (generated)
├── Makefile            # Build configuration
└── README.md           # This file
```

## Module Organization

### Core (`src/core/`)
- **packet_buffer.c**: Dynamic buffer for Ogg packets
- **signal_handler.c**: Handles Ctrl+C for graceful shutdown
- **format_detector.c**: Detects file format (Ogg Opus vs Custom)

### Decoder (`src/decoder/`)
- **ogg_reader.c**: Parses Ogg container format and extracts packets
- **custom_opus_player.c**: Plays custom raw Opus format
- **ogg_opus_player.c**: Plays standard Ogg Opus files

### Audio (`src/audio/`)
- **audio_callback.c**: PortAudio callback for real-time playback

## Building

### Prerequisites
- GCC compiler
- libopus
- libportaudio
- make

### Build Commands

```bash
# Build the project
make

# Clean build artifacts
make clean

# Rebuild from scratch
make rebuild

# Show help
make help
```

## Usage

```bash
# Play an Ogg Opus file
./opusplay song.ogg

# Play a custom Opus file
./opusplay recording.opus
```

### Controls
- **Ctrl+C**: Stop playback

## Features

- ✅ Modular architecture with clear separation of concerns
- ✅ Supports Ogg Opus (universal format)
- ✅ Supports custom raw Opus format
- ✅ Real-time playback with buffering
- ✅ Graceful shutdown on Ctrl+C
- ✅ Automatic format detection
- ✅ Clean compilation with minimal warnings

## Technical Details

### Audio Pipeline
1. **Format Detection**: Identifies file type
2. **Decoding**: Opus decoder processes compressed audio
3. **Buffering**: Circular buffer (30 seconds capacity)
4. **Playback**: PortAudio streams to audio device

### Buffer Management
- 30-second circular buffer
- 5-second maximum buffered data during playback
- Prevents buffer overflow and underflow

## Development

### Adding New Features
1. Add header declaration in `include/`
2. Implement in appropriate `src/` subdirectory
3. Update `Makefile` if adding new source files
4. Rebuild with `make rebuild`

### Code Style
- Follow existing naming conventions
- Keep functions focused and modular
- Document complex algorithms
- Use meaningful variable names

## License

This project is open source.
