/*
 *
 *  D-Bus++ - C++ bindings for D-Bus
 *
 *  Copyright (C) 2005-2007  Paolo Durante <shackan@gmail.com>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <dbus-c++/debug.h>
#include <dbus-c++/property.h>

#include <dbus-c++/introspection.h>

using namespace DBus;

static const char *properties_name = "org.freedesktop.DBus.Properties";

PropertiesAdaptor::PropertiesAdaptor()
  : InterfaceAdaptor(properties_name)
{
  register_method(PropertiesAdaptor, Get, Get);
  register_method(PropertiesAdaptor, Set, Set);
  register_method(PropertiesAdaptor, GetAll, GetAll);
}

Message PropertiesAdaptor::Get(const CallMessage &call)
{
  MessageIter ri = call.reader();

  std::string iface_name;
  std::string property_name;

  ri >> iface_name >> property_name;

  debug_log("requesting property %s on interface %s", property_name.c_str(), iface_name.c_str());

  InterfaceAdaptor *interface = (InterfaceAdaptor *) find_interface(iface_name);

  if (!interface)
    throw ErrorFailed("requested interface not found");

  Variant *value = interface->get_property(property_name);

  if (!value)
    throw ErrorFailed("requested property not found");

  if (value->signature().empty())
    throw ErrorFailed("requested property has not been initialized");

  on_get_property(*interface, property_name, *value);

  ReturnMessage reply(call);

  MessageIter wi = reply.writer();

  wi << *value;
  return reply;
}

Message PropertiesAdaptor::Set(const CallMessage &call)
{
  MessageIter ri = call.reader();

  std::string iface_name;
  std::string property_name;
  Variant value;

  ri >> iface_name >> property_name >> value;

  InterfaceAdaptor *interface = (InterfaceAdaptor *) find_interface(iface_name);

  if (!interface)
    throw ErrorFailed("requested interface not found");

  on_set_property(*interface, property_name, value);

  interface->set_property(property_name, value);

  ReturnMessage reply(call);

  return reply;
}

Message PropertiesAdaptor::GetAll(const CallMessage &call)
{
  MessageIter ri = call.reader();

  std::string iface_name;
  std::string property_name;
  Variant value;

  ri >> iface_name;

  InterfaceAdaptor *interface = (InterfaceAdaptor *) find_interface(iface_name);

  if (!interface)
    throw ErrorFailed("requested interface not found");

  PropertyDict *properties;
  properties = interface->get_all_properties();

  ReturnMessage reply(call);

  MessageIter wi = reply.writer();

  wi << *properties;
  delete properties;
  return reply;
}

void PropertiesAdaptor::PropertiesChanged(const std::string& interface,
                                          const std::map<std::string, ::DBus::Variant>& changed_properties,
                                          const std::vector<std::string>& invalidated_properties)
{
  ::DBus::SignalMessage sig("PropertiesChanged");
  ::DBus::MessageIter wi = sig.writer();
  wi << interface;
  wi << changed_properties;
  wi << invalidated_properties;
  emit_signal(sig);
}

IntrospectedInterface *PropertiesAdaptor::introspect() const
{
  static IntrospectedArgument Get_args[] =
  {
    { "interface_name", "s", true },
    { "property_name", "s", true },
    { "value", "v", false },
    { 0, 0, 0 }
  };
  static IntrospectedArgument Set_args[] =
  {
    { "interface_name", "s", true },
    { "property_name", "s", true },
    { "value", "v", true },
    { 0, 0, 0 }
  };
  static IntrospectedArgument GetAll_args[] =
  {
    { "interface_name", "s", true },
    { "properties", "a{sv}", false },
    { 0, 0, 0 }
  };
  static IntrospectedMethod Properties_methods[] =
  {
    { "Get", Get_args },
    { "Set", Set_args },
    { "GetAll", GetAll_args },
    { 0, 0 }
  };
  static IntrospectedMethod Properties_signals[] =
  {
    { 0, 0 }
  };
  static IntrospectedProperty Properties_properties[] =
  {
    { 0, 0, 0, 0 }
  };
  static IntrospectedInterface Properties_interface =
  {
    properties_name,
    Properties_methods,
    Properties_signals,
    Properties_properties
  };
  return &Properties_interface;
}

PropertiesProxy::PropertiesProxy()
  : InterfaceProxy(properties_name)
{
  connect_signal(PropertiesProxy, PropertiesChanged, _PropertiesChanged_stub);
}

Variant PropertiesProxy::Get(const std::string &iface, const std::string &property)
{
  CallMessage call;
  call.member("Get");
  call.interface("org.freedesktop.DBus.Properties");
  MessageIter wi = call.writer();
  wi << iface;
  wi << property;
  Message ret = this->invoke_method (call);
  MessageIter ri = ret.reader ();
  Variant argout;
  ri >> argout;
  return argout;
}

void PropertiesProxy::Set(const std::string &iface, const std::string &property, const Variant &value)
{
  CallMessage call;
  call.member("Set");
  call.interface("org.freedesktop.DBus.Properties");
  MessageIter wi = call.writer();
  wi << iface;
  wi << property;
  wi << value;
  Message ret = this->invoke_method (call);
}

std::map< std::string, ::DBus::Variant > PropertiesProxy::GetAll(const std::string &iface)
{
  CallMessage call;
  call.member("GetAll");
  call.interface("org.freedesktop.DBus.Properties");
  MessageIter wi = call.writer();
  wi << iface;
  Message ret = this->invoke_method (call);
  MessageIter ri = ret.reader ();
  std::map< std::string, ::DBus::Variant > argout;
  ri >> argout;
  return argout;
}

void PropertiesProxy::_PropertiesChanged_stub(const ::DBus::SignalMessage &sig)
{
  ::DBus::MessageIter ri = sig.reader();

  std::string iface;
  ri >> iface;
  std::map< std::string, ::DBus::Variant > changed_properties;
  ri >> changed_properties;
  std::vector< std::string > invalidated_properties;
  ri >> invalidated_properties;
  PropertiesChanged(iface, changed_properties, invalidated_properties);
}
