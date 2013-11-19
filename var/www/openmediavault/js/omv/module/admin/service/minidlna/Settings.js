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
// require("js/omv/WorkspaceManager.js")
// require("js/omv/workspace/form/Panel.js")

/**
 * @class OMV.module.admin.service.minidlna.Settings
 * @derived OMV.workspace.form.Panel
 */
Ext.define("OMV.module.admin.service.minidlna.Settings", {
    extend: "OMV.workspace.form.Panel",

    rpcService: "MiniDlna",
    rpcGetMethod: "getSettings",
    rpcSetMethod: "setSettings",

    plugins: [{
        ptype: "linkedfields",
        correlations: [{
            name: [
                "rescan"
            ],
            conditions: [
                { name: "enable", value: false }
            ],
            properties: "disabled"
        }]
    }],

    getFormItems: function () {
        return [{
            xtype: "fieldset",
            title: _("General settings"),
            fieldDefaults: {
                labelSeparator: ""
            },
            items: [{
                xtype: "checkbox",
                name: "enable",
                fieldLabel: _("Enable"),
                checked: false
            }, {
                xtype: "textfield",
                name: "name",
                value: _("MiniDLNA on OpenMediaVault"),
                fieldLabel: _("Name")
            }, {
                xtype: "numberfield",
                name: "port",
                fieldLabel: _("Port"),
                vtype: "port",
                minValue: 1,
                maxValue: 65535,
                allowDecimals: false,
                allowBlank: false,
                value: 8200
            }, {
                xtype: "checkbox",
                name: "strict",
                fieldLabel: _("Strict DLNA"),
                boxLabel: _("Strictly adhere to DLNA standards."),
                checked: false
            }, {
                xtype: "checkbox",
                name: "tivo",
                fieldLabel: _("TiVo support"),
                checked: false
            },{
                xtype: "button",
                name: "rescan",
                text: _("Rescan"),
                scope: this,
                handler: function() {
                    // Execute RPC.
                    OMV.Rpc.request({
                        scope: this,
                        callback: function(id, success, response) {
                            this.doReload();
                        },
                        relayErrors: false,
                        rpcData: {
                            service: "MiniDlna",
                            method: "doRescan"
                        }
                    });
                }
            },{
                border: false,
                html: "<br />"
            }]
        }];
    }
});

OMV.WorkspaceManager.registerPanel({
    id: "settings",
    path: "/service/minidlna",
    text: _("Settings"),
    position: 10,
    className: "OMV.module.admin.service.minidlna.Settings"
});
