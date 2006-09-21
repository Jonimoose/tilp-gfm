/*
  Name: Group File Manager
  Copyright (C) 2006 Tyler Cassidy, Romain Lievin, Kevin Kofler
  04/06/06 16:23 - tilibs.h

  This program is free software you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/*
  This file includes libtifiles, libticables & libticalcs headers and some
  other needed headers.
*/

#ifndef __GFMTILIBS_H__
#define __GFMTILIBS_H__

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <ticonv.h>
#include <tifiles.h>

#ifdef _MSC_VER
#define snprintf	_snprintf
#endif

// Prototypes
int load_tilibs(void);

#endif
