//
// Sony NEX camera firmware toolbox
//
// written (reverse-engineered) by Paul Bartholomew, released under the GPL
// (originally based on "pr.exe" from nex-hack.info, with much more since then)
//
// Copyright (C) 2012-2013, nex-hack project
//
// This file "zipfile.h" is part of fwtool (http://www.nex-hack.info)
//
// All rights reserved.
//
// This uses modified parts of minizip  Version 1.1, February 14h, 2010
//   part of the MiniZip project - ( http://www.winimage.com/zLibDll/minizip.html )
//   Copyright (C) 1998-2010 Gilles Vollant (minizip)
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

#ifndef	ZIPFILE_H
#define	ZIPFILE_H


#ifdef	ZIPFILE_C	// internal to zipfile.c
	#define	ZIP_IOBUF_SIZE	1048576

	typedef struct t_zip_handle_struct {
		unzFile	uf;
	    unz_global_info64 gi;
		unz_file_info64 file_info;

		int	next_idx;
	} zip_handle_struct;
#endif


typedef struct t_zip_handle_struct *zip_handle;

extern	int	is_zipfile(const char *fname);

extern	zip_handle	zipfile_open(const char *fname);
extern	void	zipfile_close(zip_handle zh);

extern	int	zipfile_find_first(zip_handle zh, char *p_fname_buf, int sz_fname_buf, unsigned long *p_uncomp_size, unsigned int *p_crc32);
extern	int	zipfile_find_next(zip_handle zh, char *p_fname_buf, int sz_fname_buf, unsigned long *p_uncomp_size, unsigned int *p_crc32);

extern	int	zipfile_list(zip_handle zh);

extern	int	zipfile_extract_file(char *fname_zip, char *fname_inzip, char *dirname_out, char *p_extracted_namebuf);
extern	int	zipfile_extract_all(char *fname_zip, char *dirname_out, char *p_extracted_dir, char *p_extracted_fname, int show_extract_names);

#endif // ZIPFILE_H
