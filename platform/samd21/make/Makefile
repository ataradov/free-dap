##############################################################################
BUILD = build
BIN = free_dap

##############################################################################
.PHONY: all directory clean size

CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

ifeq ($(OS), Windows_NT)
  MKDIR = gmkdir
else
  MKDIR = mkdir
endif

CFLAGS += -W -Wall -Wextra --std=gnu11 -Os
CFLAGS += -fno-diagnostics-show-caret
CFLAGS += -fdata-sections -ffunction-sections
CFLAGS += -funsigned-char -funsigned-bitfields
CFLAGS += -mcpu=cortex-m0plus -mthumb
CFLAGS += -MD -MP -MT $(BUILD)/$(*F).o -MF $(BUILD)/$(@F).d
CFLAGS += -flto

LDFLAGS += -mcpu=cortex-m0plus -mthumb
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,--script=../linker/samd21e17_bl_8k.ld
LDFLAGS += -flto

INCLUDES += \
  -I../include \
  -I../../samdx1/usb \
  -I../../samdx1 \
  -I../../.. \
  -I..

SRCS += \
  ../sys_init.c \
  ../../samdx1/main.c \
  ../../samdx1/uart.c \
  ../../samdx1/usb/usb_samdx1.c \
  ../../samdx1/usb/usb_std.c \
  ../../samdx1/usb/usb_cdc.c \
  ../../samdx1/usb/usb_hid.c \
  ../../samdx1/usb/usb_winusb.c \
  ../../samdx1/usb_descriptors.c \
  ../startup_samd21.c \
  ../../../dap.c \

DEFINES += \
  -D__SAMD21E17A__ \
  -DDONT_USE_CMSIS_INIT \
  -DF_CPU=48000000

CFLAGS += $(INCLUDES) $(DEFINES)

OBJS = $(addprefix $(BUILD)/, $(notdir %/$(subst .c,.o, $(SRCS))))

all: directory $(BUILD)/$(BIN).elf $(BUILD)/$(BIN).hex $(BUILD)/$(BIN).bin size

$(BUILD)/$(BIN).elf: $(OBJS)
	@echo LD $@
	@$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o $@

$(BUILD)/$(BIN).hex: $(BUILD)/$(BIN).elf
	@echo OBJCOPY $@
	@$(OBJCOPY) -O ihex $^ $@

$(BUILD)/$(BIN).bin: $(BUILD)/$(BIN).elf
	@echo OBJCOPY $@
	@$(OBJCOPY) -O binary $^ $@

%.o:
	@echo CC $@
	@$(CC) $(CFLAGS) $(filter %/$(subst .o,.c,$(notdir $@)), $(SRCS)) -c -o $@

directory:
	@$(MKDIR) -p $(BUILD)

size: $(BUILD)/$(BIN).elf
	@echo size:
	@$(SIZE) -t $^

clean:
	@echo clean
	@-rm -rf $(BUILD)

-include $(wildcard $(BUILD)/*.d)

