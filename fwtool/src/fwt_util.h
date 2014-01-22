//
// Sony NEX camera firmware toolbox
//
// written (reverse-engineered) by Paul Bartholomew, released under the GPL
// (originally based on "pr.exe" from nex-hack.info, with much more since then)
//
// Copyright (C) 2012-2013, nex-hack project
//
// This file "fwt_util.h" is part of fwtool (http://www.nex-hack.info)
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

#ifndef FWT_UTIL_H
#define FWT_UTIL_H

// globals for logfile
extern int fwt_verbose_global;
extern char	plog_global[1024];
// globals for logfile

// for model database see fwt_util.c:name_model()
#define	NUM_MODEL_TYPE	47

// model type database, add fields if neccesary
typedef struct tagMODEL_TYPE {
	unsigned int	modt_fih_model;
	unsigned char	modt_name[32];
} MODEL_TYPE;


extern void	do_version(const char *tool_name);
extern void	do_usage(const char *tool_name);
extern void	do_help(const char *tool_name);
extern void	do_todolist(const char *tool_name);

extern char	*convert_path_slashes(char *buf);
extern void	log_it(char *pinfo);

extern void	name_model(const unsigned int fih_model_type, MODEL_TYPE *buf);


#endif // FWT_UTIL_H
