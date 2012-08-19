PowerMan: UAV power management
==============================

The functionality of this software includes:

- parameter onfiguration via OPCD using SCL gate "opcd_ctrl"
- acquisition of battery voltage and current
- estimation of remaining battery lifetime
- warns the user using acoustic signals and console messages when the battery voltage is low
- publishing monitoring data via SCL gate called "mon"
- logging the same data to: "$HOME/.ARCADE/PowerMan.log"
- providing a high current circruitry control SCL gate called "ctrl"

PowerMan should be started at boot time using "svctrl --start powerman"
as it monitors the battery voltage and warns the user in case of low battery voltage.
This is useful not only when working in the field, but also in the lab
when working with the system may drain the battery.