# ============================================================
# ATmega2560 serial shell unit tests
# ============================================================

PROJECT   := serial_shell_atmega2560
MCU       := atmega2560
F_CPU     := 16000000UL

CC        := avr-gcc
HOST_CC   := gcc

MK_DIR := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
SH_ROOT := $(abspath $(MK_DIR)/..)

BUILD_DIR := build
OBJ_DIR   := $(BUILD_DIR)/obj
GEN_DIR   := $(BUILD_DIR)/generated

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
	-ffunction-sections \
	-fdata-sections


LDFLAGS := -Wl,--gc-sections

# ============================================================
# Host unit-test discovery
# ============================================================

TEST_BUILD_DIR := $(BUILD_DIR)
TEST_OBJ_DIR   := $(TEST_BUILD_DIR)/obj
TEST_BIN       := $(TEST_BUILD_DIR)/shell_tests

# This Makefile is run from the tests directory.
TEST_SRCS := $(call rwildcard,$(MK_DIR),*.c)

# Production shell sources are linked into the host tests.
# Exclude platform-specific files here if they cannot compile on the host.
TEST_SHELL_SRCS := $(SHELL_SRCS)

HOST_TEST_INCLUDES := \
	$(SH_ROOT)/include \
	$(SH_ROOT)/src \
	$(MK_DIR)

HOST_TEST_CFLAGS := \
	-std=c11 \
	-Wall \
	-Wextra \
	-Wpedantic \
	-g3 \
	-O0 

HOST_TEST_LDFLAGS := 

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

TEST_OBJS := $(patsubst \
	$(MK_DIR)/%.c,\
	$(TEST_OBJ_DIR)/tests/%.o,\
	$(TEST_SRCS))

TEST_SHELL_OBJS := $(patsubst \
	$(SH_ROOT)/src/%.c,\
	$(TEST_OBJ_DIR)/shell/%.o,\
	$(TEST_SHELL_SRCS))

HOST_TEST_OBJS := $(TEST_OBJS) $(TEST_SHELL_OBJS)

OBJS := $(SHELL_OBJS) $(DRIVER_OBJS) $(APP_OBJS)

# ============================================================
# Host unit-test compile rules
# ============================================================

$(TEST_OBJ_DIR)/tests/%.o: $(MK_DIR)/%.c $(BUILD_INFO)
	@mkdir -p $(dir $@)
	@$(HOST_CC) \
		$(HOST_TEST_CFLAGS) \
		$(addprefix -I,$(HOST_TEST_INCLUDES)) \
		-c $< \
		-o $@

$(TEST_OBJ_DIR)/shell/%.o: $(SH_ROOT)/src/%.c $(BUILD_INFO)
	@mkdir -p $(dir $@)
	@$(HOST_CC) \
		$(HOST_TEST_CFLAGS) \
		$(addprefix -I,$(HOST_TEST_INCLUDES)) \
		-c $< \
		-o $@


# ============================================================
# User targets
# ============================================================

.PHONY: all test clean clean-test

# ============================================================
# Host unit-test link and run
# ============================================================

$(TEST_BIN): $(HOST_TEST_OBJS)
	@mkdir -p $(dir $@)
	$(HOST_CC) $(HOST_TEST_OBJS) $(HOST_TEST_LDFLAGS) -o $@

.PHONY: test

test: $(TEST_BIN)
	./$(TEST_BIN)

.PHONY: clean clean-tests

clean-tests:
	@echo "Removing $(TEST_BUILD_DIR)"
	@rm -rf "$(TEST_BUILD_DIR)"

clean:
	@echo "Removing $(BUILD_DIR)"
	@rm -rf "$(BUILD_DIR)"