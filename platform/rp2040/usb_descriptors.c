// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2017-2022, Alex Taradov <alex@taradov.com>. All rights reserved.

/*- Includes ----------------------------------------------------------------*/
#include <stdalign.h>
#include "usb.h"
#include "usb_descriptors.h"

/*- Variables ---------------------------------------------------------------*/
const alignas(4) usb_device_descriptor_t usb_device_descriptor =
{
  .bLength            = sizeof(usb_device_descriptor_t),
  .bDescriptorType    = USB_DEVICE_DESCRIPTOR,
  .bcdUSB             = 0x0200,
  .bDeviceClass       = USB_DEVICE_CLASS_MISCELLANEOUS,
  .bDeviceSubClass    = USB_DEVICE_SUBCLASS_COMMON,
  .bDeviceProtocol    = USB_DEVICE_PROTOCOL_INTERFACE_ASSOCIATION,
  .bMaxPacketSize0    = USB_CTRL_EP_SIZE,
  .idVendor           = 0x6666,
  .idProduct          = 0x6600,
  .bcdDevice          = 0x0101,
  .iManufacturer      = USB_STR_MANUFACTURER,
  .iProduct           = USB_STR_PRODUCT,
  .iSerialNumber      = USB_STR_SERIAL_NUMBER,
  .bNumConfigurations = 1,
};

const alignas(4) usb_configuration_hierarchy_t usb_configuration_hierarchy =
{
  .configuration =
  {
    .bLength             = sizeof(usb_configuration_descriptor_t),
    .bDescriptorType     = USB_CONFIGURATION_DESCRIPTOR,
    .wTotalLength        = sizeof(usb_configuration_hierarchy_t),
    .bNumInterfaces      = 3,
    .bConfigurationValue = 1,
    .iConfiguration      = 0,
    .bmAttributes        = 0x80,
    .bMaxPower           = 250, // 500 mA
  },

  .hid_interface =
  {
    .bLength             = sizeof(usb_interface_descriptor_t),
    .bDescriptorType     = USB_INTERFACE_DESCRIPTOR,
    .bInterfaceNumber    = 0,
    .bAlternateSetting   = 0,
    .bNumEndpoints       = 2,
    .bInterfaceClass     = USB_HID_DEVICE_CLASS,
    .bInterfaceSubClass  = 0,
    .bInterfaceProtocol  = 0,
    .iInterface          = USB_STR_CMSIS_DAP,
  },

  .hid =
  {
    .bLength             = sizeof(usb_hid_descriptor_t),
    .bDescriptorType     = USB_HID_DESCRIPTOR,
    .bcdHID              = 0x0111,
    .bCountryCode        = 0,
    .bNumDescriptors     = 1,
    .bDescriptorType1    = USB_HID_REPORT_DESCRIPTOR,
    .wDescriptorLength   = sizeof(usb_hid_report_descriptor),
  },

  .hid_ep_in =
  {
    .bLength             = sizeof(usb_endpoint_descriptor_t),
    .bDescriptorType     = USB_ENDPOINT_DESCRIPTOR,
    .bEndpointAddress    = USB_IN_ENDPOINT | USB_HID_EP_SEND,
    .bmAttributes        = USB_INTERRUPT_ENDPOINT,
    .wMaxPacketSize      = 64,
    .bInterval           = 1,
  },

  .hid_ep_out =
  {
    .bLength             = sizeof(usb_endpoint_descriptor_t),
    .bDescriptorType     = USB_ENDPOINT_DESCRIPTOR,
    .bEndpointAddress    = USB_OUT_ENDPOINT | USB_HID_EP_RECV,
    .bmAttributes        = USB_INTERRUPT_ENDPOINT,
    .wMaxPacketSize      = 64,
    .bInterval           = 1,
  },

  .iad =
  {
    .bLength             = sizeof(usb_interface_association_descriptor_t),
    .bDescriptorType     = USB_INTERFACE_ASSOCIATION_DESCRIPTOR,
    .bFirstInterface     = 1,
    .bInterfaceCount     = 2,
    .bFunctionClass      = USB_CDC_COMM_CLASS,
    .bFunctionSubClass   = USB_CDC_ACM_SUBCLASS,
    .bFunctionProtocol   = 0,
    .iFunction           = USB_STR_COM_PORT,
  },

  .interface_comm =
  {
    .bLength             = sizeof(usb_interface_descriptor_t),
    .bDescriptorType     = USB_INTERFACE_DESCRIPTOR,
    .bInterfaceNumber    = 1,
    .bAlternateSetting   = 0,
    .bNumEndpoints       = 1,
    .bInterfaceClass     = USB_CDC_COMM_CLASS,
    .bInterfaceSubClass  = USB_CDC_ACM_SUBCLASS,
    .bInterfaceProtocol  = 0,
    .iInterface          = 0,
  },

  .cdc_header =
  {
    .bFunctionalLength   = sizeof(usb_cdc_header_functional_descriptor_t),
    .bDescriptorType     = USB_CS_INTERFACE_DESCRIPTOR,
    .bDescriptorSubtype  = USB_CDC_HEADER_SUBTYPE,
    .bcdCDC              = 0x0110,
  },

  .cdc_acm =
  {
    .bFunctionalLength   = sizeof(usb_cdc_abstract_control_managment_descriptor_t),
    .bDescriptorType     = USB_CS_INTERFACE_DESCRIPTOR,
    .bDescriptorSubtype  = USB_CDC_ACM_SUBTYPE,
    .bmCapabilities      = USB_CDC_ACM_SUPPORT_LINE_REQUESTS | USB_CDC_ACM_SUPPORT_SENDBREAK_REQUESTS,
  },

  .cdc_call_mgmt =
  {
    .bFunctionalLength   = sizeof(usb_cdc_call_managment_functional_descriptor_t),
    .bDescriptorType     = USB_CS_INTERFACE_DESCRIPTOR,
    .bDescriptorSubtype  = USB_CDC_CALL_MGMT_SUBTYPE,
    .bmCapabilities      = USB_CDC_CALL_MGMT_OVER_DCI,
    .bDataInterface      = 2,
  },

  .cdc_union =
  {
    .bFunctionalLength   = sizeof(usb_cdc_union_functional_descriptor_t),
    .bDescriptorType     = USB_CS_INTERFACE_DESCRIPTOR,
    .bDescriptorSubtype  = USB_CDC_UNION_SUBTYPE,
    .bMasterInterface    = 1,
    .bSlaveInterface0    = 2,
  },

  .ep_comm =
  {
    .bLength             = sizeof(usb_endpoint_descriptor_t),
    .bDescriptorType     = USB_ENDPOINT_DESCRIPTOR,
    .bEndpointAddress    = USB_IN_ENDPOINT | USB_CDC_EP_COMM,
    .bmAttributes        = USB_INTERRUPT_ENDPOINT,
    .wMaxPacketSize      = 64,
    .bInterval           = 1,
  },

  .interface_data =
  {
    .bLength             = sizeof(usb_interface_descriptor_t),
    .bDescriptorType     = USB_INTERFACE_DESCRIPTOR,
    .bInterfaceNumber    = 2,
    .bAlternateSetting   = 0,
    .bNumEndpoints       = 2,
    .bInterfaceClass     = USB_CDC_DATA_CLASS,
    .bInterfaceSubClass  = USB_CDC_NO_SUBCLASS,
    .bInterfaceProtocol  = 0,
    .iInterface          = 0,
  },

  .ep_in =
  {
    .bLength             = sizeof(usb_endpoint_descriptor_t),
    .bDescriptorType     = USB_ENDPOINT_DESCRIPTOR,
    .bEndpointAddress    = USB_IN_ENDPOINT | USB_CDC_EP_SEND,
    .bmAttributes        = USB_BULK_ENDPOINT,
    .wMaxPacketSize      = 64,
    .bInterval           = 0,
  },

  .ep_out =
  {
    .bLength             = sizeof(usb_endpoint_descriptor_t),
    .bDescriptorType     = USB_ENDPOINT_DESCRIPTOR,
    .bEndpointAddress    = USB_OUT_ENDPOINT | USB_CDC_EP_RECV,
    .bmAttributes        = USB_BULK_ENDPOINT,
    .wMaxPacketSize      = 64,
    .bInterval           = 0,
  },
};

const alignas(4) uint8_t usb_hid_report_descriptor[28] =
{
  0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
  0x09, 0x00,        // Usage (Undefined)
  0xa1, 0x01,        // Collection (Application)
  0x15, 0x00,        //   Logical Minimum (0)
  0x26, 0xff, 0x00,  //   Logical Maximum (255)
  0x75, 0x08,        //   Report Size (8)
  0x95, 0x40,        //   Report Count (64)
  0x09, 0x00,        //   Usage (Undefined)
  0x81, 0x82,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
  0x75, 0x08,        //   Report Size (8)
  0x95, 0x40,        //   Report Count (64)
  0x09, 0x00,        //   Usage (Undefined)
  0x91, 0x82,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Volatile)
  0xc0,              // End Collection
};

const alignas(4) usb_string_descriptor_zero_t usb_string_descriptor_zero =
{
  .bLength               = sizeof(usb_string_descriptor_zero_t),
  .bDescriptorType       = USB_STRING_DESCRIPTOR,
  .wLANGID               = 0x0409, // English (United States)
};

char usb_serial_number[16];

const char *usb_strings[] =
{
  [USB_STR_MANUFACTURER]  = "Alex Taradov",
  [USB_STR_PRODUCT]       = "Combined VCP and CMSIS-DAP Adapter",
  [USB_STR_COM_PORT]      = "Virtual COM-Port",
  [USB_STR_CMSIS_DAP]     = "CMSIS-DAP Adapter",
  [USB_STR_SERIAL_NUMBER] = usb_serial_number,
};


