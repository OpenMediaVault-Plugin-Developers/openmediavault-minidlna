<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Aaron Murray <aaron@omv-extras.org>
 * @copyright Copyright (c) 2013 Aaron Murray
 *
 * OpenMediaVault is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * OpenMediaVault is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenMediaVault. If not, see <http://www.gnu.org/licenses/>.
 */
require_once("openmediavault/module.inc");
require_once("openmediavault/config.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/initscript.inc");
require_once("openmediavault/log.inc");

class OMVModuleMiniDlna extends OMVModuleServiceAbstract
  implements OMVINotifyListener, OMVIModuleServiceStatus {
	/**
	 * Get the module name.
	 */
	public function getName() {
		return "minidlna";
	}

	/**
	 * Generate the configuration.
	 * @throw E_EXEC_FAILED
	 */
	public function applyConfig() {
		global $xmlConfig;

		$cmd = "omv-mkconf minidlna 2>&1";
		if(0 !== $this->exec($cmd, $output)) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED, $cmd, implode("\n", $output));
		}

		$xpath = "//services/minidlna";
		$object = $xmlConfig->get($xpath);

		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $xpath);
		}
		if(FALSE === boolval($object['enable']))
			return;	
		
		$cmd = "sudo /etc/init.d/minidlna force-reload 2>&1";
		if(0 !== $this->exec($cmd, $output)) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED, $cmd, implode("\n", $output));
		}
	}

	/**
	 * Stop the managed service.
	 */
	public function stopService() {
		$initd = new OMVSysVInitScript("minidlna", FALSE);
		$initd->exec();

		$cmd = "sudo /etc/init.d/minidlna stop 2>&1";
		if(0 !== $this->exec($cmd, $output)) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED, $cmd, implode("\n", $output));
		}		
	}

	/**
	 * Start the managed service.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	public function startService() {
		global $xmlConfig;
		$xpath = "//services/minidlna";
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		if(FALSE === boolval($object['enable']))
			return;
		$initd = new OMVSysVInitScript("minidlna", TRUE);
		$initd->exec();
	}

	/**
	 * Get the status of the managed service.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	public function getStatus() {
		global $xmlConfig;
		$xpath = "//services/minidlna";
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		$initd = new OMVSysVInitScript("minidlna");
		return array(
			"name" => $this->getName(),
			"title" => gettext("MiniDLNA"),
			"enabled" => boolval($object['enable']),
			"running" => ($initd->status() == 0)
		);
	}

	/**
	 * Helper function to find out whether the given shared folder
	 * configuration object is used. If it is used, then mark the
	 * module as dirty.
	 * @param type The event message type.
	 * @param path The event message path.
	 * @param object The configuration object.
	 */
	final public function onSharedFolder($type, $path, $object) {
		global $xmlConfig;
		$xpath = sprintf("//services/minidlna/shares/share[sharedfolderref='%s']",
		  $object['uuid']);
		if(TRUE === $xmlConfig->exists($xpath))
			$this->setDirty();
		switch($path) {
		case "org.openmediavault.system.shares.sharedfolder":
			$this->setDirtyByName("fstab");
			break;
		default:
			break;
		}
	}

	/**
	 * Bind listeners.
	 */
	function bindListeners(OMVNotifyDispatcher $dispatcher) {
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.minidlna",
		  array($this, "setDirty"));
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
		  "org.openmediavault.services.minidlna.shares.share",
		  array($this, "setDirty"));
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.shares.sharedfolder",
		  array($this, "onSharedFolder"));
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
			"func"  => create_function('$v', 'return strpdate($v,"Y/n/j G:i:s");')
		),
		"component" => 2,
		"event"     => 3
	)
));
?>
