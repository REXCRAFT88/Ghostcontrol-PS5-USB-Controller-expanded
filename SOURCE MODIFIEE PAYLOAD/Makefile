PS5_HOST ?=
PORT     ?= 9021

TARGET      := GhostControl-Expanded.elf
STOP_TARGET := GhostControl-Stop.elf

ifndef PS5_PAYLOAD_SDK
$(error PS5_PAYLOAD_SDK is not set)
endif

include $(PS5_PAYLOAD_SDK)/toolchain/prospero.mk

CFLAGS  += -D__PROSPERO__ -Wall -Wextra -g -O2 -fPIC -fno-stack-protector
LDFLAGS += -lScePad -lSceUserService -lpthread -ldl

SRC := gc_main.c shellui_pad.c usb_helpers.c controller_nintendo.c \
       controller_xbox.c controller_ds4.c controller_ds3.c \
       controller_mamba.c controller_gamecube.c controller_generic_hid.c

.PHONY: all stop clean deploy deploy-stop

all: $(TARGET) $(STOP_TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

stop: $(STOP_TARGET)

$(STOP_TARGET): gc_stop.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGET) $(STOP_TARGET)

deploy: $(TARGET)
ifndef PS5_HOST
	$(error PS5_HOST is not set)
endif
	nc -w 8 $(PS5_HOST) $(PORT) < $(TARGET)

deploy-stop: $(STOP_TARGET)
ifndef PS5_HOST
	$(error PS5_HOST is not set)
endif
	nc -w 8 $(PS5_HOST) $(PORT) < $(STOP_TARGET)
