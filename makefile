# ============================================================
# Project Configuration
# ============================================================

MCU        = atmega2560
F_CPU      = 16000000UL
PORT       = COM5
BAUD_UPLOAD = 115200

TARGET     = atmega2560_shell

# ============================================================
# File Structure
# ============================================================

SRC_DIR    = src
INC_DIR    = include

# main file stays in top-level directory
MAIN       = main.cpp

# Collect all C/C++ source files
SRC        = $(MAIN) \
			$(filter-out $(SRC_DIR)/commands.cpp, $(wildcard $(SRC_DIR)/*.cpp)) \
			$(wildcard $(SRC_DIR)/*.c)

OBJ        = $(SRC:.cpp=.o)
OBJ        := $(OBJ:.c=.o)

# ============================================================
# Toolchain
# ============================================================

CC         = avr-gcc
CXX        = avr-g++
OBJCOPY    = avr-objcopy
AVRDUDE    = avrdude

CXXFLAGS   = -std=c++17 -Os -Wall -mmcu=$(MCU) -DF_CPU=$(F_CPU) -I$(INC_DIR)
CFLAGS     = -Os -Wall -mmcu=$(MCU) -DF_CPU=$(F_CPU) -I$(INC_DIR)
LDFLAGS    = -mmcu=$(MCU) -Wl,--print-memory-usage

# ============================================================
# Build Targets
# ============================================================

memusage.txt: $(TARGET).elf
	avr-size --format=avr --mcu=$(MCU) $(TARGET).elf > memusage.txt

all: $(TARGET).hex

$(TARGET).elf: $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $(TARGET).elf $(TARGET).hex

flash: $(TARGET).hex
	$(AVRDUDE) -v -patmega2560 -cwiring -P$(PORT) -b$(BAUD_UPLOAD) -D -Uflash:w:$(TARGET).hex:i

clean:
	rm -f $(OBJ) $(TARGET).elf $(TARGET).hex

.PHONY: all flash clean
