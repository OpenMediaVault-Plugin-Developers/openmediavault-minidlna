<?php

/**
 * Copyright (C) 2009-2013 Volker Theile <volker.theile@openmediavault.org>
 * Copyright (C) 2013-2014 OpenMediaVault Plugin Developers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

require_once "openmediavault/config.inc";
require_once "openmediavault/error.inc";
require_once "openmediavault/initscript.inc";
require_once "openmediavault/log.inc";
require_once "openmediavault/module.inc";

class OMVModuleMiniDlna extends OMVModuleServiceAbstract implements
    OMVINotifyListener,
    OMVIModuleServiceStatus
{
    public function getName()
    {
        return "minidlna";
    }

    private function getPluginName()
    {
        return strtolower($this->getName());
    }

    private function getXPath()
    {
        return sprintf("//services/%s", $this->getPluginName());
    }

    private function getSharesXPath()
    {
        return sprintf("%s/shares", $this->getXPath());
    }

    private function getShareXPath()
    {
        return sprintf("%s/share", $this->getSharesXPath());
    }

    private function getShareXPathByUuid($uuid)
    {
        return sprintf(
            "%s[uuid='%s']",
            $this->getShareXPath(),
            $uuid
        );
    }

    private function getEventMessagePath()
    {
        return sprintf("org.openmediavault.services.%s", $this->getPluginName());
    }

    private function getShareEventMessagePath()
    {
        return sprintf("%s.shares.share", $this->getEventMessagePath());
    }

    private function getSharedFolderEventMessagePath()
    {
        return "org.openmediavault.system.shares.sharedfolder";
    }

    public function applyConfig()
    {
        global $xmlConfig;

        $cmd = sprintf("omv-mkconf %s 2>&1", $this->getName());

        if (0 !== $this->exec($cmd, $output)) {
            throw new OMVException(
                OMVErrorMsg::E_EXEC_FAILED,
                $cmd,
                implode("\n", $output)
            );
        }

        $object = $xmlConfig->get($this->getXPath());

        if (is_null($object)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getXPath()
            );
        }

        if (!boolval($object["enable"])) {
            return;
        }

        $initd = new OMVSysVInitScript($this->getName());
        $initd->invoke("force-reload");
    }

    public function startService()
    {
        global $xmlConfig;

        $object = $xmlConfig->get($this->getXPath());

        if (is_null($object)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getXPath()
            );
        }

        if (!boolval($object["enable"])) {
            return;
        }

        $initd = new OMVSysVInitScript($this->getName(), true);
        $initd->exec();
    }

    public function stopService()
    {
        $initd = new OMVSysVInitScript($this->getName());
        $initd->exec();
    }

    public function getStatus()
    {
        global $xmlConfig;

        $object = $xmlConfig->get($this->getXPath());

        if (is_null($object)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getXPath()
            );
        }

        $initd = new OMVSysVInitScript($this->getName());

        return array(
            "name"    => $this->getName(),
            "title"   => gettext("MiniDLNA"),
            "enabled" => boolval($object["enable"]),
            "running" => ($initd->status() == 0)
        );
    }

    final public function onModifySharedFolder($type, $path, $object)
    {
        global $xmlConfig;

        $xPath = $this->getShareXPathByUuid($object["uuid"]);

        if ($xmlConfig->exists($xPath))
        {
            $this->setDirty();
        }
    }

    public function bindListeners(OMVNotifyDispatcher $dispatcher)
    {
        $dispatcher->addListener(
            OMV_NOTIFY_MODIFY,
            $this->getEventMessagePath(),
            array($this, "setDirty")
        );

        $dispatcher->addListener(
            OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
            $this->getShareEventMessagePath(),
            array($this, "setDirty")
        );

        $dispatcher->addListener(
            OMV_NOTIFY_MODIFY,
            $this->getSharedFolderEventMessagePath(),
            array($this, "onModifySharedFolder")
        );
    }
}

// Register module.
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new OMVModuleMiniDlna());

// Register new logfile type
OMVLogFile::registerType("minidlna", array(
    "filename" => "minidlna.log",
    "filepath" => "/var/log/minidlna.log",
    "regex"    => "/^\[(.*?)\]\s+(.*?)\..*?:\d+:\s+.*?:\s+(.*)$/",
    "columns"  => array(
        "date"      => array(
            "index" => 1,
            "func"  => create_function(
                "\$v",
                "return strpdate(\$v,\"Y/n/j G:i:s\");"
            )
        ),
        "component" => 2,
        "event"     => 3
    )
));
