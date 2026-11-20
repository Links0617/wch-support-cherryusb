# --- Supported IPs ---
SUPPORT_IPS := usbhs usbss

# --- Default IP ---
ifeq ($(IP),)
IP := usbhs
endif

# --- Compiler Flags ---
CFLAGS += \
	-DCONFIG_USB_MAX_BUS=1 \
	-DCONFIG_USBDEV_MAX_BUS=CONFIG_USB_MAX_BUS \
	-DCONFIG_USBHOST_MAX_BUS=CONFIG_USB_MAX_BUS \

ifeq ($(IP),usbhs)
CFLAGS += \
	-DUSB_IP=0 \
	-DUSBD_REG_BASE0=0x40030000 \
	-DUSBH_REG_BASE0=0x40030100
endif

ifeq ($(IP),usbss)
CFLAGS += \
	-DUSB_IP=1 \
	-DUSBD_REG_BASE0=0x40038000 \
	-DUSBH_REG_BASE0=0x40038000
endif
