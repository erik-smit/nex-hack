//
// Sony NEX camera firmware toolbox
//
// written (reverse-engineered) by Paul Bartholomew, released under the GPL
// (originally based on "pr.exe" from nex-hack.info, with much more since then)
//
// Copyright (C) 2012-2013, nex-hack project
//
// This file "config.h" is part of fwtool (http://www.nex-hack.info)
//
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//

#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#if	defined(__POCC__)	// POCC lib use '_function' for POSIX 'function'

	#pragma warn(disable : 2030) //'=' used in conditional expression.

	// POCC io.h
	extern	int	_unlink(const char *name);
	#define	unlink(fname)	_unlink(fname)
	extern	int	_mkdir(const char *name);
	#define	mkdir(path, mode)	_mkdir(path)
	// POCC sys/types.h
	#ifndef _INO_T_DEFINED
	#define _INO_T_DEFINED
		typedef unsigned short _ino_t;
		#define ino_t	_ino_t
	#endif /* _INO_T_DEFINED */
	// POCC dirent.h
	typedef struct _dirent_dir _DIR;
	#define	DIR	_DIR
	struct _dirent {
		_ino_t d_ino;
		unsigned char d_type;  /* non-standard field for dummies */
		char d_name[260];
	};
	#define	dirent	_dirent
	extern	DIR *_opendir(const char *name);
	#define	opendir(path)	_opendir(path)
	extern	struct dirent *_readdir(DIR *);
	#define	readdir(dir)	_readdir(dir)
	extern	int _closedir(DIR *);
	#define closedir(dir)	_closedir(dir)
#else
	#include <unistd.h>
	#include <sys/types.h>
	#include <dirent.h>
#endif // __POCC__


#define	_TMP_IO_BUFLEN		4096
#define	_TMP_FNAME_BUFLEN	512

typedef	unsigned int	u32;
typedef	unsigned short	u16;
typedef	unsigned char	u8;

#define	MAXPATH		256

#endif // CONFIG_H
