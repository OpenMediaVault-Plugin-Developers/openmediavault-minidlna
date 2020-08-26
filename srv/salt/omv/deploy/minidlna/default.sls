# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    OpenMediaVault Plugin Developers <plugins@omv-extras.org>
# @copyright Copyright (c) 2019-2020 OpenMediaVault Plugin Developers
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

{% set config = salt['omv_conf.get']('conf.service.minidlna') %}
{% set dbdir = salt['pillar.get']('default:OMV_MINIDLNA_DB_DIR', '/var/cache/minidlna') -%}
{% set logdir = salt['pillar.get']('default:OMV_MINIDLNA_LOG_DIR', '/var/log') -%}

{% if config.enable | to_bool %}

configure_minidlna:
  file.managed:
    - name: "/etc/minidlna.conf"
    - source:
      - salt://{{ tpldir }}/files/etc-minidlna_conf.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 644

start_minidlna_service:
  service.running:
    - name: minidlna
    - enable: True
    - watch:
      - file: configure_minidlna

{% else %}

stop_minidlna_service:
  service.dead:
    - name: minidlna
    - enable: False

{% endif %}
