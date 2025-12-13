# ============================================================
# Project configuration
# ============================================================

PROJECT        := lightweight-serial-shell-c

CORE_DIR       := core_c
SRC_DIR        := $(CORE_DIR)/src
INC_DIR        := $(CORE_DIR)/include

BUILD_INFO     := $(INC_DIR)/build_info.h

# Where to drop final binaries
BUILD_DIR      := build

# Default platform: host (PC)
PLATFORM ?= host
BOARD    ?= mega2560

# ============================================================
# Source files
#  - core shell C sources
#  - plus your demo / main file
# ============================================================

CORE_SRCS  := $(wildcard $(SRC_DIR)/*.c)
MAIN_SRCS  := main.c          # change/extend if you have more demos

SRCS       := $(CORE_SRCS) $(MAIN_SRCS) 



# Make sure build dir exists
$(shell mkdir -p $(BUILD_DIR) >/dev/null 2>&1)

# ============================================================
# Auto-generated build info header
# ============================================================

$(BUILD_INFO):
	@echo "// Auto-generated, do not edit"                                >  $@
	@echo "#ifndef BUILD_INFO_H"                                         >> $@
	@echo "#define BUILD_INFO_H"                                         >> $@
	@echo "#define GIT_HASH      \"$(shell git rev-parse --short HEAD)\"" >> $@
	@echo "#define GIT_BRANCH    \"$(shell git rev-parse --abbrev-ref HEAD)\"" >> $@
	@echo "#define GIT_DIRTY     \"$(shell git diff --quiet >/dev/null 2>&1 || echo +dirty)\"" >> $@
	@echo "#define BUILD_TARGET  \"$(TARGET)\""                          >> $@
	@echo "#define FIRM_VER_NAME \"v1.0\""                               >> $@
	@echo "#endif // BUILD_INFO_H"                                       >> $@


# ============================================================
# Toolchain / flags per platform
# ============================================================

# Common C flags
CSTD    := -std=c11
CWARN   := -Wall -Wextra -Wpedantic
CDEFS   := -D__COLOR_ON__

INCLUDES := -I$(INC_DIR) -I../usart_hal/c/include
# --------------- AVR  ---------------
ifeq ($(PLATFORM),avr)

BOARD ?= mega2560

CC        := avr-gcc
OBJCOPY   := avr-objcopy
SIZE      := avr-size
AVRDUDE   := avrdude

#hal usart driver
HAL_DIR    := ../usart_hal/c/src
BACKEND_DIR := ../ATmega2560_drivers/src
BACKEND_SRCS := $(wildcard $(BACKEND_DIR)/*.c)
HAL_SRCS := $(wildcard $(HAL_DIR)/*.c) ../ATmega2560_drivers/src/tiny_reg_map_2560.c ../ATmega2560_drivers/src/avr_helpers.c

SRCS	:= $(CORE_SRCS) $(MAIN_SRCS) $(HAL_SRCS) 
INCLUDES := -I$(INC_DIR) -I../usart_hal/c/include -I../ATmega2560_drivers/include

# ---------- Per-board settings ----------
#ATmega2560
ifeq ($(BOARD),mega2560)
MCU       		:= atmega2560
F_CPU     		:= 16000000UL
AVRDUDE_MCU 	:= m2560
AVRDUDE_PROG 	:= wiring
AVRDUDE_PORT 	:= COM5
AVRDUDE_BAUD 	:= 115200
#Atmel328p(arduino uno)
else ifeq ($(BOARD),uno)
MCU       		:= atmega328p
F_CPU     		:= 16000000UL
AVRDUDE_MCU 	:= m328p
AVRDUDE_PROG 	:= arduino
AVRDUDE_PORT 	:= COM3
AVRDUDE_BAUD 	:= 115200
else
	$(error Unknown AVR BOARD '$(BOARD)' (expected mega2560 or uno))
endif

CFLAGS    := $(CSTD) $(CWARN) $(INCLUDES) -Os -mmcu=$(MCU) -DF_CPU=$(F_CPU) -ffunction-sections -fdata-sections
LDFLAGS   := -mmcu=$(MCU) -Wl,--gc-sections

TARGET    := $(BUILD_DIR)/$(PROJECT)_avr.elf

else ifeq ($(PLATFORM),host)
# ------------ Host (PC) ------------
CC        := gcc
SIZE      := size

SRCS	:= $(CORE_SRCS) $(MAIN_SRCS) 
INCLUDES := -I$(INC_DIR) 

CFLAGS    := $(CSTD) $(CWARN) $(INCLUDES) -g
LDFLAGS   :=

TARGET    := $(BUILD_DIR)/$(PROJECT)_host

else
$(error Unknown PLATFORM '$(PLATFORM)'. Use PLATFORM=host or PLATFORM=avr)
endif

OBJS       := $(SRCS:.c=.o)

# Make sure everyone sees BUILD_TARGET as the same as $(TARGET)
CDEFS += -include build_info.h

CFLAGS += $(CDEFS)

# ============================================================
# Default targets
# ============================================================

.PHONY: all host avr clean flash size

all: host

host: PLATFORM := host
host: $(TARGET)

avr: PLATFORM := avr
avr: $(TARGET)

# ============================================================
# Build rules
# ============================================================

# Link
$(TARGET): $(BUILD_INFO) $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)
ifeq ($(PLATFORM),avr)
	$(SIZE) -C --mcu=$(MCU) $@
else
	$(SIZE) $@
endif

# Compile
%.o: %.c $(BUILD_INFO)
	$(CC) $(CFLAGS) -c $< -o $@

# ============================================================
# Flash (AVR only)
# ============================================================
AVR_HEX := $(BUILD_DIR)/$(PROJECT).hex

flash: avr $(AVR_HEX)
ifeq ($(PLATFORM),avr)
	$(AVRDUDE) -v -p$(AVRDUDE_MCU) -c$(AVRDUDE_PROG) \
		-P$(AVRDUDE_PORT) -b$(AVRDUDE_BAUD) \
		-D -Uflash:w:$(AVR_HEX):i
else
	@echo "flash target only valid for PLATFORM=avr"
endif

$(AVR_HEX): $(TARGET)
	$(OBJCOPY) -O ihex $< $@

# ============================================================
# Utility targets
# ============================================================

size:
	$(SIZE) $(TARGET) || true

clean:
	rm -f $(OBJS) $(TARGET) $(BUILD_INFO) $(BUILD_DIR)/*.hex
