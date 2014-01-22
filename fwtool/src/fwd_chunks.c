//
// Sony NEX camera firmware toolbox
//
// written (reverse-engineered) by Paul Bartholomew, released under the GPL
// (originally based on "pr.exe" from nex-hack.info, with much more since then)
//
// Copyright (C) 2012-2013, nex-hack project
//
// This file "fwd_chunks.c" is part of fwtool (http://www.nex-hack.info)
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

// fwdata_chunks contains unpacking and repacking of fwdata file

#include "config.h"
#include "fwt_names.h"
#include "fwt_util.h"
#include "fwd_chunks.h"

#include "endian.h"

#include "zlib.h"	// for crc-32

// firmware images should start with this magic header
static const unsigned char	fwdata_header_magic[] = {
	0x89, 0x55, 0x46, 0x55, 0x0d, 0x0a, 0x1a, 0x0a,
};

// firmware images end with this DEND chunk, add crc-32
static const unsigned char	fwdata_dend_head[] = {
	0x00, 0x00, 0x00, 0x04, 0x44, 0x45, 0x4E, 0x44,
};


static int
copy_chunk_payload_to_file(FILE *fh_in, const char *chunk_id, u32 chunk_len, const char *fname_out)
{
	unsigned char	io_buf[_TMP_IO_BUFLEN]; // =4k defined in config.h
	int	this_len;
	FILE	*fh_chunk_out = NULL;
	int	retval;

	sprintf(plog_global,"Write chunk %s to '%s' ... ", chunk_id, fname_out); log_it(plog_global);

	if (!(fh_chunk_out = fopen((const char *)fname_out, "wb"))) {
		fprintf(stderr, "copy_chunk_payload_to_file(): Error creating chunk payload file '%s'!\n", fname_out);
		goto exit_err;
	}

	while(chunk_len != 0) {
		if (chunk_len < sizeof(io_buf)) {
			this_len = chunk_len;
		} else {
			this_len = sizeof(io_buf);
		}
		if (fread(io_buf, 1, this_len, fh_in) != this_len) {
			fprintf(stderr, "copy_chunk_payload_to_file(): Error reading from input file!\n");
			goto exit_err;
		}
		if (fwrite(io_buf, 1, this_len, fh_chunk_out) != this_len) {
			fprintf(stderr, "copy_chunk_payload_to_file(): Error writing to output file!\n");
			goto exit_err;
		}

		chunk_len -= this_len;
	}
	fclose(fh_chunk_out);
	goto exit_ok;

exit_err:
	if (fh_chunk_out) fclose(fh_chunk_out);
	// remove output file on error
	retval = 1;
	goto exit_common;

exit_ok:
	sprintf(plog_global,"Done\n"); log_it(plog_global);
	retval = 0;
	// fallthru
exit_common:
	return retval;
}


static int
_fwdata_do_unpack(const char *fname_fwdata_in, const char *single_chunk_id, const char *single_chunk_fname_out, const char *full_extract_dirname_out)
{
	FILE	*fh_in = NULL, *fh_chunk_toc_out = NULL;
	unsigned char	hdr_magic_buf[sizeof(fwdata_header_magic)];
	int	retval;
	FWD_CHUNK_HDR	chunk_hdr;
	char	fname_buf[_TMP_FNAME_BUFLEN];
	u32	chunk_len;
	char	chunk_id[sizeof(chunk_hdr.chunk_id)+1];

	if (!(fh_in = fopen(fname_fwdata_in, "rb"))) {
		fprintf(stderr, "Error opening input file '%s'!\n", fname_fwdata_in);
		goto exit_err;
	}

	// confirm that the input file starts with the 'magic' bytes
	// for an update image file
	if ((fread(hdr_magic_buf, 1, sizeof(hdr_magic_buf), fh_in) != sizeof(hdr_magic_buf)) ||
		memcmp(hdr_magic_buf, fwdata_header_magic, sizeof(fwdata_header_magic)) ) {
		fprintf(stderr, "fwdata_unpack_chunks(): Bad file header!\n");
		goto exit_err;
	}

	if (full_extract_dirname_out) {
		// confirm that output directory exists (create if necessary)
		// (void)mkdir(full_extract_dirname_out, 0777);
		mkdir(full_extract_dirname_out);

		// create chunk TOC (table of contents) file
		sprintf(fname_buf, "%s/%s", (char *)full_extract_dirname_out, FWD_CHUNK_TOC_NAME);

		if (!(fh_chunk_toc_out = fopen(fname_buf, "wb"))) {
			fprintf(stderr, "fwdata_unpack_chunks(): Error creating chunk TOC file!\n");
			goto exit_err;
		}
	}

	//
	// the structure of the file (after the 8-byte 'magic' value
	// {0x89,0x55,0x46,0x55,0x0d,0x0a,0x1a,0x0a,} = "‰UFU<cr><lf><sub><lf>")
	// is a series of 'records'.  a record consist of:
	//
	//	- a 32-bit 'record length' value (stored BIG-endian ! )
	//	- a 4-character 'record type' identifier
	//	- 'n' bytes of 'payload data' ('n' determined by 'record length' above)
	//
	// the 'record types' I have seen in the firmware image I looked at:
	// (extended by kenan)
	//
	//	- DATV n=4  : target type: {0x01,0x00,0x00,0x00} = body
	//                             {0x01,0x00,0x00,0x01} = lens (e-mount only)
	//	- PROV n=4  : unknown: {0x01,0x00,0x00,0x00} in all user fw files to date
	//	- UDID n=var: unknown: 32-bit BIG-endian 'count' value c,
	//                         c-1 x {0x..,0x..,0x05,0x4C}, {0x01,0x00,0x00,0x00}
	//                               {0x03,0xE2,0x05,0x4C}, {0x02,0x00,0x00,0x00}
	//	- FDAT n=var: the firmware data (the big piece we want to decrypt)
	//	- DEND n=4  : end of file: last record, payload is a 32-bit CRC
	//                (stored BIG-endian) of the entire file *preceding* the
	//                DEND record/record header)
	//
	// this loop will read each record header, determine what
	// the 'record type' is, and create an output file in "full_extract_dirname_out"
	// based on that type, containing the entire payload.
	//
	// the loop stops when the DEND record is seen.
	//
	while(!feof(fh_in)) {
		if (fread(&chunk_hdr, 1, sizeof(chunk_hdr), fh_in) != sizeof(chunk_hdr)) {
			fprintf(stderr, "fwdata_unpack_chunks(): Error reading chunk header!\n");
			goto exit_err;
		}
		chunk_len = readBE32(chunk_hdr.chunk_len_BE_bytes);

		memset(chunk_id, 0, sizeof(chunk_id));
		sprintf(chunk_id, "%.*s", sizeof(chunk_hdr.chunk_id), chunk_hdr.chunk_id);

		if (!strcmp(chunk_id, "DEND")) {
			// DEND payload should be 4-byte CRC (stored big-endian)
			// of the entire file, other than the DEND chunk (and DEND chunk header)
			break;
		}

		if (single_chunk_id && !strcmp((char *)single_chunk_id, chunk_id)) {
			strcpy(fname_buf, single_chunk_fname_out);
		} else if (full_extract_dirname_out) {
			sprintf((char *)fname_buf, "%s/"FWD_CHUNK_NAME_FMT, (char *)full_extract_dirname_out, chunk_id);
			if (fh_chunk_toc_out) fprintf(fh_chunk_toc_out, "%s\t"FWD_CHUNK_NAME_FMT"\n", chunk_id, chunk_id);
		} else {
			fseek(fh_in, chunk_len, SEEK_CUR);
			continue;
		}

		if (copy_chunk_payload_to_file(fh_in, chunk_id, chunk_len, fname_buf) != 0) {
			fprintf(stderr, "fwdata_unpack_chunks(): Error copying chunk payload to file!\n");
			unlink(fname_buf);
			goto exit_err;
		}
	}
	fclose(fh_in);
	goto exit_ok;

exit_err:
	if (fh_in) fclose(fh_in);
	retval = 1;
	goto exit_common;

exit_ok:
	retval = 0;
	// fallthru

exit_common:
	return retval;
}


int
fwdata_unpack_chunks(const char *fname_fwdata_in, const char *dirname_out)
{
	return _fwdata_do_unpack(fname_fwdata_in, NULL, NULL, dirname_out);
}


int
fwdata_extract_chunk_to_file(const char *fname_fwdata_in, const char *chunk_id, const char *fname_out)
{
	return _fwdata_do_unpack(fname_fwdata_in, chunk_id, fname_out, NULL);
}


// added by kenan
static int
read_chunk_file_to_buffer(unsigned char *pbuf, const char *fname_in, const char *chunk_id, u32 *p_chunk_len)
{
	int	retval;
	FILE	*fc_in = NULL;
	size_t	filesize=0;
	FWD_CHUNK_HDR	chunk_hdr;

	if (!(fc_in = fopen(fname_in, "rb"))) {
		fprintf(stderr, "read_chunk_file_to_buffer(): Error reading chunk file '%s'!\n", fname_in);
		goto exit_err;
	}

	// get total input file size
	fseek(fc_in, 0L, SEEK_END);
	filesize = ftell(fc_in);
	fseek(fc_in, 0L, SEEK_SET);
	*p_chunk_len=filesize+sizeof(chunk_hdr);

	if (fread(pbuf+sizeof(chunk_hdr), 1, filesize, fc_in) != filesize) {
		fprintf(stderr, "read_chunk_file_to_buffer(): Failed to read chunk file '%s'!\n", fname_in);
		goto exit_err;
	}
	fclose(fc_in);

	memcpy(pbuf+sizeof(chunk_hdr.chunk_len_BE_bytes),chunk_id,sizeof(chunk_hdr.chunk_id));
	// store payload length BE (UGLY!)
	pbuf[3] = (u8) ((filesize & 0xff));
	pbuf[2] = (u8) ((filesize & 0xff00) >> 8);
	pbuf[1] = (u8) ((filesize & 0xff0000) >> 16);
	pbuf[0] = (u8) ((filesize & 0xff000000) >> 24);

	sprintf(plog_global,"read chunk file '%s', id '%s', len %02d\n", fname_in, chunk_id, *p_chunk_len); log_it(plog_global);

	goto exit_ok;

exit_err:
	if (fc_in) fclose(fc_in);
	retval = 1;
	goto exit_common;

exit_ok:
	retval = 0;
	// fallthru
exit_common:
	return retval;
}


// added by kenan
int
fwdata_repack_chunks(const char *fname_fwdata_in, const char *dirname_out)
{
	char	fname_chunk[MAXPATH] = "";
	char	fname_out[MAXPATH] = "";
	char	fname_fdatrepack[MAXPATH] = "";
	FILE	*fc_out = NULL, *fc_in = NULL;

	int	retval;
	FWD_CHUNK_HDR	chunk_hdr;
	u32	chunk_len=0;
	u32	*p_chunk_len=&chunk_len;
	char	chunk_id[sizeof(chunk_hdr.chunk_id)+1] = "";

	unsigned char	*p_buf = NULL;
	int	p_buf_offset=sizeof(fwdata_header_magic);
	size_t	filesize=0;
	int	block_len = 0;

	uLong dend_crc = crc32(0L, Z_NULL, 0);	// init from zlib

	//
	sprintf(fname_fdatrepack, "%s/%s", dirname_out, BASENAME_FDAT_REENCRYPTED);

	// size of repacked FDAT for buffer (UGLY, may use >200mb, should do recursive with smaller buffer)
	if (!(fc_in = fopen(fname_fdatrepack, "rb"))) {
		fprintf(stderr, "fwdata_repack_chunks(): Error reading FDAT.repack file '%s'!\n", fname_fdatrepack);
		goto exit_err;
	}
	// get total input file size
	fseek(fc_in, 0L, SEEK_END);
	filesize = ftell(fc_in);
	fseek(fc_in, 0L, SEEK_SET);
	fclose(fc_in);

	block_len=filesize+FWDATA_MAXHEADLEN;
	if (!(p_buf = malloc(block_len))) {
		fprintf(stderr, "fwdata_repack_chunks(): no memory for block buffer!\n");
		goto exit_err;
	}

	// create magic
	sprintf(plog_global,"Create fwdata header\n"); log_it(plog_global);
	memcpy(p_buf, fwdata_header_magic, sizeof(fwdata_header_magic));

	// create header chunks
	strcpy(chunk_id,"DATV");
	sprintf(fname_chunk, "%s/%s", dirname_out, BASENAME_DATV_CHUNK);
	read_chunk_file_to_buffer(p_buf+p_buf_offset, fname_chunk, chunk_id, p_chunk_len);
	p_buf_offset=p_buf_offset+*p_chunk_len;
	sprintf(plog_global,"%s chunk: len %02d offs %02d\n", chunk_id, *p_chunk_len, p_buf_offset); log_it(plog_global);

	strcpy(chunk_id,"PROV");
	sprintf(fname_chunk, "%s/%s", dirname_out, BASENAME_PROV_CHUNK);
	read_chunk_file_to_buffer(p_buf+p_buf_offset, fname_chunk, chunk_id, p_chunk_len);
	p_buf_offset=p_buf_offset+*p_chunk_len;
	sprintf(plog_global,"%s chunk: len %02d offs %02d\n", chunk_id, *p_chunk_len, p_buf_offset); log_it(plog_global);

	strcpy(chunk_id,"UDID");
	sprintf(fname_chunk, "%s/%s", dirname_out, BASENAME_UDID_CHUNK);
	read_chunk_file_to_buffer(p_buf+p_buf_offset, fname_chunk, chunk_id, p_chunk_len);
	p_buf_offset=p_buf_offset+*p_chunk_len;
	sprintf(plog_global,"%s chunk: len %02d offs %02d\n", chunk_id, *p_chunk_len, p_buf_offset); log_it(plog_global);

	strcpy(chunk_id,"FDAT");
	sprintf(fname_chunk, "%s/%s", dirname_out, BASENAME_FDAT_REENCRYPTED);
	read_chunk_file_to_buffer(p_buf+p_buf_offset, fname_chunk, chunk_id, p_chunk_len);	//TODO read n times 1m blocks, do ++crc32
	p_buf_offset=p_buf_offset+*p_chunk_len;
	sprintf(plog_global,"%s chunk: len %02d offs %02d\n", chunk_id, *p_chunk_len, p_buf_offset); log_it(plog_global);

	// do crc-32
	dend_crc = crc32(dend_crc, p_buf, p_buf_offset);
	// memcpy(p_buf+p_buf_offset+sizeof(chunk_hdr),&dend_crc,sizeof(u32)); // wrong endian
	// store payload crc32 BE (UGLY!)
	p_buf[p_buf_offset+sizeof(chunk_hdr)+3] = (u8) ((dend_crc & 0xff));
	p_buf[p_buf_offset+sizeof(chunk_hdr)+2] = (u8) ((dend_crc & 0xff00) >> 8);
	p_buf[p_buf_offset+sizeof(chunk_hdr)+1] = (u8) ((dend_crc & 0xff0000) >> 16);
	p_buf[p_buf_offset+sizeof(chunk_hdr)+0] = (u8) ((dend_crc & 0xff000000) >> 24);
	// create DEND chunk header
	memcpy(p_buf+p_buf_offset,fwdata_dend_head,sizeof(chunk_hdr));
	p_buf_offset=p_buf_offset+sizeof(chunk_hdr)+fwdata_dend_head[3];
	sprintf(plog_global,"DEND chunk: len %02d offs %02d crc32 %#08x ... ", sizeof(chunk_hdr)+fwdata_dend_head[3], p_buf_offset, (unsigned int)dend_crc); log_it(plog_global);

	// write out fwdata.repack
	sprintf(fname_out, "%s/%s", dirname_out, FWD_REPACKED_NAME);
	if (!(fc_out = fopen(fname_out, "wb"))) {
		fprintf(stderr, "fwdata_repack_chunks(): Error opening repack file '%s'!\n", fname_out);
		goto exit_err;
	}

	if (fwrite(p_buf, 1, p_buf_offset, fc_out) != p_buf_offset) {
		fprintf(stderr, "fwdata_repack_chunks(): Failed to write output file '%s'!\n", fname_out);
		goto exit_err;
	}
	fclose(fc_out);
	sprintf(plog_global,"Modified Firmware written to '%s'\n", fname_out);

	goto exit_ok;

exit_err:
	if (fc_out) fclose(fc_out);
	unlink(fname_out);
	retval = 1;
	goto exit_common;

exit_ok:
	sprintf(plog_global,"Done\n"); log_it(plog_global);
	retval = 0;
	// fallthru
exit_common:
	if (p_buf) free(p_buf);
	return retval;
}
