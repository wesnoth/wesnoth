/*
 * dirent.h - dirent API for Microsoft Visual Studio
 *
 * Copyright (C) 2006 Toni Ronkko
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * ``Software''), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL TONI RONKKO BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * May 18, 2007, YogiHH
 * Made all the modifications to adapt this file for wesnoth. In particular
 * removing the function definitions (they are provided by wesnoth already)
 * and renaming/adding some of the DIR-struct members.
 *
 * Mar 4, 2007, Toni Ronkko
 * Bug fix: due to the strncpy_s() function this file only compiled in
 * Visual Studio 2005.  Using the new string functions only when the
 * compiler version allows.
 *
 * Nov  2, 2006, Toni Ronkko
 * Major update: removed support for Watcom C, MS-DOS and Turbo C to
 * simplify the file, updated the code to compile cleanly on Visual
 * Studio 2005 with both unicode and multi-byte character strings,
 * removed rewinddir() as it had a bug.
 *
 * Aug 20, 2006, Toni Ronkko
 * Removed all remarks about MSVC 1.0, which is antiqued now.  Simplified
 * comments by removing SGML tags.
 *
 * May 14 2002, Toni Ronkko
 * Embedded the function definitions directly to the header so that no
 * source modules need to be included in the Visual Studio project.  Removed
 * all the dependencies to other projects so that this very header can be
 * used independently.
 *
 * May 28 1998, Toni Ronkko
 * First version.
 */
#ifndef DIRENT_H
#define DIRENT_H

#include <windows.h>
#include <tchar.h>
#include <string.h>
#include <assert.h>


typedef struct dirent {
  /* name of current directory entry (a multi-byte character string) */
  char d_name[MAX_PATH + 1];

  /* file attributes */
  WIN32_FIND_DATA data;

  int d_mode;
} dirent;

typedef struct wdirent {
  /* name of current directory entry (a multi-byte character string) */
  wchar_t d_name[MAX_PATH + 1];

  /* file attributes */
  WIN32_FIND_DATA data;
} wdirent;

typedef struct DIR {
  /* current directory entry */
  dirent dirent;

  /* is there an un-processed entry in current? */
  int cached;

  /* file search handle */
  HANDLE hFind;

  /* search pattern (3 = zero terminator + pattern "\\*") */
  TCHAR patt[MAX_PATH + 3];

  _WIN32_FIND_DATAA find_data;

  LPSTR directory;
} DIR;


static DIR *opendir (const char *dirname);
static struct dirent *readdir (DIR *dirp);
static int closedir (DIR *dirp);

#define _IFMT		0170000
#define _IFREG		0100000
#define _IFDIR		0040000
#define S_ISREG(m)  (((m)&_IFMT) == _IFREG)
#define S_ISDIR(m)  (((m)&_IFMT) == _IFDIR)

/* use the new safe string functions introduced in Visual Studio 2005 */
#if defined(_MSC_VER) && _MSC_VER >= 1400
# define STRNCPY(dest,src,size) strncpy_s((dest),(size),(src),_TRUNCATE)
#else
# define STRNCPY(dest,src,size) strncpy((dest),(src),(size))
#endif


#endif /*DIRENT_H*/
