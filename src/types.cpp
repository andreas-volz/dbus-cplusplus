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

#include <dbus-c++/types.h>
#include <dbus-c++/object.h>
#include <dbus/dbus.h>
#include <cstdlib>
#include <stdarg.h>

#if !HAVE_WIN32
#include <errno.h>
#include <unistd.h>
#endif

#include "message_p.h"
#include "internalerror.h"

namespace DBus {

UnixFD::UnixFD()
  : _fd(-1)
{
}

UnixFD::UnixFD(int fd)
  : _fd(fd)
{
#if HAVE_WIN32
  if (_fd != -1)
    throw ErrorInvalidArgs("Unix FDs not supported under windows");
#endif
}

UnixFD::~UnixFD()
{
#if !HAVE_WIN32
  if (_fd != -1)
    close (_fd);
#endif
}

UnixFD::UnixFD(const UnixFD &o)
{
  _fd = -1;
#if !HAVE_WIN32
  if (o._fd != -1) {
    _fd = dup(o._fd);
    if (_fd == -1)
      throw Error("dup:errno", toString(errno).c_str());
  }
#endif
}

UnixFD &UnixFD::operator=(const UnixFD &o)
{
  if (this == &o)
    return *this;

#if !HAVE_WIN32
  if (_fd != -1) {
    close (_fd);
    _fd = -1;
  }
  if (o._fd != -1) {
    _fd = dup(o._fd);
    if (_fd == -1)
      throw Error("dup:errno", toString(errno).c_str());
  }
#endif
  return *this;
}

Variant::Variant()
  : _msg(CallMessage()) // dummy message used as temporary storage for variant data
{
}

Variant::Variant(MessageIter &it)
  : _msg(CallMessage())
{
  MessageIter vi = it.recurse();
  MessageIter mi = _msg.writer();
  vi.copy_data(mi);
}

Variant &Variant::operator = (const Variant &v)
{
  if (&v != this)
  {
    _msg = v._msg;
  }
  return *this;
}

void Variant::clear()
{
  CallMessage empty;
  _msg = empty;
}

const Signature Variant::signature() const
{
  char *sigbuf = reader().signature();

  Signature signature = sigbuf;

  free(sigbuf);

  return signature;
}

MessageIter &operator << (MessageIter &iter, const Variant &val)
{
  const Signature sig = val.signature();

  MessageIter rit = val.reader();
  MessageIter wit = iter.new_variant(sig.c_str());

  rit.copy_data(wit);

  iter.close_container(wit);

  return iter;
}

MessageIter &operator >> (MessageIter &iter, Variant &val)
{
  if (iter.type() != DBUS_TYPE_VARIANT)
    throw ErrorInvalidArgs("variant type expected");

  val.clear();

  MessageIter vit = iter.recurse();
  MessageIter mit = val.writer();

  vit.copy_data(mit);

  return ++iter;
}

} /* namespace DBus */
