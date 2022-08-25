// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022, Alex Taradov <alex@taradov.com>. All rights reserved.

/*- Includes ----------------------------------------------------------------*/
#include "M480.h"
#include "usb.h"
#include "usb_std.h"
#include "usb_descriptors.h"

/*- Definitions -------------------------------------------------------------*/
#define USB_EP_NUM     12
#define USB_MEM_SIZE   1024
#define USBD_BUF_BASE (USBD_BASE + 0x100ul)
#define USBD_CFG_EPMODE_IN (2ul << USBD_CFG_STATE_Pos)
#define USBD_CFG_EPMODE_OUT (1ul << USBD_CFG_STATE_Pos)

/*- Variables ---------------------------------------------------------------*/
static uint8_t usb_ctrl_out_buf[USB_CTRL_EP_SIZE];
static void (*usb_control_recv_callback)(uint8_t *data, int size);
static uint8_t *usb_ep_data[USB_EP_NUM];
static int usb_ep_size[USB_EP_NUM];
static int usb_setup_length;
static int usb_ep_mem_ptr;

/*- Prototypes --------------------------------------------------------------*/
static void usb_reset_endpoints(void);

/*- Implementations ---------------------------------------------------------*/

//-----------------------------------------------------------------------------
void usb_hw_init(void)
{
  SYS->USBPHY = (SYS->USBPHY & ~SYS_USBPHY_USBROLE_Msk) | SYS_USBPHY_USBEN_Msk | SYS_USBPHY_SBO_Msk;
  CLK->APBCLK0 |= CLK_APBCLK0_USBDCKEN_Msk;

  // Set PA.12 ~ PA.14 to input mode and USB
  PA->MODE &= ~(GPIO_MODE_MODE12_Msk | GPIO_MODE_MODE13_Msk | GPIO_MODE_MODE14_Msk);
  SYS->GPA_MFPH &= ~(SYS_GPA_MFPH_PA12MFP_Msk|SYS_GPA_MFPH_PA13MFP_Msk|SYS_GPA_MFPH_PA14MFP_Msk);
  SYS->GPA_MFPH |= (
    (0x0EUL << SYS_GPA_MFPH_PA12MFP_Pos) | // PA12 USB_VBUS
    (0x0EUL << SYS_GPA_MFPH_PA13MFP_Pos) | // PA13 USB_D
    (0x0EUL << SYS_GPA_MFPH_PA14MFP_Pos) // PA14 USB_P
  );

  USBD->ATTR = 0x6D0; //USBD_ATTR_PHYEN_Msk | USBD_ATTR_USBEN_Msk | USBD_ATTR_BYTEM_Msk;
  USBD->SE0 |= USBD_SE0_SE0_Msk;
  usb_setup_length = -1;
  usb_reset_endpoints();
  USBD->SE0 &= ~USBD_SE0_SE0_Msk;
  USBD->ATTR = 0x7D0;
}

//-----------------------------------------------------------------------------
void usb_attach(void)
{
  USBD->ATTR |= USBD_ATTR_DPPUEN_Msk;
}

//-----------------------------------------------------------------------------
void usb_detach(void)
{
  USBD->ATTR &= ~USBD_ATTR_DPPUEN_Msk;

}

//-----------------------------------------------------------------------------
static void usb_reset_endpoints(void)
{
  for (int i = 0; i < USB_EP_NUM; i++)
  {
    usb_ep_size[i] = 0;
    usb_ep_data[i] = NULL;
    USBD->EP[i].CFG &= ~USBD_CFG_DSQSYNC_Msk;
  }

  // EP0 control IN (tx)
  USBD->EP[0].CFG = USBD_CFG_CSTALL_Msk | USBD_CFG_EPMODE_IN | 0;
  USBD->EP[0].BUFSEG = 8;
  // USBD->EP[0].MXPLD = 0;

  // EP1 control OUT (rx)
  USBD->EP[1].CFG = USBD_CFG_CSTALL_Msk | USBD_CFG_EPMODE_OUT | 0;
  USBD->EP[1].BUFSEG = 8 + USB_CTRL_EP_SIZE;
  USBD->EP[1].MXPLD = USB_CTRL_EP_SIZE; // Ready to receive

  // Start EP2+ at end of EP1
  usb_ep_mem_ptr = 8 + 2 * USB_CTRL_EP_SIZE;;
}

//-----------------------------------------------------------------------------
void usb_configure_endpoint(usb_endpoint_descriptor_t *desc)
{
  int ep, dir, type, size;

  ep = desc->bEndpointAddress & USB_INDEX_MASK;
  dir = desc->bEndpointAddress & USB_DIRECTION_MASK;
  type = desc->bmAttributes & 0x03;
  size = desc->wMaxPacketSize;

  if (type == USB_CONTROL_ENDPOINT)
    while (1);
  else if (type == USB_ISOCHRONOUS_ENDPOINT)
    type = USBD_CFG_ISOCH_Msk; /*Isochronous*/
  else if (type == USB_BULK_ENDPOINT)
    type = 0;
  else
    type = 0;

  if (dir == USB_IN_ENDPOINT)
    dir = USBD_CFG_EPMODE_IN;
  else
    dir = USBD_CFG_EPMODE_OUT;

  // EP 0 and 1 are reserved for ctrl so add 1 to ep
  USBD->EP[ep+1].BUFSEG = usb_ep_mem_ptr;

  usb_ep_mem_ptr += size;

  while (usb_ep_mem_ptr > USB_MEM_SIZE);

  USBD->EP[ep+1].CFG = type | dir | (ep << USBD_CFG_EPNUM_Pos);
  USBD->EP[ep+1].CFGP &= ~USBD_CFGP_SSTALL_Msk;

  if (dir == USBD_CFG_EPMODE_OUT)
    USBD->EP[ep+1].MXPLD = size;
}

//-----------------------------------------------------------------------------
bool usb_endpoint_configured(int ep, int dir)
{
  return (0 != (USBD->EP[ep+1].CFG & USBD_CFG_STATE_Msk));
  (void)dir;
}

//-----------------------------------------------------------------------------
int usb_endpoint_get_status(int ep, int dir)
{
  return (USBD->EP[ep+1].CFGP & USBD_CFGP_SSTALL_Msk);
  (void)dir;
}

//-----------------------------------------------------------------------------
void usb_endpoint_set_feature(int ep, int dir)
{
  USBD->EP[ep+1].CFGP |= USBD_CFGP_SSTALL_Msk;
  (void)dir;
}

//-----------------------------------------------------------------------------
void usb_endpoint_clear_feature(int ep, int dir)
{
  USBD->EP[ep+1].CFGP &= ~USBD_CFGP_SSTALL_Msk;
  (void)dir;
}

//-----------------------------------------------------------------------------
void usb_set_address(int address)
{
  USBD->FADDR = address;
}

//-----------------------------------------------------------------------------
void usb_send(int ep, uint8_t *data, int size)
{
  int idx;
  switch (ep)
  {
    case USB_HID_EP_SEND:
    case USB_BULK_EP_SEND:
    {
      idx = ep + 1;
      break;
    }
    default:
      while (1); // Invalid ep
  }
  uint8_t* ep_buf = (uint8_t *)(USBD_BUF_BASE + USBD->EP[idx].BUFSEG);
  while (size)
  {
    // Copy to EP buffer
    int transfer_size = USB_LIMIT(size, USB_DAP_EP_SIZE);
    for (int i = 0; i < transfer_size; i++)
      ep_buf[i] = data[i];
    USBD->EP[idx].MXPLD = transfer_size;

    // Wait for completion
    while (0 == (USBD->INTSTS & USBD_INTSTS_USBIF_Msk));
    USBD->INTSTS = USBD_INTSTS_USBIF_Msk;

    size -= transfer_size;
    data += transfer_size;
  }
}

//-----------------------------------------------------------------------------
void usb_recv(int ep, uint8_t *data, int size)
{
  int idx;
  switch (ep)
  {
    case USB_HID_EP_RECV:
    case USB_BULK_EP_RECV:
    {
      idx = ep + 1;
      break;
    }
    default:
      while (1); // Invalid ep
  }
  usb_ep_size[idx] = size;
  usb_ep_data[idx] = data;
  USBD->EP[idx].MXPLD = size; // Ready to recv
}

//-----------------------------------------------------------------------------
void usb_control_send_zlp(void)
{
  USBD->EP[0].MXPLD = 0;
  USBD->EP[1].MXPLD = USB_CTRL_EP_SIZE; // Ready to recv
  while (0 == (USBD->INTSTS & USBD_INTSTS_USBIF_Msk));
  USBD->INTSTS = USBD_INTSTS_USBIF_Msk;
}

//-----------------------------------------------------------------------------
void usb_control_stall(void)
{
  USBD->EP[0].CFGP |= USBD_CFGP_SSTALL_Msk;
  USBD->EP[1].CFGP |= USBD_CFGP_SSTALL_Msk;
}

//-----------------------------------------------------------------------------
void usb_control_send(uint8_t *data, int size)
{
  uint8_t* ep0_buf = (uint8_t *)(USBD_BUF_BASE + USBD->EP[0].BUFSEG);
  while (size)
  {
    int transfer_size = USB_LIMIT(size, usb_device_descriptor.bMaxPacketSize0);

    for (int i = 0; i < transfer_size; i++)
      ep0_buf[i] = data[i];

    USBD->EP[0].MXPLD = transfer_size;
    while (0 == (USBD->INTSTS & USBD_INTSTS_USBIF_Msk));
    USBD->INTSTS = USBD_INTSTS_USBIF_Msk;

    size -= transfer_size;
    data += transfer_size;
  }
  usb_control_send_zlp();
}

//-----------------------------------------------------------------------------
void usb_control_recv(void (*callback)(uint8_t *data, int size))
{
  usb_control_recv_callback = callback;
}

//-----------------------------------------------------------------------------
void usb_task(void)
{
  volatile USBD_T* dev = USBD;
  int busint = USBD->INTSTS;
  int state = USBD->ATTR & 0xF;
  int epint;

  if (busint & USBD_INTSTS_VBDETIF_Msk)
  {
    USBD->INTSTS = USBD_INTSTS_VBDETIF_Msk;
    HAL_GPIO_DAP_STATUS_clr();
    if (USBD->VBUSDET & USBD_VBUSDET_VBUSDET_Msk)
      usb_attach();
    else
      usb_detach();
  }

  if (busint & USBD_INTSTS_NEVWKIF_Msk)
    USBD->INTSTS = USBD_INTSTS_NEVWKIF_Msk;

  if (busint & USBD_INTSTS_BUSIF_Msk)
  {
    USBD->INTSTS = USBD_INTSTS_BUSIF_Msk;
    HAL_GPIO_DAP_STATUS_clr();
    if (state & USBD_ATTR_USBRST_Msk)
    {
      USBD->FADDR  = 0;
      USBD->STBUFSEG = 0;
      usb_reset_endpoints();
      //USBD->SE0 &= ~USBD_SE0_SE0_Msk;
      USBD->ATTR = 0x7D0;

    }
    if (state & USBD_ATTR_SUSPEND_Msk)
      USBD->ATTR &= ~USBD_ATTR_PHYEN_Msk;
    if (state & USBD_ATTR_RESUME_Msk)
      USBD->ATTR = 0x7D0;

  }

  if (busint & USBD_INTSTS_USBIF_Msk)
  {
    HAL_GPIO_DAP_STATUS_clr();

    USBD->INTSTS = USBD_INTSTS_USBIF_Msk;

    if (busint & USBD_INTSTS_SETUP_Msk)
    {
      usb_request_t request;

      // Read setup packet from USB buffer
      uint8_t* setup_buf = (uint8_t *)(USBD_BUF_BASE);
      request.bmRequestType = setup_buf[0];
      request.bRequest      = setup_buf[1];
      request.wValue        = (setup_buf[3] << 8) | setup_buf[2];
      request.wIndex        = (setup_buf[5] << 8) | setup_buf[4];
      request.wLength       = (setup_buf[7] << 8) | setup_buf[6];

      usb_setup_length = request.wLength;

      // Setup is always Data 0 so reply with data 1
      USBD->EP[0].CFG |= USBD_CFG_DSQSYNC_Msk;

      // Halt until ready
      USBD->EP[0].CFGP |= USBD_CFGP_CLRRDY_Msk;
      USBD->EP[1].CFGP |= USBD_CFGP_CLRRDY_Msk;

      if (!usb_handle_standard_request(&request))
        usb_control_stall();

      usb_setup_length = -1;
    }

    for (int ep = 0; ep < USB_EP_NUM; ep++)
    {
      int ep_intsts_msk = 0x1ul << (ep + USBD_INTSTS_EPEVT0_Pos);

      if ((busint & ep_intsts_msk) == 0)
        continue; // No event

      int ep_sts_pos;
      if (ep >= 8)
      {
        ep_sts_pos = (ep - 8) * 4 + USBD_EPSTS1_EPSTS8_Pos;
        epint = ((USBD->EPSTS1 & (0xful << ep_sts_pos)) >> ep_sts_pos) & 0xf;
      }
      else
      {
        ep_sts_pos = ep * 4 + USBD_EPSTS0_EPSTS0_Pos;
        epint = ((USBD->EPSTS0 & (0xful << ep_sts_pos)) >> ep_sts_pos) & 0xf;
      }

      switch (epint)
      {
        // Out packet on data 0 or data 1
        case 0b0010: // Data0 Ack
        case 0b0110: // Data1 Ack
        {
          int size = USBD->EP[ep].MXPLD & USBD_MXPLD_MXPLD_Msk;
          uint8_t* ep_buf = (uint8_t *)(USBD_BUF_BASE + USBD->EP[ep].BUFSEG);

          if (epint == 0x0010)
            USBD->EP[ep].CFG |= USBD_CFG_DSQSYNC_Msk; // Data 1
          else
            USBD->EP[ep].CFG &= ~USBD_CFG_DSQSYNC_Msk; // Data 0

          if (ep < 2)
          {
            if (usb_control_recv_callback)
            {
              for (int i = 0; i < size; i++)
                usb_ctrl_out_buf[i] = ep_buf[i];
              USBD->EP[ep].MXPLD = USB_CTRL_EP_SIZE;

              usb_control_recv_callback(usb_ctrl_out_buf, size);
              usb_control_recv_callback = NULL;
              usb_control_send_zlp();
            }
            else
              USBD->EP[ep].CFGP &= ~USBD_CFGP_SSTALL_Msk; // Clear stalls
          }
          else
          {
            if (usb_ep_data[ep] == NULL)
              continue;

            for (int i = 0; i < size; i++)
              usb_ep_data[ep][i] = ep_buf[i];

            usb_recv_callback(ep-1, size);

            // Do not clear, usb_ep_data, just re-enable
            USBD->EP[ep].MXPLD = usb_ep_size[ep];
          }
          break;
        }
        case 0b0000: // In Ack
        {
            if (ep == 0)
              usb_send_callback(ep);
            else
              usb_send_callback(ep - 1);
            break;
        }
        case 0b0001: // In Nack
        case 0b1111: // Iso transfer end
        default:
        {
          break;
        }
      } // end epint switch
    }

    HAL_GPIO_DAP_STATUS_set();
  }


}

