PowerMan: UAV power management
==============================

The functionality of this software includes:

- acquisition, estimation, logging and publishing of remaining battery capacity
- acoustic and textual low battery warning user messages
- control interface for enabling high current power circuitry

PowerMan is typically started at boot time using "svctrl --start powerman",
which also starts OPCD as a dependency.

It monitors the battery voltage and warns the user in case of low battery voltage.
This is useful not only when working in the field, but also in the lab
to prevent harmful low voltage levels in the battery cells.
