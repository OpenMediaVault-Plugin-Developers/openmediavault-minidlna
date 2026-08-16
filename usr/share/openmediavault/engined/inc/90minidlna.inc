<?php
/**
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @copyright Copyright (c) 2013-2026 openmediavault plugin developers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
require_once("openmediavault/functions.inc");
require_once("openmediavault/globals.inc");

// minidlnad is started with -S (systemd foreground mode), so it no longer
// writes to log_dir/minidlna.log and instead logs to stdout/stderr, which
// systemd captures in the journal under the "minidlnad" syslog identifier.
// Log lines keep their original "component.c:line: level: message" format,
// journald just prepends its own timestamp/hostname/identifier[pid]: prefix.
\OMV\System\LogFileSpec::registerSpecification("minidlna", [
    "command" => "export SYSTEMD_COLORS=0; sudo journalctl --quiet --no-pager ".
        "SYSLOG_IDENTIFIER='minidlnad' --output=short-iso",
    "filename" => "minidlna",
    "regex"    => "/^(\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}[+-]\d{2}:?\d{2})\s+(\S+)\s+".
        "minidlnad\[\d+\]:\s+(.*?):\d+:\s+(.*?):\s+(.*)$/",
    "columns"  => [
        "ts" => [
            "index" => 1,
            "func" => function ($v) {
                return strpdate($v, "Y-m-d*H:i:sT");
            }
        ],
        "date" => [
            "index" => 1,
            "func" => function ($v) {
                return strftime("%c", strpdate($v, "Y-m-d*H:i:sT"));
            }
        ],
        "hostname" => 2,
        "component" => 3,
        "level" => 4,
        "event" => 5
    ]
]);
