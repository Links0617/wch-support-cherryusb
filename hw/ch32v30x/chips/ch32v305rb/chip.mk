# --- Include Directories ---
INCLUDES += \
	$(CHIP_DIR)/board \
	CherryUSB/port/ch32/legacy/ch32hs \

# --- Assembly Source Directories ---
ASM_DIR +=

# --- C Source Directories ---
SRC_DIR += \
	$(CHIP_DIR)/board \
	CherryUSB/port/ch32/legacy/ch32hs \

# --- Library Directories ---
LIB_DIR +=

# --- Assembly Source Files ---
ASMS += startup_ch32v30x_D8C.S

# --- C Source Files ---
SRCS +=

# --- Libraries ---
LIBS +=

# --- Compiler Flags ---
CFLAGS += \
	-DCH32V30x_D8C \
	-DREG_BASE0=0x40023400

# --- Linker Flags ---
LDFLAGS += -T "$(CHIP_DIR)/linker_script/Link.ld"
