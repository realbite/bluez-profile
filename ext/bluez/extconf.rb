#!/usr/bin/env ruby
require 'mkmf'

pkg_config("glib-2.0")
pkg_config("gio-2.0")
pkg_config("gio-unix-2.0")

find_header("gio/gio.h")

find_library('gio-2.0','g_bus_watch_name')


create_makefile('bluez/profile')
