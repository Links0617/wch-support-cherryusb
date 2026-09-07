ifneq ($(filter all, $(MAKECMDGOALS)),)
    ifndef CHIP
        $(error No 'CHIP' parameter provided, Usage: make all CHIP=ch32v307vc)
    endif

# --- Include Directories ---
INCLUDES := \
	src \
	CherryUSB/core \
	CherryUSB/common \
	$(wildcard CherryUSB/class/*) \

# --- Assembly Source Directories ---
ASM_DIR :=

# --- C Source Directories ---
SRC_DIR :=

# --- Library Directories ---
LIB_DIR :=

# --- Assembly Source Files ---
ASMS :=

# --- C Source Files ---
SRCS :=

# --- Libraries ---
LIBS :=

# --- Compiler Flags ---
CFLAGS :=

# --- Linker Flags ---
LDFLAGS :=

# --- Chip Directories ---
CHIP_DIR := $(wildcard $(abspath hw/*/chips/$(CHIP)))

# --- Family Directories ---
FAMILY_DIR := $(abspath $(CHIP_DIR)/../../)

# --- Include Chip and Family Makefiles ---
include $(CHIP_DIR)/chip.mk
include $(FAMILY_DIR)/family.mk

# --- Add C Source Files Directories ---
SRC_DIR += \
	$(CURDIR)/src \
	$(CURDIR)/CherryUSB/demo \
	$(CURDIR)/CherryUSB/core \
	$(CURDIR)/CherryUSB/common \
	$(wildcard $(CURDIR)/CherryUSB/class/*)

# --- Add C Source Files ---
SRCS += \
	$(CURDIR)/CherryUSB/demo/cdc_acm_template.c \
	$(CURDIR)/CherryUSB/class/cdc/usbd_cdc_acm.c \
	$(wildcard $(CURDIR)/src/*.c) \
	$(wildcard $(CURDIR)/CherryUSB/core/*.c) \

# --- Vpath for Source Files ---
vpath %.S $(ASM_DIR)
vpath %.c $(SRC_DIR)

# --- Output Directories ---
OBJECT_DIR := build/$(CHIP)/object
OUTPUT_DIR := build/$(CHIP)/output

# --- Create Output Directories ---
$(shell mkdir -p $(OBJECT_DIR))
$(shell mkdir -p $(OUTPUT_DIR))

# --- Object Files ---
OBJECT_FILES := \
	$(addprefix $(OBJECT_DIR)/,$(notdir $(ASMS:.S=.o))) \
	$(addprefix $(OBJECT_DIR)/,$(notdir $(SRCS:.c=.o))) \

# --- Target and Output Files ---
TARGET := $(notdir $(CHIP))
ELF_FILE := $(OUTPUT_DIR)/$(TARGET).elf
BIN_FILE := $(OUTPUT_DIR)/$(TARGET).bin
HEX_FILE := $(OUTPUT_DIR)/$(TARGET).hex
MAP_FILE := $(OUTPUT_DIR)/$(TARGET).map
LST_FILE := $(OUTPUT_DIR)/$(TARGET).lst
endif

# --- Build Targets ---
all: $(BIN_FILE) $(HEX_FILE) $(LST_FILE)

# --- Clean Target ---
clear:
	@rm -rf build

# --- Compilation Rules ---
$(BIN_FILE): $(ELF_FILE)
	@$(OBJCOPY) -Obinary $< $(BIN_FILE)

$(HEX_FILE): $(ELF_FILE)
	@$(OBJCOPY) -Oihex $< $(HEX_FILE)

$(LST_FILE): $(ELF_FILE)
	@$(OBJDUMP) --all-headers --demangle --disassemble $< > $(LST_FILE)

$(ELF_FILE): $(OBJECT_FILES)
	@echo "[LINK] $@"
	@$(CC) $(LDFLAGS) $(OBJECT_FILES) -o $@
	@echo "Build complete!"

$(OBJECT_DIR)/%.o: %.c
	@echo "[CC]   $< -> $@"
	@$(CC) $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -c $< -o $@

$(OBJECT_DIR)/%.o: %.S
	@echo "[CC]   $< -> $@"
	@$(CC) $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -c $< -o $@

# --- Phony Targets ---
.PHONY: all clear
