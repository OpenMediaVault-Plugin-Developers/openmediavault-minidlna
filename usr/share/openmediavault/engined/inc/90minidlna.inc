<?php
/**
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @copyright Copyright (c) 2013-2022 OpenMediaVault Plugin Developers
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

\OMV\System\LogFileSpec::registerSpecification("minidlna", [
    "filename" => "minidlna.log",
    "filepath" => \OMV\Environment::get("OMV_MINIDLNA_LOG_DIR", "/var/log/minidlna") .
        "/minidlna.log",
    "regex"    => "/^(.*?):\d+:\s+(.*?):\s+(.*)$/",
    "columns"  => [
        "component" => 1,
        "level" => 2,
        "event" => 3
    ]
]);
