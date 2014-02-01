//
// Sony NEX camera firmware toolbox
//
// written (reverse-engineered) by Paul Bartholomew, released under the GPL
// (originally based on "pr.exe" from nex-hack.info, with much more since then)
//
// Copyright (C) 2012-2013, nex-hack project
//
// This file "fwd_pack.c" is part of fwtool (http://www.nex-hack.info)
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

// fwd_pack contains the master unpack/repack and find_firmware_dat_in_zipfile

#include "config.h"
#include "fwt_names.h"
#include "fwt_util.h"
#include "fwd_pack.h"

#include "fwd_chunks.h"
#include "fdat_crypt.h"
#include "fdat_image.h"

#include "zipfile.h"
#include "tarfile.h"
#include "lzpt_io.h"


// added by kenan
int
do_repack(const char *dirname_in, const int fwt_level, const int fwt_majorver, const int fwt_minorver)
{
	//TODO delete if finished
	fprintf(stderr, "create: only repacking of level3 '*.mod.fsimg' files, encryption,\n" \
					"        and repacking to FirmwareData_*.dat implemented to date!\n\n");
	//

	char	enc_dirname_in[MAXPATH] = "";
	char	enc_dirname_out[MAXPATH] = "";
	strcpy(enc_dirname_in, dirname_in);
	strcat(enc_dirname_in, "/"FWD_SUB_OUTDIR);
	strcpy(enc_dirname_out, dirname_in);
	strcat(enc_dirname_out, "/"FWD_SUB_INDIR);

	char	dirname_lv2[MAXPATH] = "";
	char	dirname_lv3[MAXPATH] = "";
	char	dirname_lv4[MAXPATH] = "";
	char	dirname_lv5[MAXPATH] = "";
	char	dirname_lv6[MAXPATH] = "";
	sprintf(dirname_lv2, "%s/%s", enc_dirname_in, LEVEL2_SUBDIR);
	sprintf(dirname_lv3, "%s/%s", enc_dirname_in, LEVEL3_SUBDIR);
	sprintf(dirname_lv4, "%s/%s", enc_dirname_in, LEVEL4_SUBDIR);
	sprintf(dirname_lv5, "%s/%s", enc_dirname_in, LEVEL5_SUBDIR);
	sprintf(dirname_lv6, "%s/%s", enc_dirname_in, LEVEL6_SUBDIR);

	char	fname_fwdata_orig[MAXPATH] = "";
	char	fname_fwdata_save[MAXPATH] = "";
	char	fname_fwdata_repack[MAXPATH] = "";
	char	fname_fdat_enc[MAXPATH] = "";
	char	fname_fdat_dec[MAXPATH] = "";
	char	fname_fdat_orig[MAXPATH] = "";


	//TODO more stuff

	// repack FDAT.repack from level3 tar file and fs images
	sprintf(fname_fdat_orig, "%s/%s", dirname_lv2, BASENAME_FDAT_DECRYPTED);    // var reused!
	sprintf(fname_fdat_dec, "%s/%s", dirname_lv2, BASENAME_FDAT_REPACKED);
	printf("=== Repack '%s' files from '%s' ===\n===  to '%s' ===\n\n", FSIMAGE_EXT_MOD, dirname_lv3, fname_fdat_dec);
	sprintf(plog_global, "=== Repack '%s' files from '%s' ===\n===  to '%s' ===\n\n", FSIMAGE_EXT_MOD, dirname_lv3, fname_fdat_dec); log_it(plog_global);

	if (fdat_repack(fname_fdat_orig, fname_fdat_dec, dirname_lv3, fwt_majorver, fwt_minorver) != 0) {
		fprintf(stderr, "fdat_repack() returned error!\n");
		return 1;
	}

	// encrypt the FDAT dec chunk
	sprintf(fname_fdat_orig, "%s/%s", dirname_lv2, BASENAME_FDAT_CHUNK);    // var reused!
	// sprintf(fname_fdat_dec, "%s/%s", dirname_lv2, BASENAME_FDAT_REPACKED); // put one step above
	sprintf(fname_fdat_enc, "%s/%s", dirname_lv2, BASENAME_FDAT_REENCRYPTED);
	printf("=== Encrypt '%s' file ===\n===  to '%s' ===\n===  with method defined by'%s' ===\n\n",
		fname_fdat_dec, fname_fdat_enc, fname_fdat_orig);
	sprintf(plog_global, "=== Encrypt '%s' file ===\n===  to '%s' ===\n===  with method defined by'%s' ===\n\n",
		fname_fdat_dec, fname_fdat_enc, fname_fdat_orig); log_it(plog_global);

	if (fdat_encrypt_file(fname_fdat_dec, fname_fdat_enc, fname_fdat_orig, NULL) != 0) {
		fprintf(stderr, "fdat_encrypt_file() returned error!\n");
		return 1;
	}

	// repack FirmwareData_*.dat
	printf("=== Repack '%s' file ===\n===  to '%s' ===\n\n", fname_fdat_enc, dirname_lv2);
	sprintf(plog_global, "\n=== Repack '%s' file ===\n===  to '%s' ===\n", fname_fdat_enc, dirname_lv2); log_it(plog_global);
	if (fwdata_repack_chunks(fname_fdat_enc, dirname_lv2) != 0) {
		fprintf(stderr, "fwdata_repack_chunks() returned error!\n");
		return 1;
	}

	// save orig fwdata and move repack fwdata
	sprintf(fname_fwdata_orig, "%s/%s", enc_dirname_out, FWD_ORIG_GEN_NAME);
	sprintf(fname_fwdata_save, "%s/%s%s", enc_dirname_out, FWD_ORIG_GEN_NAME, FWD_SAVE_SUFFIX);
	if (rename(fname_fwdata_orig, fname_fwdata_save) != 0) {
		fprintf(stderr, "do_repack() rename orig fwdata returned error!\n");
		return 1;
	}

	// move repack fwdata
	sprintf(fname_fwdata_repack, "%s/%s", dirname_lv2, FWD_REPACKED_NAME);
	sprintf(fname_fwdata_orig, "%s/%s", enc_dirname_out, FWD_REPACKED_NAME);
	if (rename(fname_fwdata_repack, fname_fwdata_orig) != 0) {
		fprintf(stderr, "do_repack() move repack fwdata returned error!\n");
		return 1;
	}
	printf("=== Moved modified Firmware '%s' file ===\n===  to '%s' ===\n===  saving original Firmware to '%s' ===\n",
			fname_fwdata_repack, fname_fwdata_orig,fname_fwdata_save);
	sprintf(plog_global, "\n=== Moved modified Firmware '%s' file ===\n===  to '%s' ===\n===  saving original Firmware to '%s' ===\n",
			fname_fwdata_repack, fname_fwdata_orig,fname_fwdata_save); log_it(plog_global);

	printf("\nSuccess!\n"); sprintf(plog_global, "\n===== Success! =====\n"); log_it(plog_global);
	return 0;
}

//
// Firmware unpack:
//
// The input file must be a Sony firmware update ".exe" file.
// This is a self-extracting archive, which contains the Windows firmware update software,
// plus a "FirmwareData*.dat" file (exact embedded filename differs, Sony firmware update
// software looks for "FirmwareData*.dat").
//
// The "FirmwareData*.dat" file consists of several 'chunks' - each identified with a
// 4-character "chunk ID" There are several "chunks" within the "FirmwareData*.dat" file,
// they are all written out for further use.
//
// but the only one we are interested in is the one called "FDAT".  That image is encrypted,
// and needs to be decrypted before further processing. The other chunks are written out to
// be used with repacking
//
// We decrypt the encrypted FDAT chunk. There are currently two different encryption methods
// known for "FDAT" chunks - the decrypt function tries both (to determine the correct method),
// then decrypts the file using the appropriate method.
//
// The decrypted "FDAT_dec.bin" file contains a "firmware image" (a .tar file), and one
// (or more?) "filesystem" images (I've seen "cramfs" format image so far).  I've only seen
// a single "filesystem" image in an "FDAT" file so far, but the code seems to allow for
// multiple.
//
// We extract the "firmware" (.tar) to "FDAT_fwimage.bin", and each of the "filesystem"
// images to 'FDAT_fsimage_??.bin" ("??" replaced with numbers "00"-"nn" - again, I've only
// seen a single filesystem image so far).
//
// Next, if we recognize the "firmware" image ("FDAT_fwimage.bin") as a .tar file, we extract
// the contents (to "FDAT_fwimage.untar_out/" directory).
//
// So, at this point, we've got an "FDAT_fsimage_00.bin" (cramfs image) that (so far)
// we haven't extracted the contents from.
//
// And, we've got an "FDAT_fwimage.untar_out/" directory, with the extracted contents of the
// firmware .tar.
//
//
// Here, there is a difference between "first wave" and "second wave" firmware.
//
// "First wave" firmware (.tar expanded) contains "initrd" and "rootfs" images
// ("0800_appli/usr2/boot/initrd.img" and "0800_appli_usr2/boot/rootfs.img").
//
// The "initrd.img" is a standard "ext2" filesystem image.  Mounting this image "loopback"
// is straightforward under Linux.
//
// The "rootfs.img" is an lz77-compressed "cramfs" image.  It requires special
// (Sony-modified) cramfs drivers to mount under Linux.
//
//
// "Second wave" firmware contains "LZPT" image files:
// "0700_part_image/dev/nflasha3" (LZPT-compressed FAT filesystem),
// "0700_part_image/dev/nflasha8" (LZPT-compressed ext2 filesystem).
// The nex5r/nex6 and later may contain more "LZPT" images:
// "0700_part_image/dev/nflasha9" (LZPT-compressed ext2 filesystem),
// "0700_part_image/dev/nflashb1" (LZPT-compressed ext2 filesystem)
// and beginning with a99
// "0700_part_image/dev/nflasha5" (WBI raw filesystem) preconfigured
// "warm boot image" contained in the fw update.
//
// The LZPT are lz77-compressed flash partitions, which appear to be Sony-proprietary.
// and they are decompressed to FAT or ext2 images, that may be loop mounted under Linux.
//
// *** NOTE ***
//
// At the moment, this code will decrypt/unpack the FDAT image, and extract the contents
// of the firmware .tar file.
//
// It then looks for the "second wave" "nflash[a[389]|b1]" files, and (if found) will
// LZPT-decompress them to "nflash*_mount_fstyp.img".
//
// At the moment, the primary "cramfs" image (the "filesystem" image from the FDAT file)
// is NOT expanded. Nor are the "first wave" initrd/rootfs images and further compressed
// parts like the axfs fs etc.
// And, although decompressed to the top-level, the "second wave" "nflash*_mount_fstyp.img"
// files are not expanded any further.
//

int
do_unpack(const char *fname_exefile_in, const char *dest_name, const int fwt_level)
{
	char	zip_dirname_out[MAXPATH] = "";
	char	zip_extracted_dir[MAXPATH] = "";
	char	zip_extracted_fullname[MAXPATH] = "";
	char	inzip_fname_firmware_dat[MAXPATH] = "";
	char	inzip_fbase_fwdat[MAXPATH] = "";
	char	inzip_fdir_fwdat[MAXPATH] = "";
	char	inzip_fbasedir_fwdat[MAXPATH] = "";
	char	fname_fwdata_generic[MAXPATH] = "";
	char	fname_updater_log[MAXPATH] = "";
	FILE	*pulog;

	char	dirname_lv2[MAXPATH] = "";
	char	dirname_lv3[MAXPATH] = "";
	char	dirname_lv4[MAXPATH] = "";
	char	dirname_lv5[MAXPATH] = "";
	char	dirname_lv6[MAXPATH] = "";

	char	fname_fdat_enc[MAXPATH] = "";
	char	fname_fdat_dec[MAXPATH] = "";
	char	fname_fdat_head[MAXPATH] = "";
	char	fname_fdat_fw[MAXPATH] = "";
	char	fname_fdat_fs[MAXPATH] = "";
	char	dirname_fdat_fw_untar[MAXPATH] = "";
	char	fname_lzpt_in_fname[MAXPATH] = "";
	char	fname_lzpt_out_fname[MAXPATH] = "";

	int	i;
	int	fs_image_count = 0;
	int show_extract_names = 1;

	// check to see if input file is a zip archive
	// (Sony firmware update .exe self-extracting autorun archive)
	if (is_zipfile(fname_exefile_in)) {

		// look for "FirmwareData*.dat" inside .zip archive
		if (find_firmware_dat_in_zipfile(fname_exefile_in, inzip_fname_firmware_dat, sizeof(inzip_fname_firmware_dat)-1,
			inzip_fbase_fwdat, sizeof(inzip_fbase_fwdat)-1, inzip_fdir_fwdat, sizeof(inzip_fdir_fwdat)-1, inzip_fbasedir_fwdat, sizeof(inzip_fbasedir_fwdat)-1) != 0) {

			fprintf(stderr, "do_unpack() FAILED to find firmware in zip!\n");
			return 1;
		}

		//debug
		sprintf(plog_global, "\ndo_unpack is_zip: exe_in %s,\n fullfile %s,\n basename %s,\n fdir     %s,\n fbasedir %s.\n\n",
				fname_exefile_in, inzip_fname_firmware_dat, inzip_fbase_fwdat, inzip_fdir_fwdat, inzip_fbasedir_fwdat);
		log_it(plog_global);
		//debug

		// create dest_name output directory, TODO look into zipfile.c
		if(dest_name) {
			//(void)mkdir(dest_name, 0777);
			mkdir(dest_name);
		}

		// extract complete zip archive, needed for modify mode
		printf("=== Extract update zip to '%s' ===\n", inzip_fbasedir_fwdat);
		sprintf(plog_global, "=== Extract update zip to '%s' ===\n", inzip_fbasedir_fwdat); log_it(plog_global);
		if (zipfile_extract_all((char *)fname_exefile_in, (char *)dest_name, (char *)zip_extracted_dir, zip_extracted_fullname, show_extract_names) != 0) {
			fprintf(stderr, "do_unpack() FAILED to extract zip!\n");
			return 1;
		}

		if(dest_name) {
			char	l_inzip_fbasedir_fwdat[MAXPATH];
			sprintf(l_inzip_fbasedir_fwdat,"%s/%s", dest_name, inzip_fbasedir_fwdat);
			strcpy(inzip_fbasedir_fwdat, l_inzip_fbasedir_fwdat);
			char	l_inzip_fname_firmware_dat[MAXPATH];
			sprintf(l_inzip_fname_firmware_dat,"%s/%s", dest_name, inzip_fname_firmware_dat);
			strcpy(inzip_fname_firmware_dat, l_inzip_fname_firmware_dat);
			char	l_inzip_fdir_fwdat[MAXPATH];
			sprintf(l_inzip_fdir_fwdat,"%s/%s", dest_name, inzip_fdir_fwdat);
			strcpy(inzip_fdir_fwdat, l_inzip_fdir_fwdat);
		}
		strcpy(fname_updater_log, inzip_fbasedir_fwdat);
		strcat(inzip_fbasedir_fwdat, "/"FWD_SUB_OUTDIR);
		strcpy(zip_dirname_out, inzip_fbasedir_fwdat);	//TODO names
		strcpy(zip_extracted_fullname, inzip_fname_firmware_dat);

		//debug
		sprintf(plog_global, "\ndo_unpack is_zip complete: in %s,\n fullfile %s,\n basename %s,\n dir in   %s,\n dir out  %s,\n zipdir   %s.\n\n",
				fname_exefile_in, inzip_fname_firmware_dat, inzip_fbase_fwdat, inzip_fdir_fwdat, zip_dirname_out, zip_extracted_dir);
		log_it(plog_global);
		//debug

		// assure "FirmwareUpdater.log" exists for use in debugging fwupdate of modified fw
		strcat(fname_updater_log, "/"UPDATER_LOG);
		if ((pulog = fopen(fname_updater_log, "a"))) {
			fclose(pulog);
		}
		else fprintf(stderr, "do_unpack() FAILED to access or create %s!\n", fname_updater_log);

		// rename orig fwdata to generic name for ease of repack
		sprintf(fname_fwdata_generic, "%s/%s", inzip_fdir_fwdat, FWD_ORIG_GEN_NAME);
		unlink(fname_fwdata_generic);
		if (rename(zip_extracted_fullname, fname_fwdata_generic) != 0) {
			fprintf(stderr, "do_unpack() rename fwdata to generic name returned error!\n");
			return 1;
		}
		// code below (which expects FirmwareData*.dat input) will now use this extracted image
		// printf("Done\n");
	}
	else {
		fprintf(stderr, "do_unpack() no Update exe file specified!\n");
		return 1;
	}

	// create nexhack output directory
	//(void)mkdir(zip_dirname_out, 0777);
	mkdir(zip_dirname_out);

	// create level2 output directory
	sprintf(dirname_lv2, "%s/%s", zip_dirname_out, LEVEL2_SUBDIR);
	//(void)mkdir(dirname_lv2, 0777);
	mkdir(dirname_lv2);

	// extract the (encrypted) "FDAT" chunk from the FirmwareData.dat file
	sprintf(fname_fdat_enc, "%s/%s", dirname_lv2, BASENAME_FDAT_CHUNK);
	printf("=== Extract encrypted FDAT record to '%s' ===\n", fname_fdat_enc);
	sprintf(plog_global, "=== Extract encrypted FDAT record to '%s' ===\n", fname_fdat_enc); log_it(plog_global);
	if (fwdata_unpack_chunks(fname_fwdata_generic, dirname_lv2) != 0) {
		fprintf(stderr, "fwdata_unpack_chunks() returned error!\n");
		return 1;
	}

	// decrypt the FDAT chunk
	sprintf(fname_fdat_dec, "%s/%s", dirname_lv2, BASENAME_FDAT_DECRYPTED);
	printf("=== Decrypt FDAT record to '%s' ===\n", fname_fdat_dec);
	sprintf(plog_global, "\n=== Decrypt FDAT record to '%s' ===\n", fname_fdat_dec); log_it(plog_global);
	if (fdat_decrypt_file(fname_fdat_enc, fname_fdat_dec, NULL) != 0) {
		fprintf(stderr, "fdat_decrypt_file() returned error!\n");
		return 1;
	}

	// we now have a decrypted FDAT file: extract the "firmware" (tar)
	// create level3 output directory
	sprintf(dirname_lv3, "%s/%s", zip_dirname_out, LEVEL3_SUBDIR);
	//(void)mkdir(dirname_lv3, 0777);
	mkdir(dirname_lv3);

    // extract header for later use in repack
    sprintf(fname_fdat_head, "%s/%s", dirname_lv3, BASENAME_FDAT_HEADER);
	sprintf(plog_global, "=== Extract FDAT header to '%s' ===\n", fname_fdat_head); log_it(plog_global);
	if (fdat_header_tofile(fname_fdat_dec, fname_fdat_head) != 0) {
		fprintf(stderr, "fdat_header_tofile() returned error!\n");
		return 1;
	}

    // extract main fw image tar
	sprintf(fname_fdat_fw, "%s/%s", dirname_lv3, BASENAME_FDAT_FIRMWARE_TAR);
	printf("=== Extract FDAT firmware image to '%s' ===\n", fname_fdat_fw);
	sprintf(plog_global, "=== Extract FDAT firmware image to '%s' ===\n", fname_fdat_fw); log_it(plog_global);
	if (fdat_extract_firmware_image(fname_fdat_dec, fname_fdat_fw) != 0) {
		fprintf(stderr, "fdat_extract_firmware_image() returned error!\n");
		return 1;
	}

	// extract each of the FDAT "filesystem" images (at the moment, I'm aware of a single
	// "cramfs" filesystem image, but there could be up to 28 fs images)
	fs_image_count = fdat_fs_image_count(fname_fdat_dec);
	for (i = 0; i < fs_image_count; i++) {
		// header sometimes has zero-length entry for (non-existent) second fs image,
		// so skip zero-length without error
		if (fdat_fs_image_length(fname_fdat_dec, i) > 0) {
			sprintf(fname_fdat_fs, "%s/%s%2.2d%s", dirname_lv3, BASENAME_FDAT_FS_PREFIX, i, FSIMAGE_EXT);
			printf("=== Extract FDAT filesystem image #%d to '%s' ===\n", i, fname_fdat_fs);
			sprintf(plog_global, "=== Extract FDAT filesystem image #%d to '%s' ===\n", i, fname_fdat_fs); log_it(plog_global);
			if (fdat_extract_fs_image(fname_fdat_dec, i, fname_fdat_fs) != 0) {
				fprintf(stderr, "fdat_extract_fs_image(%d) returned error!\n", i);
				return 1;
			}
		}
	}

	// create level4 output directory
	sprintf(dirname_lv4, "%s/%s", zip_dirname_out, LEVEL4_SUBDIR);
	//(void)mkdir(dirname_lv4, 0777);
	mkdir(dirname_lv4);

	// if the extracted firmware file is a .tar file (should be), then extract all of the files
	// to a sub-dir (same basename as firmware image, with ".untar" added)
	if (is_tarfile(fname_fdat_fw)) {
		sprintf(dirname_fdat_fw_untar, "%s/%s", dirname_lv4, BASENAME_FDAT_FIRMWARE_BASE);
		printf("=== Extract FDAT firmware .tar to '%s' ===\n", dirname_fdat_fw_untar);
		sprintf(plog_global, "\n=== Extract FDAT firmware .tar to '%s' ===\n", dirname_fdat_fw_untar); log_it(plog_global);

		if (tarfile_extract_all(fname_fdat_fw, dirname_fdat_fw_untar, 1) != 0) {
				fprintf(stderr, "tarfile_extract_all() returned error!\n");
				return 1;
		}

		// here, we've extracted the firmware .tar contents.  we want to find/decompress any "LZPT"
		// images (which only exist in "second wave" firmware).  for now, we'll be lazy, and use
		// hard-coded paths to known locations of LZPT files - if they exist, we'll decompress them.

		// create level5 output directory
		sprintf(dirname_lv5, "%s/%s", zip_dirname_out, LEVEL5_SUBDIR);
		//(void)mkdir(dirname_lv5, 0777);
		mkdir(dirname_lv5);

		sprintf(fname_lzpt_in_fname, "%s/%s", dirname_fdat_fw_untar, FNAME_FWV2_LZPT_NFA3_FAT_IMAGE);
		if (is_lzpt_file(fname_lzpt_in_fname)) {
			printf("=== Decompress LZPT NFLASHA3 image ===\n");
			sprintf(plog_global, "\n=== Decompress LZPT NFLASHA3 image ===\n"); log_it(plog_global);
			sprintf(fname_lzpt_out_fname, "%s/%s", dirname_lv5, FNAME_FWV2_NFA3_FAT_IMAGE_OUT);
			if (lzpt_decompress_file(fname_lzpt_in_fname, fname_lzpt_out_fname) != 0) {
				fprintf(stderr, "lzpt_decompress_file(NFLASHA3) returned error!\n");
				return 1;
			}
		}
		sprintf(fname_lzpt_in_fname, "%s/%s", dirname_fdat_fw_untar, FNAME_FWV2_LZPT_NFA8_EXT2_IMAGE);
		if (is_lzpt_file(fname_lzpt_in_fname)) {
			printf("=== Decompress LZPT NFLASHA8 image ===\n");
			sprintf(plog_global, "\n=== Decompress LZPT NFLASHA8 image ===\n"); log_it(plog_global);
			sprintf(fname_lzpt_out_fname, "%s/%s", dirname_lv5, FNAME_FWV2_NFA8_EXT2_IMAGE_OUT);
			if (lzpt_decompress_file(fname_lzpt_in_fname, fname_lzpt_out_fname) != 0) {
				fprintf(stderr, "lzpt_decompress_file(NFLASHA8) returned error!\n");
				return 1;
			}
		}
		sprintf(fname_lzpt_in_fname, "%s/%s", dirname_fdat_fw_untar, FNAME_FWV2_LZPT_NFA9_EXT2_IMAGE);
		if (is_lzpt_file(fname_lzpt_in_fname)) {
			printf("=== Decompress LZPT NFLASHA9 image ===\n");
			sprintf(plog_global, "\n=== Decompress LZPT NFLASHA9 image ===\n"); log_it(plog_global);
			sprintf(fname_lzpt_out_fname, "%s/%s", dirname_lv5, FNAME_FWV2_NFA9_EXT2_IMAGE_OUT);
			if (lzpt_decompress_file(fname_lzpt_in_fname, fname_lzpt_out_fname) != 0) {
				fprintf(stderr, "lzpt_decompress_file(NFLASHA9) returned error!\n");
				return 1;
			}
		}
		sprintf(fname_lzpt_in_fname, "%s/%s", dirname_fdat_fw_untar, FNAME_FWV2_LZPT_NFB1_EXT2_IMAGE);
		if (is_lzpt_file(fname_lzpt_in_fname)) {
			printf("=== Decompress LZPT NFLASHB1 image ===\n");
			sprintf(plog_global, "\n=== Decompress LZPT NFLASHB1 image ===\n"); log_it(plog_global);
			sprintf(fname_lzpt_out_fname, "%s/%s", dirname_lv5, FNAME_FWV2_NFB1_EXT2_IMAGE_OUT);
			if (lzpt_decompress_file(fname_lzpt_in_fname, fname_lzpt_out_fname) != 0) {
				fprintf(stderr, "lzpt_decompress_file(NFLASHB1) returned error!\n");
				return 1;
			}
		}

		// create level6 output directory
		sprintf(dirname_lv6, "%s/%s", zip_dirname_out, LEVEL6_SUBDIR);
		//(void)mkdir(dirname_lv6, 0777);
		mkdir(dirname_lv6);

	}
	printf("Success!\n"); sprintf(plog_global, "\n===== Success! =====\n"); log_it(plog_global);
	return 0;
}


int
find_firmware_dat_in_zipfile(const char *fname_zip, char *p_fname_outbuf, int sz_fname_outbuf,
	char *p_fbase_outbuf, int sz_fbase_outbuf, char *p_fdir_outbuf, int sz_fdir_outbuf, char *p_fbasedir_outbuf, int sz_fbasedir_outbuf)
{
	char	l_fname[256];
	int	ret = 0;
	zip_handle	zh;
	char	*p_fwdata, *p_ext;
	int	len, lenbase, lendir, lenbasedir;

	p_fname_outbuf[0] = '\0';

	if (!(zh = zipfile_open(fname_zip))) {
		return 1;
	}
	if ((ret = zipfile_find_first(zh, l_fname, sizeof(l_fname)-1, NULL, NULL))) {
		zipfile_close(zh);
		return 1;
	}
	while(ret == 0) {
		// look for file that starts with "FirmwareData" and ends with ".dat"
		if ((p_fwdata = strstr(l_fname, FWD_NAME_PREFIX))) {
			if (!strchr(p_fwdata, '/') && (p_ext = strstr(p_fwdata, FWD_NAME_SUFFIX))) {
				len = strlen(l_fname);
				if (len > (sz_fname_outbuf-1)) {
					len = sz_fname_outbuf-1;
				}
				strncpy(p_fname_outbuf, l_fname, len);
				p_fname_outbuf[len] = '\0';

				lenbase = strlen(p_fwdata);
				if (lenbase > (sz_fbase_outbuf-1)) {
					lenbase = sz_fbase_outbuf-1;
				}
				strncpy(p_fbase_outbuf, p_fwdata, lenbase);
				p_fbase_outbuf[lenbase] = '\0';

				lendir = strlen(l_fname) - strlen(p_fwdata) -1;
				if (lendir > (sz_fdir_outbuf-1)) {
					lendir = sz_fdir_outbuf-1;
				}
				strncpy(p_fdir_outbuf, l_fname, lendir);
				p_fdir_outbuf[lendir] = '\0';

				//TODO quick'n'dirty: "/Resource" is the subdir, len=9
				lenbasedir = strlen(l_fname) - strlen(p_fwdata) - sizeof(FWD_SUB_INDIR) -1;
				if (lenbasedir > (sz_fbasedir_outbuf-1)) {
					lenbasedir = sz_fbasedir_outbuf-1;
				}
				strncpy(p_fbasedir_outbuf, l_fname, lenbasedir);
				p_fbasedir_outbuf[lenbasedir] = '\0';

				zipfile_close(zh);
				return 0;
			}
		}
		ret = zipfile_find_next(zh, l_fname, sizeof(l_fname)-1, NULL, NULL);
	}
	zipfile_close(zh);
	return 1;
}
