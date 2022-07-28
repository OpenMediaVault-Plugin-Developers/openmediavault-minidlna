#!/bin/sh
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @author    OpenMediaVault Plugin Developers <plugins@omv-extras.org>
# @copyright Copyright (c) 2009-2013 Volker Theile
# @copyright Copyright (c) 2013-2020 OpenMediaVault Plugin Developers
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

set -e

. /usr/share/openmediavault/scripts/helper-functions

OMV_MINIDLNA_DB_DIR=${OMV_MINIDLNA_DB_DIR:-"/var/cache/minidlna"}
OMV_MINIDLNA_LOG_DIR=${OMV_MINIDLNA_LOG_DIR:-"/var/log/minidlna"}

SERVICE_XPATH_NAME="minidlna"
SERVICE_XPATH="/config/services/${SERVICE_XPATH_NAME}"

if ! omv_config_exists "${SERVICE_XPATH}"; then
    omv_config_add_node "/config/services" "${SERVICE_XPATH_NAME}"
    omv_config_add_key "${SERVICE_XPATH}" "enable" "0"
    omv_config_add_key "${SERVICE_XPATH}" "name" "MiniDLNA Server on OpenMediaVault"
    omv_config_add_key "${SERVICE_XPATH}" "netinterface" ""
    omv_config_add_key "${SERVICE_XPATH}" "port" "8200"
    omv_config_add_key "${SERVICE_XPATH}" "strict" "0"
    omv_config_add_key "${SERVICE_XPATH}" "tivo" "0"
    omv_config_add_key "${SERVICE_XPATH}" "widelinks" "0"
    omv_config_add_key "${SERVICE_XPATH}" "rootcontainer" "."
    omv_config_add_node "${SERVICE_XPATH}" "shares"
    omv_config_add_key "${SERVICE_XPATH}" "loglevel" "error"
    omv_config_add_key "${SERVICE_XPATH}" "extraoptions" ""
fi

echo "Add ${SERVICE_XPATH_NAME} user to group: users"
usermod -G users ${SERVICE_XPATH_NAME}

# set max user watches for inotify
echo "fs.inotify.max_user_watches=100000" > /etc/sysctl.d/90openmediavault-minidlna.conf
sysctl -p /etc/sysctl.d/90openmediavault-minidlna.conf

# create and set permissions for database directory
if [ ! -d ${OMV_MINIDLNA_DB_DIR} ]; then
    mkdir -p ${OMV_MINIDLNA_DB_DIR}
fi
chown -R minidlna:minidlna ${OMV_MINIDLNA_DB_DIR}

# create and set permissions for log directory
if [ ! -d ${OMV_MINIDLNA_LOG_DIR} ]; then
    mkdir -p ${OMV_MINIDLNA_LOG_DIR}
fi
if [ ! "${OMV_MINIDLNA_LOG_DIR}" = "/var/log" ]; then
    chown minidlna:minidlna ${OMV_MINIDLNA_LOG_DIR}
fi
if [ -f "${OMV_MINIDLNA_LOG_DIR}/minidlna.log" ]; then
    chown minidlna:minidlna ${OMV_MINIDLNA_LOG_DIR}/minidlna.log
fi

exit 0
