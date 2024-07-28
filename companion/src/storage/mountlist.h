/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

/* mountlist.h -- declarations for list of mounted file systems

   Copyright (C) 1991, 1992, 1998, 2000, 2001, 2002, 2003, 2004, 2005
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, see <http://www.gnu.org/licenses/>.
*/

//////////////////////////////////////////////////////////////////////////////
///@file mountlist.h
/// A function to enumerate the mounting points in the filesystem.
/// Used to display them in fileselectors.
//////////////////////////////////////////////////////////////////////////////

#ifndef MOUNTLIST_H_
# define MOUNTLIST_H_

#if defined __GNUC__
# include <stdbool.h>
#endif

#include <sys/types.h>

/* A mount table entry. */
struct mount_entry
{
  char *me_devname;             /* Device node name, including "/dev/". */
  char *me_mountdir;            /* Mount point directory name. */
  char *me_type;                /* "nfs", "4.2", etc. */
  dev_t me_dev;                 /* Device number of me_mountdir. */
  unsigned int me_dummy : 1;    /* Nonzero for dummy file systems. */
  unsigned int me_remote : 1;   /* Nonzero for remote fileystems. */
  unsigned int me_type_malloced : 1; /* Nonzero if me_type was malloced. */
  struct mount_entry *me_next;
};

struct mount_entry *read_file_system_list (bool need_fs_type);

void free_file_system_list(struct mount_entry * mount_list);

#endif
