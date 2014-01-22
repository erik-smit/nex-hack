//
// Sony NEX camera firmware toolbox
//
// written (reverse-engineered) by Paul Bartholomew, released under the GPL
// (originally based on "pr.exe" from nex-hack.info, with much more since then)
//
// Copyright (C) 2012-2013, nex-hack project
//
// This file "tarfile.c" is part of fwtool (http://www.nex-hack.info)
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

#define	TARFILE_C

#include "config.h"
#include "fwt_util.h"
#include <errno.h>

#include "archive.h"
#include "archive_entry.h"

#include "tarfile.h"


#define	ENTRY_ATTR_FNAME	".attr_unpack.txt"


static tar_handle
_create_th(struct archive *a)
{
	tar_handle	retval = NULL;

	if ((retval = malloc(sizeof(tar_handle_struct)))) {
		memset((void *)retval, 0, sizeof(tar_handle_struct));
		retval->a = a;
	}

	return retval;
}


static void
_free_th(tar_handle th)
{
	if (th) {
		free((void *)th);
	}
}


tar_handle
tarfile_open(const char *fname)
{
	struct archive *a = NULL;

	if ((a = archive_read_new())) {
		archive_read_support_format_tar(a);
		if (archive_read_open_filename(a, fname, TAR_IOBUF_SIZE) == 0) {
			return _create_th(a);
		} else {
			archive_read_free(a);
		}
	}
	return NULL;
}


void
tarfile_close(tar_handle th)
{
	if (th) {
		if (th->a) {
			archive_read_close(th->a);
			archive_read_free(th->a);
		}
		_free_th(th);
	}

}


int
is_tarfile(const char *fname)
{
	tar_handle	th;

	if ((th = tarfile_open(fname))) {
		tarfile_close(th);
		return 1;
	} else {
		return 0;
	}
}


int
tarfile_find_first(tar_handle th, char *p_fname_buf, int sz_fname_buf, unsigned long *p_uncomp_size, unsigned int *p_crc32)
{
	// FIXME: can I seek back to first file??
	return tarfile_find_next(th, p_fname_buf, sz_fname_buf, p_uncomp_size, p_crc32);
}


int
tarfile_find_next(tar_handle th, char *p_fname_buf, int sz_fname_buf, unsigned long *p_uncomp_size, unsigned int *p_crc32)
{
	int	len;
	struct archive_entry *entry;
	int	ret;
	const char	*p_entry_fname;

	ret = archive_read_next_header(th->a, &entry);
	if (ret == ARCHIVE_EOF) {
		return 1;
	}
	if (ret != ARCHIVE_OK) {
		return -1;
	}

	if (!(p_entry_fname = archive_entry_pathname(entry))) {
		return -1;
	}
#if	0
	// 'mode' is 'perm | filetype'
	printf("FNAME: '%s'\n", p_entry_fname);
	printf("\tUID: 0x%lx\n", (unsigned long)archive_entry_uid(entry));
	printf("\tGID: 0x%lx\n", (unsigned long)archive_entry_gid(entry));
	printf("\tPERM: 0x%lx\n", (unsigned long)archive_entry_perm(entry));
	printf("\tINODE: 0x%lx\n", (unsigned long)(archive_entry_ino_is_set(entry) ? archive_entry_ino(entry) : -1));
	printf("\tMODE: '%s'\n", archive_entry_strmode(entry));
	printf("\tTYPE: 0x%lx\n", (unsigned long)archive_entry_filetype(entry));
	printf("\tATIME/BTIME/CTIME/MTIME set: %d/%d/%d/%d\n",
		archive_entry_atime_is_set(entry),
		archive_entry_birthtime_is_set(entry),
		archive_entry_ctime_is_set(entry),
		archive_entry_mtime_is_set(entry));
	printf("\tHARDLINK: '%s'\n", archive_entry_hardlink(entry));
	printf("\tSYMLINK: '%s'\n", archive_entry_symlink(entry));
	printf("\tDEV: %d (%d,%d)\n", archive_entry_dev_is_set(entry), archive_entry_rdevmajor(entry), archive_entry_rdevminor(entry));
#endif
	len = strlen(p_entry_fname);
	if (len > (sz_fname_buf-1)) {
		len = sz_fname_buf-1;
	}
	if (p_fname_buf) {
		strncpy(p_fname_buf, p_entry_fname, len);
		p_fname_buf[len] = '\0';
	}
	return 0;
}


int
tarfile_list(tar_handle th)
{
	char	fname[256];
	int	ret = 0;

	if (!th) {
		return 1;
	}
	ret = tarfile_find_first(th, fname, sizeof(fname)-1, NULL, NULL);
	while(ret == 0) {
		printf("FNAME: '%s'\n", fname);
		ret = tarfile_find_next(th, fname, sizeof(fname)-1, NULL, NULL);
	}
	return ret;
}


static int
_tarfile_extract_create_dir(char *path)
{
	struct stat st;
	char *slash, *base;
	int r;

	/* Check for special names and just skip them. */
	slash = strrchr(path, '/');
	if (slash == NULL)
		base = path;
	else
		base = slash + 1;

	if (base[0] == '\0' ||
	    (base[0] == '.' && base[1] == '\0') ||
	    (base[0] == '.' && base[1] == '.' && base[2] == '\0')) {
		/* Don't bother trying to create null path, '.', or '..'. */
		if (slash != NULL) {
			*slash = '\0';
			r = _tarfile_extract_create_dir(path);
			*slash = '/';
			return r;
		}
		return 0;
	}

	/*
	 * Yes, this should be stat() and not lstat().  Using lstat()
	 * here loses the ability to extract through symlinks.  Also note
	 * that this should not use the a->st cache.
	 */
	if (stat(path, &st) == 0) {
		if (S_ISDIR(st.st_mode))
			return 0;
		if (unlink(path) != 0) {
			return 1;
		}
	} else if (errno != ENOENT && errno != ENOTDIR) {
		/* Stat failed? */
		return 1;
	} else if (slash != NULL) {
		*slash = '\0';
		r = _tarfile_extract_create_dir(path);
		*slash = '/';
		if (r != 0)
			return r;
	}
	// FIXME: try real mode here??
	//if (mkdir(path, 0777) == 0) {
	if (mkdir(path) == 0) {
		return 0;
	}

	/*
	 * Without the following check, a/b/../b/c/d fails at the
	 * second visit to 'b', so 'd' can't be created.  Note that we
	 * don't add it to the fixup list here, as it's already been
	 * added.
	 */
	if (stat(path, &st) == 0 && S_ISDIR(st.st_mode))
		return 0;

	return 1;
}


int
_tarfile_create_entry_attributes(const char *dirname)
{
	FILE	*fh;
	char	fname_buf[512];

	sprintf(fname_buf, "%s/%s", dirname, ENTRY_ATTR_FNAME);

	// create (and truncate if exists) new entry attributes file
	if (!(fh = fopen(fname_buf, "w"))) {
		return 1;
	}
	fclose(fh);
	return 0;
}


int
_tarfile_add_entry_attributes(const char *dirname, const char *entry_attr_string)
{
	FILE	*fh;
	char	fname_buf[512];

	sprintf(fname_buf, "%s/%s", dirname, ENTRY_ATTR_FNAME);

	// append to existing file
	if (!(fh = fopen(fname_buf, "a"))) {
		return 1;
	}
	fprintf(fh, "%s\n", entry_attr_string);
	fclose(fh);
	return 0;
}


static int
_tarfile_extract_current_entry(tar_handle th, const char *dirname_out, struct archive_entry *entry, int show_extract_names)
{
	const char	*entry_fname = archive_entry_pathname(entry);
	unsigned long	entry_uid = (unsigned long)archive_entry_uid(entry);
	unsigned long	entry_gid = (unsigned long)archive_entry_gid(entry);
	unsigned long	entry_mode = (unsigned long)archive_entry_mode(entry);
	const char	*entry_modestr = archive_entry_strmode(entry);
	char	entry_fullname[512] = {0};
	char	parent_dirname[512] = {0};
	char	entry_attr_string[512] = {0};
	char	entry_specialinfo_str[512] = {0};
	char	*p;
	char	*p_entry_basename = NULL;
	int	len;
	int	is_dir = 0, is_special = 0;
	int	ret;
	FILE	*fh_out = NULL;

	//
	// NOTE: we *assume* here that "entry_fname" has forward-slashes delimiting the directory
	// (basename of files can have '\' (esp in /dev/), so don't translate)
	//
//printf("FNAME: '%s'\n", entry_fname);
	if (dirname_out) {
		sprintf(entry_fullname, "%s/%s", dirname_out, entry_fname);
	} else {
		if ((entry_fname[0] == '/')) {
			strcpy(entry_fullname, entry_fname+1);
		} else {
			strcpy(entry_fullname, entry_fname);
		}
	}

	is_dir = 0;
	if (entry_modestr[0] == 'd') {
		is_dir = 1;
		p = strchr(entry_fullname, '\0');
		// remove possible trailing slash
		if ((p > entry_fullname) && (p[-1] == '/')) {
			p[-1] = '\0';
		}
	} else {
		is_dir = 0;
	}
	if ((p_entry_basename = strrchr(entry_fullname, '/'))) {
		++p_entry_basename;
		len = (p_entry_basename - entry_fullname - 1);
		strncpy(parent_dirname, entry_fullname, len);
		parent_dirname[len] = '\0';
	} else {
		p_entry_basename = entry_fullname;
		parent_dirname[0] = '\0';
	}

	entry_specialinfo_str[0] = '\0';
	switch(entry_modestr[0]) {
	default:
		break;
	case 'd':	// directory
	case '-':	// regular file
		entry_specialinfo_str[0] = '\0';
		break;
	case 'c':	// character device
	case 'b':	// block device
		is_special = 1;
		sprintf(entry_specialinfo_str, "(%u,%u)", archive_entry_rdevmajor(entry), archive_entry_rdevminor(entry));
		break;
	case 's':	// socket
		is_special = 1;
		break;
	case 'p':	// fifo
		is_special = 1;
		break;
	case 'h':	// hardlink
		is_special = 1;
		sprintf(entry_specialinfo_str, "%s", archive_entry_hardlink(entry));
		break;
	case 'l':	// symlink
		is_special = 1;
		sprintf(entry_specialinfo_str, "%s", archive_entry_symlink(entry));
		break;
	}

	sprintf(entry_attr_string, "%s|%s|UID:%lu|GID:%lu|MODE:%lu|%s", entry_modestr, p_entry_basename, entry_uid, entry_gid, entry_mode, entry_specialinfo_str);
#if	0
	printf("ATTR: '%s'\n", entry_attr_string);
	printf("FNAME: '%s'\n", entry_fname);
	printf("MODE : '%s'\n", entry_modestr);
	printf("\tFULL: '%s'\n", entry_fullname);
	printf("\t DIR: '%s'\n", parent_dirname);
	printf("\tBASE: '%s'\n", p_entry_basename);

	exit(0);
#endif
	if (parent_dirname[0]) {
		if (_tarfile_extract_create_dir(parent_dirname) != 0) {
			fprintf(stderr, "Error creating parent dir: '%s'!\n", parent_dirname);
			goto exit_err;
		}
		if (_tarfile_add_entry_attributes(parent_dirname, entry_attr_string) != 0) {
			fprintf(stderr, "Error storing entry attributes for file '%s'\n", entry_fname);
			goto exit_err;
		}
	}
	if (show_extract_names) {
		sprintf(plog_global, "Extracting %s\n", entry_fullname); log_it(plog_global);
	}
	if (is_dir) {
		// if this entry is a directory, go ahead and create that directory
		// (with an empty attributes file)
		if (_tarfile_extract_create_dir(entry_fullname) != 0) {
			fprintf(stderr, "Error creating parent dir: '%s'!\n", parent_dirname);
			goto exit_err;
		}
		if (_tarfile_create_entry_attributes(entry_fullname) != 0) {
			fprintf(stderr, "Error storing entry attributes for file '%s'\n", entry_fname);
			goto exit_err;
		}
	} else if (!is_special) {
		// it's a normal file that we can extract from the tar
		const void *block_buf;
		size_t block_size;
		long long	offset;	// int64_t

		if (!(fh_out = fopen(entry_fullname, "wb"))) {
			fprintf(stderr, "Error creating tar extract file '%s'!\n", entry_fullname);
			goto exit_err;
		}
		while(1) {
			ret = archive_read_data_block(th->a, &block_buf, &block_size, &offset);
			if (ret == ARCHIVE_EOF) {
				break;
			}
			if (ret != ARCHIVE_OK) {
				fprintf(stderr, "Error reading from tar file!\n");
				goto exit_err;
			}
			if (fwrite(block_buf, 1, block_size, fh_out) != block_size) {
				fprintf(stderr, "Error writing tar extract file '%s'!\n", entry_fullname);
				goto exit_err;
			}
		}
		fclose(fh_out);
	} else {
		// special file: do I want to create a dummy file with info inside it
		// (like "symlink: '%s')??
#if	0
		if (!(fh_out = fopen(entry_fullname, "wb"))) {
			fprintf(stderr, "Error creating tar extract 'special' file '%s'!\n", entry_fullname);
			goto exit_err;
		}
		fprintf(fh_out, "SPECIAL: '%s/%s'\n", entry_modestr, entry_specialinfo_str);
		fclose(fh_out);
#endif
	}


	goto exit_ok;

exit_err:
	if (fh_out) fclose(fh_out);
	if (entry_fullname[0]) unlink(entry_fullname);
	ret = 1;
	goto exit_common;

exit_ok:
	ret = 0;
	// fallthru
exit_common:
	return ret;
}


int
tarfile_extract_all(const char *fname_tar, const char *dirname_out, int show_extract_names)
{
	tar_handle	th = NULL;
	int	ret;
	struct archive_entry *entry = NULL;
	const char	*p_entry_fname = NULL;

	if (!(th = tarfile_open(fname_tar))) {
		fprintf(stderr, "Error opening tar file '%s'!\n", fname_tar);
		goto exit_err;
	}

	//(void)mkdir(dirname_out, 0777);
	mkdir(dirname_out);
	while(1) {
		ret = archive_read_next_header(th->a, &entry);
		if (ret == ARCHIVE_EOF) {
			break;
		}
		if (ret != ARCHIVE_OK) {
			fprintf(stderr, "Error reading next tar file header!\n");
			goto exit_err;
		}
		p_entry_fname = archive_entry_pathname(entry);
		if (_tarfile_extract_current_entry(th, dirname_out, entry, show_extract_names) != 0) {
			fprintf(stderr, "Error extracting file '%s' from tar!\n", p_entry_fname);
			goto exit_err;
		}
	}
	tarfile_close(th);
	goto exit_ok;

exit_err:
	if (th) tarfile_close(th);
	ret = 1;
	goto exit_common;

exit_ok:
	ret = 0;
	// fallthru
exit_common:
	return ret;
}
