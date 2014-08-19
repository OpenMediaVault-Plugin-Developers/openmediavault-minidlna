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
require_once "openmediavault/notify.inc";
require_once "openmediavault/object.inc";
require_once "openmediavault/rpcservice.inc";
require_once "openmediavault/util.inc";

class OMVRpcServiceMiniDlna extends OMVRpcServiceAbstract
{
    public function getName()
    {
        return "MiniDLNA";
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

    private function getSharedFolderXPathByUuid($uuid)
    {
        return sprintf("//system/shares/sharedfolder[uuid='%s']", $uuid);
    }

    private function getEventMessagePath()
    {
        return sprintf("org.openmediavault.services.%s", $this->getPluginName());
    }

    private function getShareEventMessagePath()
    {
        return sprintf("%s.shares.share", $this->getEventMessagePath());
    }

    public function initialize()
    {
        $this->registerMethod("getSettings");
        $this->registerMethod("setSettings");
        $this->registerMethod("getShareList");
        $this->registerMethod("getShare");
        $this->registerMethod("setShare");
        $this->registerMethod("deleteShare");
        $this->registerMethod("doRescan");
    }

    public function getSettings($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext(
            $context,
            array( "role" => OMV_ROLE_ADMINISTRATOR )
        );

        // Get the configuration object.
        $object = $xmlConfig->get($this->getXPath());

        if (is_null($object))
        {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getXPath()
            );
        }

        // Modify the result object.
        $object["enable"] = boolval($object["enable"]);
        $object["port"]   = intval($object["port"]);
        $object["strict"] = intval($object["strict"]);
        $object["tivo"]   = intval($object["tivo"]);

        // Remove useless fields from the result object.
        unset($object["shares"]);

        return $object;
    }

    public function setSettings($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext(
            $context,
            array( "role" => OMV_ROLE_ADMINISTRATOR )
        );

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams(
            $params,
            '{
                "type"        : "object",
                "properties"  : {
                    "enable" : { "type" : "boolean" },
                    "name"   : { "type" : "string"},
                    "port"   : { "type" : "integer", "minimum" : 1, "maximum" : 65535 },
                    "strict" : { "type" : "boolean"},
                    "tivo"   : { "type" : "boolean"}
                }
            }'
        );

        // Get the existing configuration object.
        $oldObject = $xmlConfig->get($this->getXPath());

        if (is_null($oldObject))
        {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getXPath()
            );
        }

        // Update the configuration object.
        $object = array(
            "enable" => array_boolval($params, "enable"),
            "name"   => $params["name"],
            "port"   => $params["port"],
            "strict" => array_boolval($params, "strict"),
            "tivo"   => array_boolval($params, "tivo"),
            "shares" => isset($oldObject["shares"]) ? $oldObject["shares"] : array(),
        );

        if ($xmlConfig->replace($this->getXPath(), $object) === false)
        {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED
            );
        }

        // Notify configuration changes.
        $dispatcher = &OMVNotifyDispatcher::getInstance();

        $dispatcher->notify(
            OMV_NOTIFY_MODIFY,
            $this->getEventMessagePath(),
            $object
        );

        // Return the configuration object.
        return $object;
    }

    public function getShareList($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext(
            $context,
            array( "role" => OMV_ROLE_ADMINISTRATOR )
        );

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams(
            $params,
            '{
                "type"       : "object",
                "properties" : {
                    "start"     : { "type" : "integer" },
                    "limit"     : { ' . $GLOBALS["OMV_JSONSCHEMA_COUNTFIELD"] . ' },
                    "sortfield" : { ' . $GLOBALS["OMV_JSONSCHEMA_SORTFIELD"] . ' },
                    "sortdir"   : { ' . $GLOBALS["OMV_JSONSCHEMA_SORTDIR"] . ' }
                }
            }'
        );

        // Get the configuration object.
        $shares = $xmlConfig->getList($this->getShareXPath());

        if (is_null($shares)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getShareXPath()
            );
        }

        // Add additional share informations.
        foreach ($shares as &$share)
        {
            $share["sharedfoldername"] = gettext("n/a");

            // Get the shared folder configuration object.
            $xPath = $this->getSharedFolderXPathByUuid($share["sharedfolderref"]);

            $sharedFolder = $xmlConfig->get($xPath);

            if (!is_null($sharedFolder) && !empty($sharedFolder))
            {
                $share["sharedfoldername"] = $sharedFolder["name"];
            }
        }

        // Filter the result.
        return $this->applyFilter(
            $shares,
            $params["start"],
            $params["limit"],
            $params["sortfield"],
            $params["sortdir"]
        );
    }

    public function getShare($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams(
            $params,
            '{
                "type"       : "object",
                "properties" : {
                    "uuid" : { ' . $GLOBALS["OMV_JSONSCHEMA_UUID"] . ' }
                }
            }'
        );

        // Get the configuration object.
        $xPath = $this->getShareXPathByUuid($params["uuid"]);
        $share = $xmlConfig->get($xPath);

        if (is_null($share))
        {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $xPath
            );
        }

        return $share;
    }

    public function setShare($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext(
            $context,
            array( "role" => OMV_ROLE_ADMINISTRATOR )
        );

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams(
            $params,
            '{
                "type"       : "object",
                "properties" : {
                    "uuid"            : { ' . $GLOBALS["OMV_JSONSCHEMA_UUID_UNDEFINED"] . ' },
                    "sharedfolderref" : { ' . $GLOBALS["OMV_JSONSCHEMA_UUID"] . ' },
                    "mtype"           : { "type" : "string" }
                }
            }'
        );

        // Prepare the configuration object.
        $share = array(
            "uuid"            => ($params["uuid"] == $GLOBALS["OMV_UUID_UNDEFINED"]) ?
                OMVUtil::uuid() : $params["uuid"],
            "sharedfolderref" => $params["sharedfolderref"],
            "mtype"           => $params["mtype"]
        );

        // Set the configuration object.
        $success = false;

        if ($params["uuid"] == $GLOBALS["OMV_UUID_UNDEFINED"])
        {
            // Check uniqueness. A shared folder can only be configured
            // once for MiniDlna.
            $xPath = sprintf(
                "%s[sharedfolderref='%s']",
                $this->getShareXPath(),
                $params["sharedfolderref"]
            );

            if ($xmlConfig->exists($xPath) === true)
            {
                throw new OMVException(
                    OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                    gettext("The shared folder is already used by another share.")
                );
            }

            // Get the shared folder object.
            $xPath = $this->getSharedFolderXPathByUuid($params["sharedfolderref"]);
            $sharedFolder = $xmlConfig->get($xPath);

            if (is_null($sharedFolder))
            {
                throw new OMVException(
                    OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                    $xPath
                );
            }

            // Append the configuration object.
            $success = $xmlConfig->set($this->getSharesXPath(), array("share" => $share));
        }
        else
        {
            // Update the existing configuration object.
            $success = $xmlConfig->replace($this->getShareXPathByUuid($params["uuid"]),$share);
        }

        if (!$success)
        {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED
            );
        }

        // Notify configuration changes.
        $dispatcher = &OMVNotifyDispatcher::getInstance();

        $dispatcher->notify(
            ($params["uuid"] == $GLOBALS["OMV_UUID_UNDEFINED"]) ? OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
            $this->getShareEventMessagePath(),
            $share
        );

        // Return the configuration object.
        return $share;
    }

    public function deleteShare($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams(
            $params,
            '{
                "type"       : "object",
                "properties" : {
                    "uuid" : { ' . $GLOBALS["OMV_JSONSCHEMA_UUID"] . ' }
                }
            }'
        );

        // Delete the configuration object.
        $share = $this->deleteConfigObjectByPath(
            $this->getShareXPathByUuid($params["uuid"]),
            $this->getShareEventMessagePath()
        );

        // Return the deleted configuration object.
        return $share;
    }

    public function doRescan($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext(
            $context,
            array( "role" => OMV_ROLE_ADMINISTRATOR )
        );

        $object = $xmlConfig->get($this->getXPath());

        if (is_null($object))
        {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getXPath()
            );
        }

        if (!boolval($object["enable"]))
        {
            return;
        }

        $initd = new OMVSysVInitScript($this->getPluginName());
        $initd->invoke("force-reload");
    }
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceMiniDlna());
