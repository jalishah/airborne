
PowerMan: UAV power management
==============================

This functionality of this software includes:

- parameter onfiguration via OPCD using SCL gate "opcd_ctrl"
- acquisition of battery voltage and current
- estimation of remaining battery lifetime
- publishing monitoring data via SCL gate called "mon"
- logging the same data to: "$HOME/.ARCADE/PowerMan.log"
- providing a high current circruitry control SCL gate called "ctrl"
