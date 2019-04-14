/*
 * Copyright (c) 2017, Alex Taradov <alex@taradov.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*- Includes ----------------------------------------------------------------*/
#include <stdbool.h>
#include <stdalign.h>
#include <string.h>
#include "utils.h"
#include "usb.h"
#include "usb_std.h"
#include "usb_cdc.h"
#include "usb_descriptors.h"

/*- Prototypes --------------------------------------------------------------*/
static void usb_cdc_send_state_notify(void);
static void usb_cdc_ep_comm_callback(int size);
static void usb_cdc_ep_send_callback(int size);
static void usb_cdc_ep_recv_callback(int size);

/*- Variables ---------------------------------------------------------------*/
static usb_cdc_line_coding_t usb_cdc_line_coding =
{
  .dwDTERate   = 115200,
  .bCharFormat = USB_CDC_1_STOP_BIT,
  .bParityType = USB_CDC_NO_PARITY,
  .bDataBits   = USB_CDC_8_DATA_BITS,
};

static alignas(4) usb_cdc_notify_serial_state_t usb_cdc_notify_message;
static int usb_cdc_serial_state;
static bool usb_cdc_comm_busy;

/*- Implementations ---------------------------------------------------------*/

//-----------------------------------------------------------------------------
void usb_cdc_init(void)
{
  usb_set_callback(USB_CDC_EP_COMM, usb_cdc_ep_comm_callback);
  usb_set_callback(USB_CDC_EP_SEND, usb_cdc_ep_send_callback);
  usb_set_callback(USB_CDC_EP_RECV, usb_cdc_ep_recv_callback);

  usb_cdc_notify_message.request.bmRequestType = USB_IN_TRANSFER |
      USB_INTERFACE_RECIPIENT | USB_CLASS_REQUEST;
  usb_cdc_notify_message.request.bRequest = USB_CDC_NOTIFY_SERIAL_STATE;
  usb_cdc_notify_message.request.wValue = 0;
  usb_cdc_notify_message.request.wIndex = 0;
  usb_cdc_notify_message.request.wLength = sizeof(uint16_t);
  usb_cdc_notify_message.value = 0;

  usb_cdc_serial_state = 0;
  usb_cdc_comm_busy = false;

  usb_cdc_line_coding_updated(&usb_cdc_line_coding);
}

//-----------------------------------------------------------------------------
void usb_cdc_send(uint8_t *data, int size)
{
  usb_send(USB_CDC_EP_SEND, data, size);
}

//-----------------------------------------------------------------------------
void usb_cdc_recv(uint8_t *data, int size)
{
  usb_recv(USB_CDC_EP_RECV, data, size);
}

//-----------------------------------------------------------------------------
void usb_cdc_set_state(int mask)
{
  usb_cdc_serial_state |= mask;

  usb_cdc_send_state_notify();
}

//-----------------------------------------------------------------------------
void usb_cdc_clear_state(int mask)
{
  usb_cdc_serial_state &= ~mask;

  usb_cdc_send_state_notify();
}

//-----------------------------------------------------------------------------
static void usb_cdc_send_state_notify(void)
{
  if (usb_cdc_comm_busy)
    return;

  if (usb_cdc_serial_state != usb_cdc_notify_message.value)
  {
    usb_cdc_comm_busy = true;
    usb_cdc_notify_message.value = usb_cdc_serial_state;

    usb_send(USB_CDC_EP_COMM, (uint8_t *)&usb_cdc_notify_message, sizeof(usb_cdc_notify_serial_state_t));
  }
}

//-----------------------------------------------------------------------------
static void usb_cdc_ep_comm_callback(int size)
{
  const int one_shot = USB_CDC_SERIAL_STATE_BREAK | USB_CDC_SERIAL_STATE_RING |
      USB_CDC_SERIAL_STATE_FRAMING | USB_CDC_SERIAL_STATE_PARITY |
      USB_CDC_SERIAL_STATE_OVERRUN;

  usb_cdc_comm_busy = false;

  usb_cdc_notify_message.value &= ~one_shot;
  usb_cdc_serial_state &= ~one_shot;

  usb_cdc_send_state_notify();

  (void)size;
}

//-----------------------------------------------------------------------------
static void usb_cdc_ep_send_callback(int size)
{
  usb_cdc_send_callback();
  (void)size;
}

//-----------------------------------------------------------------------------
static void usb_cdc_ep_recv_callback(int size)
{
  usb_cdc_recv_callback(size);
}

//-----------------------------------------------------------------------------
static void usb_cdc_set_line_coding_handler(uint8_t *data, int size)
{
  usb_cdc_line_coding_t *line_coding = (usb_cdc_line_coding_t *)data;

  if (sizeof(usb_cdc_line_coding_t) != size)
    return;

  usb_cdc_line_coding = *line_coding;

  usb_cdc_line_coding_updated(&usb_cdc_line_coding);
}

//-----------------------------------------------------------------------------
bool usb_cdc_handle_request(usb_request_t *request)
{
  int length = request->wLength;

  switch ((request->bRequest << 8) | request->bmRequestType)
  {
    case USB_CMD(OUT, INTERFACE, CLASS, CDC_SET_LINE_CODING):
    {
      length = LIMIT(length, sizeof(usb_cdc_line_coding_t));

      usb_control_recv(usb_cdc_set_line_coding_handler);
    } break;

    case USB_CMD(IN, INTERFACE, CLASS, CDC_GET_LINE_CODING):
    {
      length = LIMIT(length, sizeof(usb_cdc_line_coding_t));

      usb_control_send((uint8_t *)&usb_cdc_line_coding, length);
    } break;

    case USB_CMD(OUT, INTERFACE, CLASS, CDC_SET_CONTROL_LINE_STATE):
    {
      usb_cdc_control_line_state_update(request->wValue);

      usb_control_send_zlp();
    } break;

    default:
      return false;
  }

  return true;
}

//-----------------------------------------------------------------------------
WEAK void usb_cdc_control_line_state_update(int line_state)
{
  (void)line_state;
}


