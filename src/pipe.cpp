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

/* Project */
#include <dbus-c++/pipe.h>
#include <dbus-c++/util.h>
#include <dbus-c++/error.h>

/* STD */
#include <unistd.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <errno.h>
#include <cassert>

using namespace DBus;
using namespace std;

Pipe::Pipe(void(*handler)(const void *data, void *buffer, unsigned int nbyte), const void *data) :
  _handler(handler),
  _fd_write(0),
  _fd_read(0),
  _data(data)
{
  int fd[2];

  if (pipe(fd) == 0)
  {
    _fd_read = fd[0];
    _fd_write = fd[1];
    fcntl(_fd_read, F_SETFL, O_NONBLOCK);
  }
  else
  {
    throw Error("PipeError:errno", toString(errno).c_str());
  }
}


// this will block if the pipe is full.
void Pipe::write(const void *buffer, unsigned int nbytes)
{
    ssize_t rc;
    ssize_t bytes_written = 0;

    // Write size handling EINTR and partial writes
    while (bytes_written < sizeof(nbytes)) {
        rc = ::write(_fd_write,
                     ((char*)static_cast <const unsigned int *>(&nbytes)) + bytes_written,
                     sizeof(nbytes)-bytes_written);
        if (-1 == rc) {
            if (errno != EINTR) {
                nbytes = 0;
                return;
            }
            continue;
        }
        bytes_written += rc;
    }

    //Write payload handling EINTR and partial writes
    bytes_written = 0;
    while (bytes_written < sizeof(nbytes)) {
        rc = ::write(_fd_write,
                     static_cast <const char *>(buffer) + bytes_written,
                     nbytes-bytes_written);
        if (-1 == rc) {
            if (errno != EINTR) {
                nbytes = 0;
                return;
            }
            continue;
        }
        bytes_written += rc;
    }

  // ...then write the real data

}

ssize_t Pipe::read(void *buffer, unsigned int &nbytes)
{

    // Read size handling EINTR and partial reads. Note: Since the
    // size is written in a single write() and this needs to return
    // without blocking if no data is waiting, we can leave the "size"
    // read as non-blocking.
    nbytes = 0;
    ssize_t rc;
    ssize_t size = 0;
    while (size < sizeof(unsigned int)) {
        rc = ::read(_fd_read, ((char*)&nbytes) + size, sizeof(unsigned int) - size);
        if (-1 == rc) {
            if (errno != EINTR) {
                nbytes = 0;
                // wait for eagain since the entire size should be showing up ASAP since
                // it is written in a single call.
                if (errno != EAGAIN) {
                    debug_log("Unexpected errno of %i when reading fd %i\n", errno, _fd_read);
                    return -1;
                }
                return 0;
            }
            continue;
        }
        size += rc;
    }


    // Change payload read to BLOCKING read so we don't fastloop and
    // burn CPU waiting for the complete payload of the data to show up.
    int old_flags = fcntl(_fd_read, F_GETFL);
    if (old_flags == -1) {
        // trouble
        debug_log("%s fcntl() failed with errno %i for FD %i\n",
                __FUNCTION__, errno, _fd_read);
    } else {
        // make blocking if was non-blocking
        if (old_flags & O_NONBLOCK) {
            fcntl(_fd_read, F_SETFL, old_flags & ~O_NONBLOCK);
        }
    }

    // Read payload handling EINTR and partial reads
    size = 0;
    while (size < nbytes ) {
        rc = ::read(_fd_read, ((char*)buffer) + size, nbytes - size);
        if (-1 == rc) {
            if (errno != EINTR) {
                nbytes = 0;
                size = 0;
                debug_log("%s read() of FD %i failed unexpected with errno %i", __FUNCTION__, _fd_read, errno);
                goto cleanup;
            }
            continue;
        }
        size += rc;
    }

cleanup:
    if ((old_flags != -1) && (old_flags & O_NONBLOCK)) {
        //restore flags if needed
        fcntl(_fd_read, F_SETFL, old_flags);
    }

    return size;
}

void Pipe::signal()
{
  // Write a "message" w/size prefix of size 0
  write("", 0);
}
