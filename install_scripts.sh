#!/bin/bash

sudo ln -f -s $MOBICOM_PATH/common/svctrl/svctrl.py /usr/local/bin/svctrl
sudo ln -f -s $MOBICOM_SUBPROJECT_PATH/components/logging/core/logger.py /usr/local/bin/core_logger
sudo ln -f -s $MOBICOM_SUBPROJECT_PATH/components/interfaces/opcd_shell/opcd_shell.sh /usr/local/bin/opcd_shell
sudo ln -f -s $MOBICOM_SUBPROJECT_PATH/components/interfaces/core_shell/core_shell.sh /usr/local/bin/core_shell
sudo ln -f -s $MOBICOM_SUBPROJECT_PATH/components/interfaces/icarus_shell/icarus_shell.sh /usr/local/bin/icarus_shell
sudo ln -f -s $MOBICOM_SUBPROJECT_PATH/components/logging/core_debug_writer/convert_core_log.py /usr/local/bin/convert_core_log
