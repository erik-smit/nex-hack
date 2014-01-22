//
// Sony NEX camera firmware toolbox
//
// written (reverse-engineered) by Paul Bartholomew, released under the GPL
// (originally based on "pr.exe" from nex-hack.info, with much more since then)
//
// Copyright (C) 2012-2013, nex-hack project
//
// This file "tarfile.h" is part of fwtool (http://www.nex-hack.info)
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

#ifndef TARFILE_H
#define TARFILE_H

#ifdef	TARFILE_C	// internal to tarfile.c
	#define	TAR_IOBUF_SIZE	10240

	typedef struct t_tar_handle_struct {
		struct archive *a;
	} tar_handle_struct;
#endif	// TARFILE_C


typedef struct t_tar_handle_struct *tar_handle;

extern	tar_handle	tarfile_open(const char *fname);
extern	void	tarfile_close(tar_handle th);

extern	int	is_tarfile(const char *fname);

extern	int	tarfile_find_first(tar_handle th, char *p_fname_buf, int sz_fname_buf, unsigned long *p_uncomp_size, unsigned int *p_crc32);
extern	int	tarfile_find_next(tar_handle th, char *p_fname_buf, int sz_fname_buf, unsigned long *p_uncomp_size, unsigned int *p_crc32);

extern	int	tarfile_list(tar_handle th);

extern	int	tarfile_extract_all(const char *fname_tar, const char *dirname_out, int show_extract_names);

#endif // TARFILE_H
