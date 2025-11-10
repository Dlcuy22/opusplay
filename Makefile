# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iinclude
LDFLAGS = -lopus -lportaudio -lm

# Directories
SRC_DIR = src
CORE_DIR = $(SRC_DIR)/core
DECODER_DIR = $(SRC_DIR)/decoder
AUDIO_DIR = $(SRC_DIR)/audio
OBJ_DIR = obj
OBJ_CORE_DIR = $(OBJ_DIR)/core
OBJ_DECODER_DIR = $(OBJ_DIR)/decoder
OBJ_AUDIO_DIR = $(OBJ_DIR)/audio

# Target
TARGET = opusplay

# Source files
CORE_SRC = $(CORE_DIR)/packet_buffer.c $(CORE_DIR)/signal_handler.c $(CORE_DIR)/format_detector.c
DECODER_SRC = $(DECODER_DIR)/ogg_reader.c $(DECODER_DIR)/custom_opus_player.c $(DECODER_DIR)/ogg_opus_player.c
AUDIO_SRC = $(AUDIO_DIR)/audio_callback.c
MAIN_SRC = $(SRC_DIR)/main.c

# Object files
CORE_OBJ = $(patsubst $(CORE_DIR)/%.c,$(OBJ_CORE_DIR)/%.o,$(CORE_SRC))
DECODER_OBJ = $(patsubst $(DECODER_DIR)/%.c,$(OBJ_DECODER_DIR)/%.o,$(DECODER_SRC))
AUDIO_OBJ = $(patsubst $(AUDIO_DIR)/%.c,$(OBJ_AUDIO_DIR)/%.o,$(AUDIO_SRC))
MAIN_OBJ = $(OBJ_DIR)/main.o

ALL_OBJ = $(CORE_OBJ) $(DECODER_OBJ) $(AUDIO_OBJ) $(MAIN_OBJ)

# Default target
all: $(TARGET)

# Link
$(TARGET): $(ALL_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)
	@echo Build complete: $(TARGET).exe

# Compile main
$(OBJ_DIR)/main.o: $(MAIN_SRC) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile core
$(OBJ_CORE_DIR)/%.o: $(CORE_DIR)/%.c | $(OBJ_CORE_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile decoder
$(OBJ_DECODER_DIR)/%.o: $(DECODER_DIR)/%.c | $(OBJ_DECODER_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile audio
$(OBJ_AUDIO_DIR)/%.o: $(AUDIO_DIR)/%.c | $(OBJ_AUDIO_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create object directories
$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(OBJ_CORE_DIR): | $(OBJ_DIR)
	@mkdir -p $(OBJ_CORE_DIR)

$(OBJ_DECODER_DIR): | $(OBJ_DIR)
	@mkdir -p $(OBJ_DECODER_DIR)

$(OBJ_AUDIO_DIR): | $(OBJ_DIR)
	@mkdir -p $(OBJ_AUDIO_DIR)

# Clean
clean:
	@rm -rf $(OBJ_DIR)
	@rm -f $(TARGET).exe $(TARGET)
	@echo Clean complete

# Rebuild
rebuild: clean all

# Help
help:
	@echo Available targets:
	@echo   all      - Build the project (default)
	@echo   clean    - Remove all build artifacts
	@echo   rebuild  - Clean and rebuild
	@echo   help     - Show this help message

.PHONY: all clean rebuild help