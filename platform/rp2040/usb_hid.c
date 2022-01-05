// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2019-2022, Alex Taradov <alex@taradov.com>. All rights reserved.

/*- Includes ----------------------------------------------------------------*/
#include <stdbool.h>
#include <stdalign.h>
#include <string.h>
#include "utils.h"
#include "usb.h"
#include "usb_std.h"
#include "usb_hid.h"
#include "usb_descriptors.h"

/*- Prototypes --------------------------------------------------------------*/
static void usb_hid_ep_send_callback(int size);
static void usb_hid_ep_recv_callback(int size);

/*- Implementations ---------------------------------------------------------*/

//-----------------------------------------------------------------------------
void usb_hid_init(void)
{
  usb_set_callback(USB_HID_EP_SEND, usb_hid_ep_send_callback);
  usb_set_callback(USB_HID_EP_RECV, usb_hid_ep_recv_callback);
}

//-----------------------------------------------------------------------------
void usb_hid_send(uint8_t *data, int size)
{
  usb_send(USB_HID_EP_SEND, data, size);
}

//-----------------------------------------------------------------------------
void usb_hid_recv(uint8_t *data, int size)
{
  usb_recv(USB_HID_EP_RECV, data, size);
}

//-----------------------------------------------------------------------------
static void usb_hid_ep_send_callback(int size)
{
  usb_hid_send_callback();
  (void)size;
}

//-----------------------------------------------------------------------------
static void usb_hid_ep_recv_callback(int size)
{
  usb_hid_recv_callback(size);
}

//-----------------------------------------------------------------------------
bool usb_hid_handle_request(usb_request_t *request)
{
  int length = request->wLength;

  switch ((request->bRequest << 8) | request->bmRequestType)
  {
    case USB_CMD(IN, INTERFACE, STANDARD, GET_DESCRIPTOR):
    {
      length = LIMIT(length, sizeof(usb_hid_report_descriptor));

      usb_control_send((uint8_t *)usb_hid_report_descriptor, length);
    } break;

    default:
      return false;
  }

  return true;
}

