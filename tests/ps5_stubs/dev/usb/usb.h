#pragma once
#include <stdint.h>

struct usb_device_descriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdUSB;
    uint8_t bDeviceClass;
    uint8_t bDeviceSubClass;
    uint8_t bDeviceProtocol;
    uint8_t bMaxPacketSize;
    uint16_t idVendor;
    uint16_t idProduct;
};

struct usb_gen_descriptor {
    void *ugd_data;
    uint32_t ugd_maxlen;
    uint32_t ugd_actlen;
    uint8_t ugd_config_index;
    uint8_t ugd_iface_index;
};
