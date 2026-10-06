#pragma once
#define USB_GET_DEVICE_DESC  0x1001
#define USB_GET_FULL_DESC    0x1002

#define USB_FS_INIT      0x1101
#define USB_FS_OPEN      0x1102
#define USB_FS_START     0x1103
#define USB_FS_COMPLETE  0x1104
#define USB_FS_STOP      0x1105
#define USB_FS_UNINIT    0x1106

#define USB_FS_FLAG_SINGLE_SHORT_OK 0x0001u
