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

struct usb_fs_endpoint {
    void **ppBuffer;
    uint32_t *pLength;
    uint32_t nFrames;
    uint32_t aFrames;
    uint32_t status;
    uint32_t flags;
};

struct usb_fs_init {
    struct usb_fs_endpoint *pEndpoints;
    uint32_t ep_index_max;
};

struct usb_fs_open {
    uint8_t ep_index;
    uint8_t ep_no;
    uint32_t max_bufsize;
    uint32_t max_frames;
};

struct usb_fs_start {
    uint8_t ep_index;
};

struct usb_fs_complete {
    uint8_t ep_index;
};

struct usb_fs_stop {
    uint8_t ep_index;
};

struct usb_fs_uninit {
    uint8_t unused;
};
