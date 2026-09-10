# --- Toolchain ---
PREFIX  := arm-none-eabi
CC      := $(PREFIX)-gcc
AR      := $(PREFIX)-ar
OBJCOPY := $(PREFIX)-objcopy
OBJDUMP := $(PREFIX)-objdump
SIZE    := $(PREFIX)-size

# --- Include Directories ---
INCLUDES += \
    $(FAMILY_DIR)/sdk/Core \
    $(FAMILY_DIR)/sdk/Debug \
    $(FAMILY_DIR)/sdk/Peripheral/inc \
    $(FAMILY_DIR)/system \

# --- Assembly Source Directories ---
ASM_DIR += $(FAMILY_DIR)/sdk/Startup

# --- C Source Directories ---
SRC_DIR += \
    $(FAMILY_DIR)/sdk/Core \
    $(FAMILY_DIR)/sdk/Debug \
    $(FAMILY_DIR)/sdk/Peripheral/src \
    $(FAMILY_DIR)/system \

# --- Library Directories ---
LIB_DIR +=

# --- Assembly Source Files ---
ASMS +=

# --- C Source Files ---
SRCS += $(foreach dir,$(SRC_DIR),$(wildcard $(dir)/*.c))

# --- Libraries ---
LIBS += -lm

# --- Compiler Flags ---
CFLAGS += \
	-mcpu=cortex-m3 \
	-mthumb \
	-mthumb-interwork \
	-Os \
	-fmessage-length=0 \
	-fsigned-char \
	-ffunction-sections \
	-fdata-sections \
	-fno-common \
	-g \
	-std=gnu11 \
	$(addprefix -I,$(INCLUDES))\
	$(addprefix -L,$(LIB_DIR))\

# --- Linker Flags ---
LDFLAGS += \
	$(CFLAGS) \
	$(LIBS) \
	-nostartfiles \
	-Xlinker \
	--gc-sections \
	-Wl,--print-memory-usage \
	-Wl,-Map,$(OUTPUT_DIR)/$(TARGET).map \
	--specs=nano.specs \
	--specs=nosys.specs \
