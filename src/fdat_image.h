//
// Sony NEX camera firmware toolbox
//
// written (reverse-engineered) by Paul Bartholomew, released under the GPL
// (originally based on "pr.exe" from nex-hack.info, with much more since then)
//
// Copyright (C) 2012-2013, nex-hack project
//
// This file "fdat_image.h" is part of fwtool (http://www.nex-hack.info)
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

#ifndef FDAT_IMAGE_H
#define FDAT_IMAGE_H

#define	FDAT_EXTRACT_IOBUF_SIZE		1048576

#define	FDAT_IMAGE_MAGIC		"UDTRFIRM"
#define	FDAT_IMAGE_MAGIC_LEN	(sizeof(FDAT_IMAGE_MAGIC)-1)
#define	FDAT_IMAGE_MAGIC_OFS	0

#define	MAX_FDAT_FS_IMAGES		28
#define	FDAT_HDR_VERSION_LEN	4

typedef struct tagFDAT_ENC_BLOCK_HDR {
	unsigned short	hdr_csum;
	unsigned short	hdr_len_and_flags;
} FDAT_ENC_BLOCK_HDR;

typedef struct tagFDAT_FS_IMAGE_DESC {
	unsigned char	ffid_ident;									// 0000
	unsigned char	ffid_unknown_01[3];							// 0001
	unsigned int	ffid_fs_offset;								// 0004 - offset to filesystem image
	unsigned int	ffid_fs_length;								// 0008 - length of filesystem image
	unsigned int	ffid_unknown_02;							// 000c
} FDAT_FS_IMAGE_DESC;

typedef struct tagFDAT_IMAGE_HEADER {
	unsigned char	fih_magic[FDAT_IMAGE_MAGIC_LEN];			// 0000
	unsigned int	fih_header_crc;								// 0008 - CRC32 of remainder of header (000c-01ff)
	unsigned char	fih_image_version[FDAT_HDR_VERSION_LEN];	// 000c - "0100" - I assume this is a version string
	unsigned char	fih_fw_mode_type;							// 0010 - ModeType: "P" (??? "U" in all fw ?)
	unsigned char	fih_fw_mode_type_unknown_pad[3];			// 0011
	unsigned char	fih_fw_luw_flag;							// 0014 - LUW flag: "N"
	unsigned char	fih_fw_luw_flag_unknown_pad[3];				// 0015
	unsigned int	fih_unknown_01;								// 0018 - unknown
	unsigned int	fih_unknown_02;								// 001c - unknown
	unsigned char	fih_version_minor;							// 0020 - minor version: FF is sign-extended on read
	unsigned char	fih_version_major;							// 0021 - major version: FF is sign-extended on read
	unsigned char	fih_version_unknown_pad[2];					// 0022
	unsigned int	fih_model;									// 0024 - body or lens model (see table in fwt_util.c)
	unsigned int	fih_unknown_03;								// 0028 - unknown u32
	unsigned int	fih_unknown_04;								// 002c
	unsigned int	fih_fw_offset;								// 0030 - offset to firmware image (.tar)
	unsigned int	fih_fw_length;								// 0034 - length of firmware image (.tar)
	unsigned int	fih_fs_image_count;							// 0038 - number of fs images starting at 0040
	unsigned int	fih_unknown_05;								// 003c

	FDAT_FS_IMAGE_DESC	fih_fs_image_info[MAX_FDAT_FS_IMAGES];

} FDAT_IMAGE_HEADER;


extern	int	fdat_extract_firmware_image(const char *fname_fdat, const char *fname_fw_image);	// the ".tar" file inside the FDAT image

extern	int	fdat_fs_image_count(const char *fname_fdat);
extern	int	fdat_fs_image_length(const char *fname_fdat, int image_idx);
extern	int	fdat_extract_fs_image(const char *fname_fdat, int image_idx, const char *fname_fs_image);
extern  int	fdat_header_tofile(const char *fname_fdat, const char *fname_fdat_header);
extern  int	fdat_repack(const char *fname_fdat, const char *fname_fdat_repack, const char *dirname_parts, const int fwt_majorver, const int fwt_minorver);


#endif // FDAT_IMAGE_H
