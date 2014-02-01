//
// Sony NEX camera firmware toolbox
//
// written (reverse-engineered) by Paul Bartholomew, released under the GPL
// (originally based on "pr.exe" from nex-hack.info, with much more since then)
//
// Copyright (C) 2012-2013, nex-hack project
//
// This file "lzpt_io.h" is part of fwtool (http://www.nex-hack.info)
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

#ifndef LZPT_IO_H
#define	LZPT_IO_H

extern	int	lzpt_read_toc(FILE *fh_in, unsigned char **pp_toc, int *pnum_entries, int *pmax_dblksz);
extern	int	lzpt_read_block(FILE *fh_in, int block_num, 
		unsigned char *p_toc, size_t toc_entries, unsigned char **pp_block_data, size_t *psz_block);
extern	int	lzpt_decompress_block(unsigned char *p_block_in, size_t sz_block_in, int sz_mxdblk_in,
		unsigned char **pp_block_out, size_t *psz_block_out);
extern	void	lzpt_free_toc(unsigned char *p_toc, size_t toc_nentries);

extern	int	is_lzpt_file(const char *fname);
extern	int	lzpt_decompress_file(const char *fname_in, const char *fname_out);

#endif // LZPT_IO_H

