# ============================================================
# STM32 - NUCLEO F411RE serial shell example
# ============================================================

PROJECT   	:= serial_shell_stm32f411re
MCU       	:= STM32F411xE
BOARD	  	:= nucleo-f411re

CC        	:= arm-none-eabi-gcc
OBJCOPY   	:= arm-none-eabi-objcopy
SIZE      	:= arm-none-eabi-size
OPENOCD   	?= openocd

MK_DIR 		:= $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
SH_ROOT 	:= $(abspath $(MK_DIR)/../..)

BUILD_DIR 	:= $(MK_DIR)/build
OBJ_DIR   	:= $(BUILD_DIR)/obj
GEN_DIR   	:= $(BUILD_DIR)/generated

OUT_ELF   	:= $(BUILD_DIR)/$(PROJECT).elf
OUT_HEX   	:= $(BUILD_DIR)/$(PROJECT).hex
OUT_BIN   	:= $(BUILD_DIR)/$(PROJECT).bin
OUT_MAP  	:= $(BUILD_DIR)/$(PROJECT).map
BUILD_INFO 	:= $(GEN_DIR)/build_info.h

# cubemx generated
CUBEMX_DIR  := $(MK_DIR)/cubemx
STARTUP 	:= $(CUBEMX_DIR)/startup_stm32f411xe.s
LDSCRIPT	:= $(CUBEMX_DIR)/stm32f411re.ld

-include local.mk

OPENOCD_INTERFACE	:= $(OPENOCD_SCRIPTS)/interface/stlink.cfg
OPENOCD_TARGET		:= $(OPENOCD_SCRIPTS)/target/stm32f4x.cfg

# ============================================================
# Source discovery
# ============================================================

rwildcard = $(foreach d,$(wildcard $(1)/*),\
	$(call rwildcard,$(d),$(2))) \
	$(wildcard $(1)/$(2))

SHELL_SRCS  := $(call rwildcard,$(SH_ROOT)/src,*.c)
DRIVER_SRCS := $(call rwildcard,$(MK_DIR)/drivers,*.c)
APP_SRCS    := $(wildcard $(MK_DIR)/*.c) $(wildcard $(MK_DIR)/src/*.c)

SRCS := $(SHELL_SRCS) $(DRIVER_SRCS) $(APP_SRCS) 

INCLUDES := \
	$(SH_ROOT)/include \
	$(SH_ROOT)/src \
	$(MK_DIR)/include \
	$(GEN_DIR)

CSTD  := -std=c11
CWARN := -Wall -Wextra -Wpedantic

CPUFLAGS := -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard

CFLAGS := \
	$(CSTD) \
	$(CWARN) \
	$(CPUFLAGS) \
	-Os -g3 \
	-D$(MCU) \
	-ffunction-sections \
	-fdata-sections \
	-MMD -MP \
	-include build_info.h

LDFLAGS := \
	$(CPUFLAGS) \
	-T$(LDSCRIPT) \
	-Wl,--gc-sections \
	-Wl,-Map=$(OUT_MAP) \
	--specs=nano.specs --specs=nosys.specs

# ============================================================
# Object mapping
# ============================================================

SHELL_OBJS := $(patsubst \
	$(SH_ROOT)/src/%.c,\
	$(OBJ_DIR)/shell/%.o,\
	$(SHELL_SRCS))

STARTUP_OBJS := $(patsubst \
	$(CUBEMX_DIR)/%.s,\
	$(OBJ_DIR)/cubemx/%.o,\
	$(STARTUP))

APP_OBJS := $(patsubst \
	$(MK_DIR)/%.c,\
	$(OBJ_DIR)/app/%.o,\
	$(APP_SRCS))

OBJS := $(SHELL_OBJS) $(STARTUP_OBJS) $(APP_OBJS)

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

$(OBJ_DIR)/cubemx/%.o: $(CUBEMX_DIR)/%.s
	@mkdir -p $(dir $@)
	$(CC) $(CPUFLAGS) -x assembler-with-cpp -c $< -o $@

$(OBJ_DIR)/app/%.o: $(MK_DIR)/%.c $(BUILD_INFO)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(addprefix -I,$(INCLUDES)) -c $< -o $@

# ============================================================
# Link and image generation
# ============================================================

$(OUT_ELF): $(BUILD_INFO) $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $(OBJS) -o $@
	$(SIZE) $@

$(OUT_HEX): $(OUT_ELF)
	$(OBJCOPY) -O ihex -R .eeprom $< $@

$(OUT_BIN): $(OUT_ELF)
	$(OBJCOPY) -O binary $< $@

# ============================================================
# User targets
# ============================================================

.PHONY: all flash size clean

all: $(OUT_HEX) $(OUT_BIN)

size: $(OUT_ELF)
	$(SIZE) $<

flash: $(OUT_ELF)
	$(OPENOCD) -f $(OPENOCD_INTERFACE) -f $(OPENOCD_TARGET) \
	-c "program $(OUT_ELF) verify reset exit"

clean:
	rm -rf $(BUILD_DIR)
-include $(OBJS:.o=.d)