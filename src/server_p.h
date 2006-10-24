/*
 *
 *  D-Bus++ - C++ bindings for DBus
 *
 *  Copyright (C) 2005-2006  Paolo Durante <shackan@gmail.com>
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#ifndef __DBUSXX_SERVER_P_H
#define __DBUSXX_SERVER_P_H

#include <dbus-c++/server.h>
#include <dbus-c++/dispatcher.h>
#include <dbus-c++/refptr_impl.h>

#include <dbus/dbus.h>

namespace DBus {

struct Server::Private 
{
	DBusServer* server;

	Dispatcher* dispatcher;

	ConnectionList connections;

	Private( DBusServer* );

	~Private();

	static void on_new_conn_cb( DBusServer* server, DBusConnection* conn, void* data );
};

} /* namespace DBus */

#endif//__DBUSXX_SERVER_P_H
