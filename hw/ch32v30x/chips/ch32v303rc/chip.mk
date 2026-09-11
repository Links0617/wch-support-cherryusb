# --- Include Directories ---
INCLUDES += \
	$(CHIP_DIR)/board \
	CherryUSB/port/wch/usbfs \

# --- Assembly Source Directories ---
ASM_DIR +=

# --- C Source Directories ---
SRC_DIR += \
	$(CHIP_DIR)/board \
	CherryUSB/port/wch/usbfs \

# --- Library Directories ---
LIB_DIR +=

# --- Assembly Source Files ---
ASMS += startup_ch32v30x_D8.S

# --- C Source Files ---
SRCS +=

# --- Libraries ---
LIBS +=

# --- Compiler Flags ---
CFLAGS += \
	-DCH32V30x_D8 \
	-DREG_BASE0=0x50000000

# --- Linker Flags ---
LDFLAGS += -T "$(CHIP_DIR)/linker_script/Link.ld"
