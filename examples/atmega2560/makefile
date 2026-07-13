# ============================================================
# ATmega2560 serial shell example
# ============================================================

PROJECT   := serial_shell_atmega2560
MCU       := atmega2560
F_CPU     := 16000000UL

CC        := avr-gcc
OBJCOPY   := avr-objcopy
SIZE      := avr-size
AVRDUDE   := avrdude

BUILD_DIR := build
OBJ_DIR   := $(BUILD_DIR)/obj
GEN_DIR   := $(BUILD_DIR)/generated

OUT_ELF   := $(BUILD_DIR)/$(PROJECT).elf
OUT_HEX   := $(BUILD_DIR)/$(PROJECT).hex
BUILD_INFO := $(GEN_DIR)/build_info.h

# Machine-specific values such as AVRDUDE_PORT
-include local.mk

# Directory containing this Makefile
MK_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

# lightweight-serial-shell repository root
SH_ROOT := $(abspath $(MK_DIR)/../..)

# ============================================================
# Source discovery
# ============================================================

rwildcard = $(foreach d,$(wildcard $(1)/*),\
	$(call rwildcard,$(d),$(2))) \
	$(wildcard $(1)/$(2))

SHELL_SRCS  := $(call rwildcard,$(SH_ROOT)/src,*.c)
DRIVER_SRCS := $(call rwildcard,$(MK_DIR)/drivers,*.c)
APP_SRCS    := $(MK_DIR)/main.c

SRCS := $(SHELL_SRCS) $(DRIVER_SRCS) $(APP_SRCS)

INCLUDES := \
	$(SH_ROOT)/include \
	$(SH_ROOT)/src \
	$(MK_DIR)/drivers \
	$(GEN_DIR)

CSTD  := -std=c11
CWARN := -Wall -Wextra -Wpedantic

CFLAGS := \
	$(CSTD) \
	$(CWARN) \
	-Os \
	-mmcu=$(MCU) \
	-DF_CPU=$(F_CPU) \
	-ffunction-sections \
	-fdata-sections \
	-include build_info.h

LDFLAGS := \
	-mmcu=$(MCU) \
	-Wl,--gc-sections

# ============================================================
# Object mapping
# ============================================================

SHELL_OBJS := $(patsubst \
	$(SH_ROOT)/src/%.c,\
	$(OBJ_DIR)/shell/%.o,\
	$(SHELL_SRCS))

DRIVER_OBJS := $(patsubst \
	$(MK_DIR)/drivers/%.c,\
	$(OBJ_DIR)/drivers/%.o,\
	$(DRIVER_SRCS))

APP_OBJS := $(patsubst \
	$(MK_DIR)/%.c,\
	$(OBJ_DIR)/app/%.o,\
	$(APP_SRCS))

OBJS := $(SHELL_OBJS) $(DRIVER_OBJS) $(APP_OBJS)

# ============================================================
# Generated build metadata
# ============================================================

.PHONY: FORCE
FORCE:

$(BUILD_INFO): FORCE
	@mkdir -p $(dir $@)
	@echo "#pragma once" > $@
	@echo "" >> $@
	@echo "#define FIRM_VER_NAME \"serial-shell example\"" >> $@
	@echo "#define BUILD_TARGET \"$(MCU)\"" >> $@
	@echo "#define GIT_HASH \"$(shell git -C $(SH_ROOT) rev-parse --short HEAD 2>/dev/null || echo unknown)\"" >> $@
	@echo "#define GIT_BRANCH \"$(shell git -C $(SH_ROOT) rev-parse --abbrev-ref HEAD 2>/dev/null || echo unknown)\"" >> $@
	@echo "#define GIT_DIRTY \"$(shell git -C $(SH_ROOT) diff --quiet --ignore-submodules HEAD 2>/dev/null || echo +dirty)\"" >> $@

# ============================================================
# Compile rules
# ============================================================

$(OBJ_DIR)/shell/%.o: $(SH_ROOT)/src/%.c $(BUILD_INFO)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(addprefix -I,$(INCLUDES)) -c $< -o $@

$(OBJ_DIR)/drivers/%.o: $(MK_DIR)/drivers/%.c $(BUILD_INFO)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(addprefix -I,$(INCLUDES)) -c $< -o $@

$(OBJ_DIR)/app/%.o: $(MK_DIR)/%.c $(BUILD_INFO)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(addprefix -I,$(INCLUDES)) -c $< -o $@

# ============================================================
# Link and image generation
# ============================================================

$(OUT_ELF): $(BUILD_INFO) $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $(OBJS) -o $@
	$(SIZE) -C --mcu=$(MCU) $@

$(OUT_HEX): $(OUT_ELF)
	$(OBJCOPY) -O ihex -R .eeprom $< $@

# ============================================================
# User targets
# ============================================================

.PHONY: all flash size clean

all: $(OUT_HEX)

size: $(OUT_ELF)
	$(SIZE) -C --mcu=$(MCU) $<

flash: $(OUT_HEX)
ifndef AVRDUDE_PORT
	$(error AVRDUDE_PORT is not set; define it in local.mk)
endif
	$(AVRDUDE) \
		-p m2560 \
		-c wiring \
		-P $(AVRDUDE_PORT) \
		-b 115200 \
		-D \
		-U flash:w:$(OUT_HEX):i

clean:
	rm -rf $(BUILD_DIR)