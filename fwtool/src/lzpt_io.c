//
// Sony NEX camera firmware toolbox
//
// written (reverse-engineered) by Paul Bartholomew, released under the GPL
// (originally based on "pr.exe" from nex-hack.info, with much more since then)
//
// Copyright (C) 2012-2013, nex-hack project
//
// This file "lzpt_io.c" is part of fwtool (http://www.nex-hack.info)
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fwt_names.h"
#include "fwt_util.h"

#include "endian.h"
#include "lz77.h"

#include "lzpt_io.h"


#define	LZPT_MAGIC_OFS		0
#define	LZPT_MAGIC	"TPZL"
#define	LZPT_MAGIC_LEN	(sizeof(LZPT_MAGIC)-1)
#define	LZPT_VERSION_OFS	4

#define	LZPT_TOC_OFFSET_OFS	8
#define	LZPT_TOC_SIZE_OFS	12

#define	LZPT_BLOCK_ALIGN_MASK	(sizeof(unsigned int)-1)

#define	LZPT_MAX_DECOMP_BLOCK_SIZE_VER10	65536
#define	LZPT_MAX_DECOMP_BLOCK_SIZE_VER11	131072


int
lzpt_read_toc(FILE *fh_in, unsigned char **pp_toc, int *pnum_entries, int *pmax_dblksz)
{
	unsigned char	iobuf[512], *p_toc = NULL;
	unsigned int	lztp_version, toc_offset, toc_size, toc_nentries;

	fseek(fh_in, LZPT_MAGIC_OFS, SEEK_SET);
	if ((fread(iobuf, 1, LZPT_MAGIC_LEN, fh_in) != LZPT_MAGIC_LEN) ||
		memcmp((void *)iobuf, (void *)LZPT_MAGIC, LZPT_MAGIC_LEN)) {
		fprintf(stderr, "lzpt_read_toc(): Bad magic!\n");
		return 1;
	}

	//added by kenan: LZTP 0x10 mxdblksz=64k, 0x11 =128k (used by EA50, FS700)
	fseek(fh_in, LZPT_VERSION_OFS, SEEK_SET);
	fread(iobuf, sizeof(unsigned int), 1, fh_in);
	lztp_version = readLE32(iobuf);
	switch (lztp_version) {
		case 0x00000010:
			if (pmax_dblksz) *pmax_dblksz = LZPT_MAX_DECOMP_BLOCK_SIZE_VER10;
			break;
		case 0x00000011:
			if (pmax_dblksz) *pmax_dblksz = LZPT_MAX_DECOMP_BLOCK_SIZE_VER11;
			break;
		default:
			fprintf(stderr, "lzpt_read_toc(): Bad version!\n");
			return 1;
	}

	fseek(fh_in, LZPT_TOC_OFFSET_OFS, SEEK_SET);
	fread(iobuf, sizeof(unsigned int), 1, fh_in);
	toc_offset = readLE32(iobuf);

	fseek(fh_in, LZPT_TOC_SIZE_OFS, SEEK_SET);
	fread(iobuf, sizeof(unsigned int), 1, fh_in);
	toc_size = readLE32(iobuf);

	toc_nentries = (toc_size/(2 * sizeof(unsigned int)));

	if (!(p_toc = (unsigned char *)malloc(toc_size))) {
		fprintf(stderr, "lzpt_read_toc(): Error allocating TOC!\n");
		return 1;
	}

	memset(p_toc, 0, toc_size);
	fseek(fh_in, toc_offset, SEEK_SET);

	if (fread(p_toc, 1, toc_size, fh_in) != toc_size) {
		fprintf(stderr, "lzpt_read_toc(): Error reading TOC!\n");
		return 1;
	}

	if (pp_toc) {
		*pp_toc = p_toc;
	} else {
		free((void *)p_toc);
	}

	if (pnum_entries) *pnum_entries = toc_nentries;

	return 0;
}


int
lzpt_read_block(FILE *fh_in, int block_num, unsigned char *p_toc, size_t toc_entries, unsigned char **pp_block_data, size_t *psz_block)
{
	unsigned int	toc_offset, entry_offset, entry_size, nread;
	unsigned char	*p_block;

	*pp_block_data = NULL;
	*psz_block = 0;

	if (block_num >= toc_entries) {
		return 1;
	}
	toc_offset = (block_num * (sizeof(unsigned int) * 2));

	entry_offset = readLE32(p_toc+toc_offset);
	entry_size = readLE32(p_toc+toc_offset+4);

	if (!(p_block = malloc(entry_size))) {
		fprintf(stderr, "lzpt_read_block(): allocation error!\n");
		return 1;
	}
	fseek(fh_in, entry_offset, SEEK_SET);
	nread = fread(p_block, 1, entry_size, fh_in);

	if (nread != entry_size) {
		// special case: last block in file may be smaller by align size
		if ((block_num == (toc_entries-1)) && ((entry_size - nread) <= LZPT_BLOCK_ALIGN_MASK)) {
			// OK
		} else {
			fprintf(stderr, "lzpt_read_block(): block read error!\n");
			free((void *)p_block);
			return 1;
		}
	}
	*pp_block_data = p_block;
	*psz_block = nread;

	return 0;
}


int
lzpt_decompress_block(unsigned char *p_block_in, size_t sz_block_in, int sz_mxdblk_in, unsigned char **pp_block_out, size_t *psz_block_out)
{
	unsigned char	*p_block = NULL, *p_in, *p_out, *next_in;
	int	remain_in, remain_out, this_declen;
	size_t	total_decomp_size;

	*pp_block_out = NULL;
	*psz_block_out = 0;

	remain_out = sz_mxdblk_in;
	if (!(p_block = malloc(remain_out))) {
		fprintf(stderr, "lzpt_decompress_block(): allocation error!\n");
		return 1;
	}

	remain_in = sz_block_in;
	p_in = p_block_in;
	p_out = p_block;
	total_decomp_size = 0;
	while((remain_in > LZPT_BLOCK_ALIGN_MASK) && (remain_out > 0)) {
		this_declen = lz77_inflate(p_in, remain_in, p_out, remain_out, &next_in);
		if (this_declen <= 0) {
			break;
		}
		p_out += this_declen;
		remain_out -= this_declen;
		total_decomp_size += this_declen;

		remain_in -= (next_in - p_in);
		p_in = next_in;
	}


	if ((remain_in > LZPT_BLOCK_ALIGN_MASK) || (remain_out < 0)) {
		fprintf(stderr, "lzpt_decompress_block(): decompress error! (%d %d %d)\n", remain_in, LZPT_BLOCK_ALIGN_MASK, remain_out);
		free((void *)p_block);
		return 1;
	}

	*pp_block_out = p_block;
	*psz_block_out = total_decomp_size;

	if (total_decomp_size != sz_mxdblk_in) {
		printf("ERROR!\n");
		exit(1);
	}
	return 0;
}


void
lzpt_free_toc(unsigned char *p_toc, size_t toc_nentries)
{
	if (p_toc) free((void *)p_toc);
}


int
is_lzpt_file(const char *fname)
{
	FILE	*fh = NULL;
	int	ret;

	if (!(fh = fopen(fname, "rb"))) {
		return 0;
	}
	ret = lzpt_read_toc(fh, NULL, NULL, NULL);
	fclose(fh);

	return (ret == 0);
}


int
lzpt_decompress_file(const char *fname_in, const char *fname_out)
{
	FILE	*fh_in = NULL, *fh_out = NULL;
	unsigned char	*p_toc = NULL, *p_enc_block = NULL, *p_dec_block = NULL;
	size_t	sz_enc_block, sz_dec_block;
	// size_t  filesize;
	int	toc_nentries, i, sz_max_dec_block, retval;


	if (!(fh_in = fopen(fname_in, "rb"))) {
		fprintf(stderr, "Error opening input file '%s'!\n", fname_in);
		goto exit_err;
	}
	// fseek(fh_in, 0L, SEEK_END);
	// filesize = ftell(fh_in);
	// fseek(fh_in, 0L, SEEK_SET);


	if (lzpt_read_toc(fh_in, &p_toc, &toc_nentries, &sz_max_dec_block)) {
		fprintf(stderr, "Error reading TOC!\n");
		goto exit_err;
	}

	sprintf(plog_global, "LZPT: decompressing '%s' (%d blocks, %d maxdblksz)\n      to '%s' ... ", fname_in, toc_nentries, sz_max_dec_block, fname_out); log_it(plog_global);

	if (!(fh_out = fopen(fname_out, "wb"))) {
		fprintf(stderr, "Error creating output file '%s'!\n", fname_out);
		goto exit_err;
	}

	for (i = 0; i < toc_nentries; i++) {
		if (lzpt_read_block(fh_in, i, p_toc, toc_nentries, &p_enc_block, &sz_enc_block)) {
			fprintf(stderr, "Error reading LZTP block #%d\n", i);
			goto exit_err;
		}
		if (lzpt_decompress_block(p_enc_block, sz_enc_block, sz_max_dec_block, &p_dec_block, &sz_dec_block)) {
			fprintf(stderr, "Error decompressing LZTP block #%d\n", i);
			goto exit_err;
		}
		free((void *)p_enc_block); p_enc_block = NULL;

		if (fwrite(p_dec_block, 1, sz_dec_block, fh_out) != sz_dec_block) {
			fprintf(stderr, "Error writing LZTP block #%d\n", i);
			goto exit_err;
		}
		free((void *)p_dec_block); p_dec_block = NULL;
	}

	fclose(fh_in); fh_in = NULL;
	fclose(fh_out); fh_out = NULL;

	sprintf(plog_global, "Done\n"); log_it(plog_global);
	goto exit_ok;


exit_err:
	if (fh_in) fclose(fh_in);
	if (fh_out) fclose(fh_out);
	unlink(fname_out);
	retval = 1;
	goto exit_common;

exit_ok:
	retval = 0;
	// fallthru

exit_common:
	if (p_toc) lzpt_free_toc(p_toc, toc_nentries);
	if (p_enc_block) free(p_enc_block);
	if (p_dec_block) free(p_dec_block);

	return retval;
}
