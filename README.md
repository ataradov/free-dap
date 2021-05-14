# Free-DAP

This is a free and open implementation of the CMSIS-DAP debugger firmware.

Both SWD and JTAG protocols are supported. However JTAG was not well tested due to lack of
good targets. If you have any issues with it - let me know and I'll try to help.

## Platform requirements

To create a CMSIS-DAP compliant debugger, your platform must:
 * Implement USB HID device able to receive and send arbitrary reports
 * Provide configuration file dap_config.h with definitions for hardware-dependent calls
 * Call dap_init() at the initialization time
 * Call dap_process_request() for every received request and send the response back

## Configuration

For complete list of settings see one of the existing configuration file, they are
pretty obvious.

To configure clock frequency you need to specify two parameters:
  * DAP_CONFIG_DELAY_CONSTANT - clock timing constant. This constant can be determined
    by calling dap_clock_test() with varying parameter value and measuring the frequency
    on the SWCLK pin. Delay constant value is the value of the parameter at which
    output frequency equals to 1 kHz.
  * DAP_CONFIG_FAST_CLOCK - threshold for switching to fast clock routines. This value
    defines the frequency, at which more optimal pin manipulation functions are used.
    This is the frequency produced by dap_clock_test(1) on the SWCLK pin.
    You can also measure maximum achievable frequency on your platform by calling dap_clock_test(0).

Your configuration file will need to define the following pin manipulation functions:

 * DAP_CONFIG_SWCLK_TCK_write()
 * DAP_CONFIG_SWDIO_TMS_write()
 * DAP_CONFIG_TDO_write()
 * DAP_CONFIG_nTRST_write()
 * DAP_CONFIG_nRESET_write()
 * DAP_CONFIG_SWCLK_TCK_read()
 * DAP_CONFIG_SWDIO_TMS_read()
 * DAP_CONFIG_TDI_read()
 * DAP_CONFIG_TDO_read()
 * DAP_CONFIG_nTRST_read()
 * DAP_CONFIG_nRESET_read()
 * DAP_CONFIG_SWCLK_TCK_set()
 * DAP_CONFIG_SWCLK_TCK_clr()
 * DAP_CONFIG_SWDIO_TMS_in()
 * DAP_CONFIG_SWDIO_TMS_out()

Note that all pin manipulation functions are required even if one of the interfaces (JTAG or SWD) is not enabled.

Additionally configuration file must provide basic initialization and control functions:

 * DAP_CONFIG_SETUP()
 * DAP_CONFIG_DISCONNECT()
 * DAP_CONFIG_CONNECT_SWD()
 * DAP_CONFIG_CONNECT_JTAG()
 * DAP_CONFIG_LED()
 * DAP_CONFIG_DELAY()


