# Free-DAP

This is a free and open implementation of the CMSIS-DAP debugger firmware.

Both SWD and JTAG protocols are supported. However JTAG was not well tested due to lack of
good targets. If you have any issues with it - let me know and I'll try to help.

## Software Changes

This forked branch is for _**Jeff Probe**_ with the following changes:
 * Three status LEDs: Yellow for VCP connection, Orange for DAP, the Red for power indicator
 * The brightness for three status LEDs has been reduced to 5% via PWM as they are too bright to view directly.
 * Implemented ADC power supply sensing, the threshold is 1.1V. If the power is supplied by _**Jeff Probe**_, Power indicator is solid on. If the power is supplied by target device itself, the power indicator is blinking 
 * Implemented Button click and hold detection. Currently the following actions are defined:
    - Single Click: Turn on/off the power to the target board using _**Jeff Probe**_'s internal 3.3v LDO
    - Double Click + Hold: Reboot the device ( when combining with [this bootloader](https://github.com/hlyi/uf2-samdx1), it can easily reflash new version of the FW)
 * Implemented ramping up power to target board. This feature require a hardware modification.
 * Added TMS_DIR control that used by _**Jeff Probe**_
 * Added configuration to support active high reset control as used by _**Jeff Probe**_

## Hardware Modification

The POWER_TGT signal (the control signal to provide 3.3V power to the tagret board) in the original _**Jeff Probe**_ is on PA28. Simply toggling this GPIO pin causes brownout problem on my setup due to in-rush current to the target board. Unfortunately, this pin doesn't support PWM function. The hardware mod is to connect PA01 (pin 2) to PA28 (pin 27) via 300 Ohm resistor ( the resistor is not absolutely required, but was added to protect against bad firmware)

## TODO List
 * Add _reset sensing_. This feature is supported by _**Jeff Probe**_ hardware
 * Use button _double click_ to cycling between the following modes:
   - DAP v1 + DAP v2 + VCP
   - DAP v1 + VCP + HID
   - DAP v2 + VCP + HID
 * The HID can
   - Provide target board voltage readout
   - Provide debugging information from _Free DAP_ firmware itself
   - Control power supply to the target board ( similar to button single click )
   - and more...
 
