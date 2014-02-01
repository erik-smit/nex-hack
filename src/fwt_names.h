//
// Sony NEX camera firmware toolbox
//
// written (reverse-engineered) by Paul Bartholomew, released under the GPL
// (originally based on "pr.exe" from nex-hack.info, with much more since then)
//
// Copyright (C) 2012-2013, nex-hack project
//
// This file "fwt_names.h" is part of fwtool (http://www.nex-hack.info)
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

#ifndef FWT_NAMES_H
#define FWT_NAMES_H

// version string and global logfile
#define VERSION	"v0.6final (2013-08-07)"
#define LOGFILE "fwtool_log.txt"

// logfile name for Sony FirmwareUpdater
#define UPDATER_LOG	"FirmwareUpdater.log"	// must be set to this name!

// suffix of UpdaterPackage
#define	EXE_SUFFIX	".exe"

// level1 dirnames
#define FWD_SUB_INDIR	"Resource"	// must be set to this name!
#define FWD_SUB_OUTDIR	"nexhack"
// level1 names inside sfx autoexec zip
#define	FWD_NAME_PREFIX	"FirmwareData"	// must be set to this name!
#define	FWD_NAME_SUFFIX	".dat"	// must be set to this name!
#define	FWD_ORIG_GEN_NAME	FWD_NAME_PREFIX "_Original" FWD_NAME_SUFFIX
#define	FWD_REPACKED_NAME	FWD_NAME_PREFIX "_NexHack" FWD_NAME_SUFFIX
#define	FWD_SAVE_SUFFIX	".save"

// level2 dirnames
#define	LEVEL2_SUBDIR	"level2"
// level2 chunks in FirmwareData*.dat and FDAT decrypted
#define	FWD_CHUNK_TOC_NAME	"fwd_chunk_toc.txt"

#define	FWD_CHUNK_NAME_PREFIX	"fwd_"
#define	FWD_CHUNK_NAME_SUFFIX	"_chunk"
#define	FWD_CHUNK_NAME_EXT	".DONT_TOUCH"

#define	FWD_CHUNK_NAME_FMT_NOEXT	FWD_CHUNK_NAME_PREFIX "%s" FWD_CHUNK_NAME_SUFFIX
#define	FWD_CHUNK_NAME_FMT	FWD_CHUNK_NAME_FMT_NOEXT FWD_CHUNK_NAME_EXT

#define	FWD_CHUNK_NAME_CONST_ID_NOEXT(id)	FWD_CHUNK_NAME_PREFIX id FWD_CHUNK_NAME_SUFFIX
#define	FWD_CHUNK_NAME_CONST_ID(id)	FWD_CHUNK_NAME_CONST_ID_NOEXT(id) FWD_CHUNK_NAME_EXT

//TODO hard coded chunk names for repack, should read in from toc file
#define	BASENAME_DATV_CHUNK	FWD_CHUNK_NAME_PREFIX "DATV" FWD_CHUNK_NAME_SUFFIX FWD_CHUNK_NAME_EXT
#define	BASENAME_PROV_CHUNK	FWD_CHUNK_NAME_PREFIX "PROV" FWD_CHUNK_NAME_SUFFIX FWD_CHUNK_NAME_EXT
#define	BASENAME_UDID_CHUNK	FWD_CHUNK_NAME_PREFIX "UDID" FWD_CHUNK_NAME_SUFFIX FWD_CHUNK_NAME_EXT
#define	BASENAME_FDAT_CHUNK	FWD_CHUNK_NAME_PREFIX "FDAT" FWD_CHUNK_NAME_SUFFIX FWD_CHUNK_NAME_EXT
// hard coded chunk names

#define	BASENAME_FDAT_DECRYPTED	"FDAT_decrypt.bin"
#define	BASENAME_FDAT_REPACKED	"FDAT_repack.bin"
#define BASENAME_FDAT_REENCRYPTED	"FDAT_reencrypt.bin"

// level3 dirnames
#define	LEVEL3_SUBDIR	"level3"
// level3 tar and filesystems inside FDAT
#define BASENAME_FDAT_HEADER  "FDAT_header" FWD_CHUNK_NAME_EXT

#define	BASENAME_FDAT_FIRMWARE_BASE	"FDAT_fw"
#define	BASENAME_FDAT_FIRMWARE_TAR	BASENAME_FDAT_FIRMWARE_BASE ".tar"
#define	BASENAME_FDAT_FIRMWARE_TAR_MOD	BASENAME_FDAT_FIRMWARE_BASE ".mod.tar"

#define FSIMAGE_EXT	".fsimg"
#define	BASENAME_FDAT_FS_PREFIX	"FDAT_fs"
#define FSIMAGE_EXT_MOD	".mod" FSIMAGE_EXT

#define FWT_NOMODELVERSION	-1
#define	FWT_MAXMAJORVER	127
#define	FWT_MAXMINORVER	127

// level4 dirnames
#define	LEVEL4_SUBDIR	"level4"
// level4 tar and cramfs

// level5 dirnames
#define	LEVEL5_SUBDIR	"level5"
// level5 fs images inside tar
// hard coded fs image names
#define	FNAME_FWV1_INITRD_IMAGE	"0800_appli_usr2/boot/initrd.img"
#define	FNAME_FWV1_ROOTFS_IMAGE	"0800_appli_usr2/boot/rootfs.img"

#define	FNAME_FWV2_LZPT_NFA3_FAT_IMAGE	"0700_part_image/dev/nflasha3"
#define	FNAME_FWV2_NFA3_FAT_IMAGE_OUT	"nflasha3_system_fat" FSIMAGE_EXT
#define	FNAME_FWV2_WBI_NFA5_WBI_IMAGE	"0700_part_image/dev/nflasha5"
#define	FNAME_FWV2_NFA5_WBI_IMAGE_OUT	"nflasha5_wbi_wbi" FSIMAGE_EXT
#define	FNAME_FWV2_LZPT_NFA8_EXT2_IMAGE	"0700_part_image/dev/nflasha8"
#define	FNAME_FWV2_NFA8_EXT2_IMAGE_OUT	"nflasha8_usr_ext2" FSIMAGE_EXT
#define	FNAME_FWV2_LZPT_NFA9_EXT2_IMAGE	"0700_part_image/dev/nflasha9"
#define	FNAME_FWV2_NFA9_EXT2_IMAGE_OUT	"nflasha9_android_ext2" FSIMAGE_EXT
#define	FNAME_FWV2_LZPT_NFB1_EXT2_IMAGE	"0700_part_image/dev/nflashb1"
#define	FNAME_FWV2_NFB1_EXT2_IMAGE_OUT	"nflashb1_android_and_res_ext2" FSIMAGE_EXT
// hard coded fs image names

// level6 dirnames
#define	LEVEL6_SUBDIR	"level6"
// level6 images inside images in tar (2.gen only)


#endif // FWT_NAMES_H
