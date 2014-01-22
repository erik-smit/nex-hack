//
// Sony NEX camera firmware toolbox
//
// written (reverse-engineered) by Paul Bartholomew, released under the GPL
// (originally based on "pr.exe" from nex-hack.info, with much more since then)
//
// Copyright (C) 2012-2013, nex-hack project
//
// This file "fdat_image.c" is part of fwtool (http://www.nex-hack.info)
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

#include "config.h"
#include "fwt_names.h"
#include "fwt_util.h"

#include "endian.h"
#include "csum.h"
#include "fdat_cipher.h"

#include "fdat_image.h"

#include "zlib.h"	// for crc-32


int
fdat_read_image_header(const char *fname_fdat, FDAT_IMAGE_HEADER *p_image_hdr)
{
	FILE	*fh;
	FDAT_IMAGE_HEADER	l_hdr;
	FDAT_FS_IMAGE_DESC	*p_desc;
	int	i;

	if (!(fh = fopen(fname_fdat, "rb"))) {
		fprintf(stderr, "Failed to open FDAT image '%s'!\n", fname_fdat);
		return 1;
	}
	if (fread((void *)&l_hdr, 1, sizeof(l_hdr), fh) != sizeof(l_hdr)) {
			fprintf(stderr, "Error reading FDAT image header!\n");
			fclose(fh);
			return 1;
	}
	fclose(fh);
	if (memcmp(l_hdr.fih_magic, FDAT_IMAGE_MAGIC, sizeof(l_hdr.fih_magic))) {
			fprintf(stderr, "Invalid FDAT image header!\n");
			return 1;
	}
	// endian-convert header

	LE32toHost((u8 *)&l_hdr.fih_header_crc);
	LE32toHost((u8 *)&l_hdr.fih_fw_offset);
	LE32toHost((u8 *)&l_hdr.fih_fw_length);
	LE32toHost((u8 *)&l_hdr.fih_fs_image_count);

	for (i = 0; i < l_hdr.fih_fs_image_count; i++) {
		p_desc = &l_hdr.fih_fs_image_info[i];
		LE32toHost((u8 *)&p_desc->ffid_fs_length);
		LE32toHost((u8 *)&p_desc->ffid_fs_offset);
	}

	memcpy((void *)p_image_hdr, (void *)&l_hdr, sizeof(l_hdr));

	return 0;
}


static int
_fdat_extract_image(const char *fname_in, unsigned int f_offset, unsigned int f_len, const char *fname_out)
{
	int	ret;
	FILE	*fh_in = NULL, *fh_out = NULL;
	unsigned char	*p_iobuf = NULL;
	unsigned int	this_nbytes;

	if (!(fh_in = fopen(fname_in, "rb"))) {
		goto exit_err;
	}
	fseek(fh_in, f_offset, SEEK_SET);
	if (!(fh_out = fopen(fname_out, "wb"))) {
		goto exit_err;
	}
	if (!(p_iobuf = malloc(FDAT_EXTRACT_IOBUF_SIZE))) {
		goto exit_err;
	}
	while(f_len) {
		if (f_len > FDAT_EXTRACT_IOBUF_SIZE) {
			this_nbytes = FDAT_EXTRACT_IOBUF_SIZE;
		} else {
			this_nbytes = f_len;
		}
		if (fread(p_iobuf, 1, this_nbytes, fh_in) != this_nbytes) {
			goto exit_err;
		}
		if (fwrite(p_iobuf, 1, this_nbytes, fh_out) != this_nbytes) {
			goto exit_err;
		}

		f_len -= this_nbytes;
	}
	fclose(fh_in);
	fclose(fh_out);
	goto exit_ok;

exit_err:
	ret = 1;
	if (fh_in) fclose(fh_out);
	if (fh_out) fclose(fh_out);
	unlink(fname_out);
	goto exit_common;
exit_ok:
	ret = 0;
	// fallthru
exit_common:
	if (p_iobuf) free((void *)p_iobuf);
	return ret;
}


int
fdat_extract_firmware_image(const char *fname_fdat, const char *fname_fw_image)
{
	FDAT_IMAGE_HEADER	fdat_hdr;
	int	ret = 0;

	if (fdat_read_image_header(fname_fdat, &fdat_hdr)) {
		goto exit_err;
	}

	if (!fdat_hdr.fih_fw_length || (_fdat_extract_image(fname_fdat, fdat_hdr.fih_fw_offset, fdat_hdr.fih_fw_length, fname_fw_image) != 0)) {
		fprintf(stderr, "Error extracting firmware image from '%s'!\n", fname_fdat);
		goto exit_err;
	}

	goto exit_ok;

exit_err:
	ret = 1;
	unlink(fname_fw_image);
	goto exit_common;
exit_ok:
	ret = 0;
	// fallthru
exit_common:
	return ret;
}


int
fdat_fs_image_count(const char *fname_fdat)
{
	FDAT_IMAGE_HEADER	fdat_hdr;

	if (fdat_read_image_header(fname_fdat, &fdat_hdr)) {
		return -1;
	}
	return fdat_hdr.fih_fs_image_count;
}


int
fdat_fs_image_length(const char *fname_fdat, int image_idx)
{
	FDAT_IMAGE_HEADER	fdat_hdr;
	FDAT_FS_IMAGE_DESC	*p_desc;

	if (fdat_read_image_header(fname_fdat, &fdat_hdr)) {
		return -1;
	}
	if (image_idx >= fdat_hdr.fih_fs_image_count) {
		return -1;
	}
	p_desc = &fdat_hdr.fih_fs_image_info[image_idx];
	return (int)p_desc->ffid_fs_length;
}


int
fdat_extract_fs_image(const char *fname_fdat, int image_idx, const char *fname_fs_image)
{
	FDAT_IMAGE_HEADER	fdat_hdr;
	int	ret = 0;
	FDAT_FS_IMAGE_DESC	*p_desc;

	if (fdat_read_image_header(fname_fdat, &fdat_hdr)) {
		goto exit_err;
	}
	if (image_idx >= fdat_hdr.fih_fs_image_count) {
		goto exit_err;
	}

	p_desc = &fdat_hdr.fih_fs_image_info[image_idx];

	// some firmware images seem to have a zero-length second fw image
	// (as opposed to setting # images to '1'), so return "ok"
	// if length is zero (and don't create any output file)
	if (!p_desc->ffid_fs_length) {
		goto exit_ok;
	}
	if (_fdat_extract_image(fname_fdat, p_desc->ffid_fs_offset, p_desc->ffid_fs_length, fname_fs_image) != 0) {
		fprintf(stderr, "Error extracting filesystem image #%d from '%s'!\n", image_idx, fname_fdat);
		goto exit_err;
	}
	goto exit_ok;

exit_err:
	ret = 1;
	unlink(fname_fs_image);
	goto exit_common;
exit_ok:
	ret = 0;
	// fallthru
exit_common:
	return ret;
}


int
fdat_header_tofile(const char *fname_fdat, const char *fname_fdat_header)
{
	FDAT_IMAGE_HEADER	fdat_hdr;
	FDAT_FS_IMAGE_DESC	*p_desc;
	MODEL_TYPE	model_name;
	int	ret = 0;

	if (fdat_read_image_header(fname_fdat, &fdat_hdr)) {
		return -1;
	}
	p_desc = &fdat_hdr.fih_fs_image_info[0];

    // header begins at pos=0 and has len=offs(fs[0])
	if (_fdat_extract_image(fname_fdat, 0, (unsigned int)p_desc->ffid_fs_offset, fname_fdat_header) != 0) {
		fprintf(stderr, "Error extracting fdat header from '%s'!\n", fname_fdat);
		goto exit_err;
	}

	name_model(fdat_hdr.fih_model, &model_name);
	sprintf(plog_global, "FDAT model '%s' (%#04x) version '%d.%02d'\n", model_name.modt_name, model_name.modt_fih_model, fdat_hdr.fih_version_major, fdat_hdr.fih_version_minor); log_it(plog_global);

   	goto exit_ok;

exit_err:
	ret = 1;
	unlink(fname_fdat_header);
	goto exit_common;
exit_ok:
	ret = 0;
	// fallthru
exit_common:
	return ret;
}


int
fdat_repack(const char *fname_fdat, const char *fname_fdat_repack, const char *dirname_parts, const int fwt_majorver, const int fwt_minorver)
{
	int	ret = 0;
	FDAT_IMAGE_HEADER	fdat_hdr;
	FDAT_FS_IMAGE_DESC	*p_desc = NULL;
	MODEL_TYPE	model_name;
	FILE	*fh_in = NULL, *fh_out = NULL;
	int fs_img_count=0, num_fs=0;
	unsigned int	fdat_crc=0;
	size_t	head_len=0, filesize=0, fsimg_offset=0, outfilesize=0;
	unsigned char	*p_block_buf = NULL;
	char	fname_fsimg[MAXPATH] = "";


    // read FDAT header from original file
	sprintf(plog_global, "Reading FDAT dec file header: '%s' ...\n", fname_fdat); log_it(plog_global);
	if (fdat_read_image_header(fname_fdat, &fdat_hdr)) {
		return -1;
	}
	fs_img_count = fdat_hdr.fih_fs_image_count;
	p_desc = &fdat_hdr.fih_fs_image_info[0];    // 0 to MAX_FDAT_FS_IMAGES-1
	head_len = outfilesize += (size_t)p_desc->ffid_fs_offset;	// header len
	sprintf(plog_global, "FDAT outfilesize: 'header' '%d'='%#04x'\n", outfilesize, outfilesize); log_it(plog_global);

    // filesize fs images
	for (num_fs=0; num_fs<MAX_FDAT_FS_IMAGES-1; num_fs++) {
		sprintf(fname_fsimg, "%s/%s%02d%s", dirname_parts, BASENAME_FDAT_FS_PREFIX, num_fs, FSIMAGE_EXT_MOD);
		if ((fh_in = fopen(fname_fsimg, "rb"))) {
			fseek(fh_in, 0L, SEEK_END); filesize = ftell(fh_in); fseek(fh_in, 0L, SEEK_SET);
			outfilesize += filesize;
			sprintf(plog_global, "FDAT outfilesize (added up): '%s' '%d'='%#04x' (fsz='%#04x')\n", fname_fsimg, outfilesize, outfilesize, filesize); log_it(plog_global);
			fclose(fh_in);
		}
	}
	sprintf(fname_fsimg, "%s/%s", dirname_parts, BASENAME_FDAT_FIRMWARE_TAR_MOD);
	if ((fh_in = fopen(fname_fsimg, "rb"))) {
		fseek(fh_in, 0L, SEEK_END); filesize = ftell(fh_in); fseek(fh_in, 0L, SEEK_SET);
		outfilesize += filesize;
		sprintf(plog_global, "FDAT outfilesize (added up): '%s' '%d'='%#04x' (fsz='%#04x')\n", fname_fsimg, outfilesize, outfilesize, filesize); log_it(plog_global);
		fclose(fh_in);
	}
	if (outfilesize < FDAT_EXTRACT_IOBUF_SIZE) {
		fprintf(stderr, "fdat_repack(): Computed filesize '%d' of '%s' much to small!\n", outfilesize, BASENAME_FDAT_REPACKED);
		goto exit_err;
	}

	// buffer for FDAT.repack
	if (!(p_block_buf = malloc(outfilesize))) {
		fprintf(stderr, "fdat_repack(): Failed to allocate block buffer!\n");
		goto exit_err;
	}

	// read in header, fs images and tar
	// TODO: do header modification TODO TODO TODO
	fsimg_offset = head_len;
	for (num_fs=0; num_fs<MAX_FDAT_FS_IMAGES-1; num_fs++) {
		sprintf(fname_fsimg, "%s/%s%02d%s", dirname_parts, BASENAME_FDAT_FS_PREFIX, num_fs, FSIMAGE_EXT_MOD);
		if ((fh_in = fopen(fname_fsimg, "rb"))) {
			p_desc = &fdat_hdr.fih_fs_image_info[num_fs];
			p_desc->ffid_fs_offset = fsimg_offset;
			fseek(fh_in, 0L, SEEK_END); filesize = ftell(fh_in); fseek(fh_in, 0L, SEEK_SET);
			sprintf(plog_global, "Read into buffer '%s' filesize '%#04x'\n", fname_fsimg, filesize); log_it(plog_global);
			if (fread(p_block_buf+fsimg_offset, 1, filesize, fh_in) != filesize) {
				fprintf(stderr, "fdat_repack(): Failed to read '%s' filesize '%d'='%#04x'\n", fname_fsimg, filesize, filesize);
				goto exit_err;
			}
			fclose(fh_in);
			fsimg_offset += filesize;
			p_desc->ffid_fs_length = filesize;
		}
		else {
			if (num_fs+1 < fs_img_count) {
				// TODO: simple info, handle full blown ...
				sprintf(plog_global, "fs image '%s' not found\n", fname_fsimg); log_it(plog_global);
			}
		}
	}
	sprintf(fname_fsimg, "%s/%s", dirname_parts, BASENAME_FDAT_FIRMWARE_TAR_MOD);
	if ((fh_in = fopen(fname_fsimg, "rb"))) {
		fseek(fh_in, 0L, SEEK_END); filesize = ftell(fh_in); fseek(fh_in, 0L, SEEK_SET);
		sprintf(plog_global, "Read into buffer '%s' filesize '%#04x'\n", fname_fsimg, filesize); log_it(plog_global);
		if (fread(p_block_buf+fsimg_offset, 1, filesize, fh_in) != filesize) {
			fprintf(stderr, "fdat_repack(): Failed to read '%s'\n", fname_fsimg);
			goto exit_err;
		}
		fclose(fh_in);
		fdat_hdr.fih_fw_offset = fsimg_offset;
		fdat_hdr.fih_fw_length = filesize;
	}
	if (fsimg_offset + filesize != outfilesize) {
		fprintf(stderr, "fdat_repack(): Filesize mismatch (is '%#04x', should '%#04x')!\n", fsimg_offset+filesize, outfilesize);
		goto exit_err;
	}

	name_model(fdat_hdr.fih_model, &model_name);
	sprintf(plog_global, "FDAT model '%s' (%#04x) original version '%d.%02d'\n", model_name.modt_name, model_name.modt_fih_model, fdat_hdr.fih_version_major, fdat_hdr.fih_version_minor); log_it(plog_global);

	// if not set from option, increment minor version, or major if overflow
	if ((fwt_majorver == -1) && (fwt_minorver == -1)) {
		if (fdat_hdr.fih_version_minor < FWT_MAXMINORVER) fdat_hdr.fih_version_minor += fdat_hdr.fih_version_minor;
		else {
			fdat_hdr.fih_version_major += fdat_hdr.fih_version_major;
			fdat_hdr.fih_version_minor = 0;
		}
	}
	else {
		if (fwt_majorver > FWT_NOMODELVERSION) fdat_hdr.fih_version_major = fwt_majorver;
		if (fwt_minorver > FWT_NOMODELVERSION) fdat_hdr.fih_version_minor = fwt_minorver;
	}
	printf("===   Model '%s' set to version '%d.%02d'   ===\n\n", model_name.modt_name, fdat_hdr.fih_version_major, fdat_hdr.fih_version_minor);
	sprintf(plog_global, "FDAT model '%s' (%#04x) set version to '%d.%02d'\n", model_name.modt_name, model_name.modt_fih_model, fdat_hdr.fih_version_major, fdat_hdr.fih_version_minor); log_it(plog_global);

	// copy modified header into buffer
	// TODO: check if this is SAVE! Maybe copying the struct this way may crash ...
	memcpy(p_block_buf, &fdat_hdr, head_len);

	// header crc32 correction
	fdat_crc = crc32(fdat_crc, p_block_buf+FDAT_IMAGE_MAGIC_LEN+sizeof(fdat_hdr.fih_header_crc), head_len-FDAT_IMAGE_MAGIC_LEN-sizeof(fdat_hdr.fih_header_crc));
	sprintf(plog_global, "Write FDAT crc32 '%#04x'\n", fdat_crc); log_it(plog_global);
	memcpy(p_block_buf+FDAT_IMAGE_MAGIC_LEN, &fdat_crc, sizeof(fdat_hdr.fih_header_crc));

	// write FDAT.repack
	sprintf(plog_global, "Writing FDAT repack output to '%s' ... ", fname_fdat_repack); log_it(plog_global);
	if (!(fh_out = fopen(fname_fdat_repack, "wb"))) {
		fprintf(stderr, "fdat_repack(): Error creating output file '%s'!\n", fname_fdat_repack);
		goto exit_err;
	}
	if (fwrite(p_block_buf, 1, outfilesize, fh_out) != outfilesize) {
		fprintf(stderr, "fdat_repack(): Failed to write output file '%s'!\n", fname_fdat_repack);
		goto exit_err;
	}
	fclose(fh_out);
	goto exit_ok;

exit_err:
	if (fh_in) fclose(fh_in);
	if (fh_out) fclose(fh_out);
	unlink(fname_fdat_repack);
	ret = 1;
	goto exit_common;
exit_ok:
	sprintf(plog_global, "Done\n"); log_it(plog_global);
	ret = 0;
	// fallthru
exit_common:
	if (p_block_buf) {
		free((void *)p_block_buf);
	}
	return ret;
}
