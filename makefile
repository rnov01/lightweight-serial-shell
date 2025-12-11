# ============================================================
# Project Configuration
# ============================================================

MCU        = atmega2560
F_CPU      = 16000000UL
PORT       = COM5
BAUD_UPLOAD = 115200

TARGET     = atmega2560_shell

# ============================================================
# Auto-generated build info (Git + target + version)
# ============================================================

BUILD_INFO = build_info.h

$(BUILD_INFO):
	@echo "// Auto-generated, do not edit"                              >  $(BUILD_INFO)
	@echo "#define GIT_HASH    \"$(shell git rev-parse --short HEAD)\"" >> $(BUILD_INFO)
	@echo "#define GIT_BRANCH  \"$(shell git rev-parse --abbrev-ref HEAD)\"" >> $(BUILD_INFO)
	@echo "#define GIT_DIRTY   \"$(shell git diff --quiet >/dev/null 2>&1 || echo +dirty)\"" >> $(BUILD_INFO)
	@echo "#define BUILD_TARGET \"$(TARGET)\""                          >> $(BUILD_INFO)
	@echo "#define FIRM_VER_NAME \"v1.0\""                              >> $(BUILD_INFO)

# ============================================================
# File Structure
# ============================================================

SRC_DIR    = core_c/src
INC_DIR    = core_c/include

# main file stays in top-level directory
MAIN       = main.cpp

# Collect all C/C++ source files
SRC        = $(MAIN) \
			$(filter-out $(SRC_DIR)/commands.cpp, $(wildcard $(SRC_DIR)/*.cpp)) \
			$(wildcard $(SRC_DIR)/*.c) \
			legacy_cpp/src/usart.cpp

OBJ        = $(SRC:.cpp=.o)
OBJ        := $(OBJ:.c=.o)

# ============================================================
# Toolchain
# ============================================================

CC         = avr-gcc
CXX        = avr-g++
OBJCOPY    = avr-objcopy
AVRDUDE    = avrdude

CXXFLAGS   = -std=c++17 -Os -Wall -mmcu=$(MCU) -DF_CPU=$(F_CPU) -I$(INC_DIR) -include $(BUILD_INFO)
CFLAGS     = -Os -Wall -mmcu=$(MCU) -DF_CPU=$(F_CPU) -I$(INC_DIR) -include $(BUILD_INFO)
LDFLAGS    = -mmcu=$(MCU) -Wl,--print-memory-usage 

# ============================================================
# Build Targets
# ============================================================

memusage.txt: $(TARGET).elf
	avr-size --format=avr --mcu=$(MCU) $(TARGET).elf > memusage.txt

all: $(TARGET).hex

$(TARGET).elf: $(BUILD_INFO) $(OBJ)
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
	rm -f $(OBJ) $(TARGET).elf $(TARGET).hex $(BUILD_INFO)

.PHONY: all flash clean
