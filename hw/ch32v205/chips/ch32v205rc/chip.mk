# --- Include Directories ---
INCLUDES += \
	$(CHIP_DIR)/board \
	CherryUSB/port/ch32/usbhs \

# --- Assembly Source Directories ---
ASM_DIR +=

# --- C Source Directories ---
SRC_DIR += \
	$(CHIP_DIR)/board \
	CherryUSB/port/ch32/usbhs \

# --- Library Directories ---
LIB_DIR +=

# --- Assembly Source Files ---
ASMS +=

# --- C Source Files ---
SRCS +=

# --- Libraries ---
LIBS +=

# --- Compiler Flags ---
CFLAGS += -DREG_BASE0=0x40023400

# --- Linker Flags ---
LDFLAGS += -T "$(CHIP_DIR)/linker_script/Link.ld"
