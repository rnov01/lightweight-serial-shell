# ============================================================
# Project config
# ============================================================

PROJECT        := lightweight-serial-shell-c

CORE_DIR       := core_c
SRC_DIR        := $(CORE_DIR)/src
INC_DIR        := $(CORE_DIR)/include

BUILD_INFO     := $(INC_DIR)/build_info.h
BUILD_DIR      := build

#external drivers
HDIR		   := ../hardware_drivers

#default host (pc)
PLATFORM ?= host
BOARD    ?= arduino_mega_atmega2560

# ============================================================
# Source files
# ============================================================

CORE_SRCS  := $(wildcard $(SRC_DIR)/*.c) $(wildcard core_c/src/helpers/*.c)
MAIN_SRCS  := main.c          

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

#C flags
CSTD    := -std=c11
CWARN   := -Wall -Wextra -Wpedantic
CDEFS   := -D__COLOR_ON__

INCLUDES := -I$(INC_DIR) -I$(INC_DIR)/helpers

include $(HDIR)/mk/hardware_drivers.mk
-include local.mk
# --------------- AVR  ---------------
ifeq ($(PLATFORM),avr)

CC        := avr-gcc
OBJCOPY   := avr-objcopy
SIZE      := avr-size
AVRDUDE   := avrdude

CFLAGS    := $(CSTD) $(CWARN) $(INCLUDES) -Os -mmcu=$(MCU) -DF_CPU=$(F_CPU) -ffunction-sections -fdata-sections
#platform header
CFLAGS += -Icore_c/platforms/$(MCU) -include core_c/platforms/$(MCU)/platform.h -DSH_PLATFORM_HEADER=\"core_c/platforms/$(MCU)/platform.h\"

LDFLAGS   := -mmcu=$(MCU) -Wl,--gc-sections

TARGET    := $(BUILD_DIR)/$(PROJECT)_avr.elf

else ifeq ($(PLATFORM),host)
# ------------ Host (PC) ------------
CC        := gcc
SIZE      := size

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
ifeq ($(strip $(AVRDUDE_PORT)),)
	$(error AVRDUDE_PORT not set. Put it in local.mk)
endif
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
	rm -f $(OBJS) $(TARGET) $(BUILD_INFO) $(BUILD_DIR)/*.hex build_info.h
