/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2012 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#ifndef ARGS_HPP
#define ARGS_HPP

#include "Compiler.h"
#include "Util/tstring.hpp"

#ifdef _UNICODE
#include "OS/PathName.hpp"
#endif

#include <list>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

class Args {
  std::list<char *> args;
  const char *name, *usage;

#ifdef WIN32
  char *cmdline;
#endif

public:
  Args(int argc, char **argv, const char *_usage)
    :name(argv[0]), usage(_usage) {
    assert(name != NULL);
    assert(usage != NULL);

    std::copy(argv + 1, argv + argc, std::back_inserter(args));

#ifdef WIN32
    cmdline = NULL;
#endif
  }

  Args(const Args &other) = delete;

  Args(Args &&other):name(other.name), usage(other.usage) {
    std::swap(args, other.args);
#ifdef WIN32
    std::swap(cmdline, other.cmdline);
#endif
  }

#ifdef WIN32
  Args(const TCHAR *_cmdline, const char *_usage)
    :usage(_usage) {
    ParseCommandLine(_cmdline);
  }

  ~Args() {
    delete[] cmdline;
  }

  void ParseCommandLine(const char *_cmdline) {
    const char *s = _cmdline;
    char *d0 = new char[strlen(s) + 1];   // start of destination buffer
    char *d = d0;                         // current position in destination buffer
    char *option = d0;

    name = NULL;
    bool in_qoute = false;
    do {
      if (*s == '"')
        in_qoute = !in_qoute;
      else if (*s == '\0' || (!in_qoute && *s == ' ')) {
        // collapse runs of unqouted ' 's to a single '\0'
        if (d > d0 && *(d-1) != '\0') {
          *d++ = '\0';
          // remember potential start position of next option
          option = d;
        }
      } else {
        *d = *s;
        if (option == d) {
          // first quoted blank or non blank character of new option
          if (name == NULL)
            name = option;
          else
            args.push_back(option);
        }
        d++;
      }
    } while (*s++);

    if (name == NULL)
      name = d0;
  }

#ifdef _UNICODE
  void ParseCommandLine(const TCHAR *_cmdline) {
    NarrowPathName convert(_cmdline);
    ParseCommandLine(convert);
  }
#endif
#endif

  Args &operator=(const Args &other) = delete;

  gcc_noreturn
  void UsageError() {
    fprintf(stderr, "Usage: %s %s\n", name, usage);
    exit(EXIT_FAILURE);
  }

  bool IsEmpty() const {
    return args.empty();
  }

  const char *GetNext() {
    assert(!IsEmpty());

    const char *p = args.front();
    args.pop_front();
    return p;
  }

  const char *PeekNext() const {
    return IsEmpty() ? NULL : args.front();
  }

  const char *ExpectNext() {
    if (IsEmpty())
      UsageError();

    return GetNext();
  }

  tstring ExpectNextT() {
    const char *p = ExpectNext();
    assert(p != NULL);

#ifdef _UNICODE
    PathName convert(p);
    return tstring(convert);
#else
    return tstring(p);
#endif
  }

  void ExpectEnd() {
    if (!IsEmpty())
      UsageError();
  }
};

#endif
