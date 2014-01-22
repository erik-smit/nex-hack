//
// Sony NEX camera firmware toolbox
//
// written (reverse-engineered) by Paul Bartholomew, released under the GPL
// (originally based on "pr.exe" from nex-hack.info, with much more since then)
//
// Copyright (C) 2012-2013, nex-hack project
//
// This file "zipfile.c" is part of fwtool (http://www.nex-hack.info)
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

#define	ZIPFILE_C

#include "config.h"
#include "fwt_util.h"
#include "contrib/minizip/unzip.h"
#include "zipfile.h"

/* // for date&time
#undef	_WIN32
#ifndef _WIN32
        #ifndef __USE_FILE_OFFSET64
                #define __USE_FILE_OFFSET64
        #endif
        #ifndef __USE_LARGEFILE64
                #define __USE_LARGEFILE64
        #endif
        #ifndef _LARGEFILE64_SOURCE
                #define _LARGEFILE64_SOURCE
        #endif
        #ifndef _FILE_OFFSET_BIT
                #define _FILE_OFFSET_BIT 64
        #endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#ifdef unix
# include <unistd.h>
# include <utime.h>
#else
//# include <direct.h>
# include <io.h>
#endif
// */

static zip_handle
_create_zh(unzFile uf)
{
	zip_handle	retval = NULL;

	if ((retval = malloc(sizeof(zip_handle_struct)))) {
		memset((void *)retval, 0, sizeof(zip_handle_struct));
		retval->uf = uf;
	}

	return retval;
}


static void
_free_zh(zip_handle zh)
{
	if (zh) {
		free((void *)zh);
	}
}


/* // from miniunzip.c
// change_file_date : change the date/time of a file
// filename : the filename of the file where date/time must be modified
// dosdate : the new date at the MSDos format (4 bytes)
// tmu_date : the SAME new date at the tm_unz format
void change_file_date(filename,dosdate,tmu_date)
    const char *filename;
    uLong dosdate;
    tm_unz tmu_date;
{
#ifdef _WIN32
  HANDLE hFile;
  FILETIME ftm,ftLocal,ftCreate,ftLastAcc,ftLastWrite;

  hFile = CreateFileA(filename,GENERIC_READ | GENERIC_WRITE,
                      0,NULL,OPEN_EXISTING,0,NULL);
  GetFileTime(hFile,&ftCreate,&ftLastAcc,&ftLastWrite);
  DosDateTimeToFileTime((WORD)(dosdate>>16),(WORD)dosdate,&ftLocal);
  LocalFileTimeToFileTime(&ftLocal,&ftm);
  SetFileTime(hFile,&ftm,&ftLastAcc,&ftm);
  CloseHandle(hFile);
#else
#ifdef unix
  struct utimbuf ut;
  struct tm newdate;
  newdate.tm_sec = tmu_date.tm_sec;
  newdate.tm_min=tmu_date.tm_min;
  newdate.tm_hour=tmu_date.tm_hour;
  newdate.tm_mday=tmu_date.tm_mday;
  newdate.tm_mon=tmu_date.tm_mon;
  if (tmu_date.tm_year > 1900)
      newdate.tm_year=tmu_date.tm_year - 1900;
  else
      newdate.tm_year=tmu_date.tm_year ;
  newdate.tm_isdst=-1;

  ut.actime=ut.modtime=mktime(&newdate);
  utime(filename,&ut);
#endif
#endif
}
//end date etc not working */

zip_handle
zipfile_open(const char *fname)
{
	unzFile uf=NULL;

	uf = unzOpen64(fname);

	if (uf) {
		return _create_zh(uf);
	} else {
		return NULL;
	}
}


void
zipfile_close(zip_handle zh)
{
	if (zh) {
		if (zh->uf) {
			unzClose(zh->uf);
		}
		_free_zh(zh);
	}

}


int
is_zipfile(const char *fname)
{
	zip_handle	zh;

	if ((zh = zipfile_open(fname))) {
		zipfile_close(zh);
		return 1;
	} else {
		return 0;
	}
}


int
zipfile_find_first(zip_handle zh, char *p_fname_buf, int sz_fname_buf, unsigned long *p_uncomp_size, unsigned int *p_crc32)
{

	if (unzGetGlobalInfo64(zh->uf,&zh->gi) != UNZ_OK) {
		return -11;
	}

	zh->next_idx = 0;
	return zipfile_find_next(zh, p_fname_buf, sz_fname_buf, p_uncomp_size, p_crc32);
}


int
zipfile_find_next(zip_handle zh, char *p_fname_buf, int sz_fname_buf, unsigned long *p_uncomp_size, unsigned int *p_crc32)
{
	char	filename_buf[512];
	int	len;

	if (zh->next_idx >= zh->gi.number_entry) {
		return 1;
	}
	filename_buf[0] = '\0';
    if (unzGetCurrentFileInfo64(zh->uf,&zh->file_info,filename_buf,sizeof(filename_buf)-1,NULL,0,NULL,0) != UNZ_OK) {
		return -1;
	}
	len = strlen(filename_buf);
	if (len > (sz_fname_buf-1)) {
		len = sz_fname_buf-1;
	}
	if (p_fname_buf) {
		strncpy(p_fname_buf, filename_buf, len);
		p_fname_buf[len] = '\0';
	}
	if (p_uncomp_size) {
		*p_uncomp_size = zh->file_info.uncompressed_size;
	}
	if (p_crc32) {
		*p_crc32 = zh->file_info.crc;
	}

	zh->next_idx++;
    unzGoToNextFile(zh->uf);
	return 0;
}


int
zipfile_list(zip_handle zh)
{
	char	fname[MAXPATH];
	int	ret = 0;

	if (!zh) {
		return 1;
	}
	ret = zipfile_find_first(zh, fname, sizeof(fname)-1, NULL, NULL);
	while(ret == 0) {
		printf("FNAME: '%s'\n", fname);
		ret = zipfile_find_next(zh, fname, sizeof(fname)-1, NULL, NULL);
	}
	return ret;
}


int
zipfile_extract_file(char *fname_zip, char *fname_inzip, char *dirname_out, char *p_extracted_namebuf)
{
	int	ret = 0;
	zip_handle	zh = NULL;
	char	fullname_out[MAXPATH] = {0};
	char	*p_basename_inzip;
	char	*p_last_slash, *p_last_backslash;
	unsigned char	*p_iobuf = NULL;
	FILE	*fh_out = NULL;
	int	nbytes;

	if (p_extracted_namebuf) *p_extracted_namebuf = '\0';

	if (!(zh = zipfile_open(fname_zip))) {
		fprintf(stderr, "Error opening zip file '%s'!\n", fname_zip);
		goto exit_err;
	}

	if (unzLocateFile(zh->uf,fname_inzip, 1) != UNZ_OK) {
		fprintf(stderr, "Couldn't find '%s' in zip file!\n", fname_inzip);
		goto exit_err;
	}

	if (!(p_iobuf = malloc(ZIP_IOBUF_SIZE))) {
		fprintf(stderr, "Couldn't allocate zip io buffer!\n");
		goto exit_err;
	}

	// find basename of file inside zip, allowing for either
	// "/" or "\" path delimeters
	p_last_slash = strrchr(fname_inzip, '/');
	p_last_backslash = strrchr(fname_inzip, '\\');
	if (!p_last_slash || (p_last_backslash && (p_last_backslash > p_last_slash))) {
		p_basename_inzip = p_last_backslash;
	} else {
		p_basename_inzip = p_last_slash;
	}
	if (!p_basename_inzip) {
		p_basename_inzip = fname_inzip;
	} else {
		// move past path sep
		++p_basename_inzip;
	}

	if (dirname_out) {
		sprintf(fullname_out, "%s/%s", dirname_out, p_basename_inzip);
		//(void)mkdir(dirname_out, 0777);
		mkdir(dirname_out);
	} else {
		strcpy(fullname_out, p_basename_inzip);
	}

	if (unzOpenCurrentFilePassword(zh->uf, NULL) != UNZ_OK) {
		fprintf(stderr, "Couldn't open '%s' in zip file!\n", fname_inzip);
		goto exit_err;
	}

	unlink(fullname_out);
	if (!(fh_out = fopen(fullname_out, "wb"))) {
		fprintf(stderr, "Error creating '%s'!\n", fullname_out);
		goto exit_err;
	}
	while(1) {
		if (!(nbytes = unzReadCurrentFile(zh->uf, p_iobuf, ZIP_IOBUF_SIZE))) {
			break;
		}
		if (nbytes < 0) {
			fprintf(stderr, "Error reading '%s' in zip file!\n", fname_inzip);
			goto exit_err;
		}
		if (fwrite(p_iobuf, 1, nbytes, fh_out) != nbytes) {
			fprintf(stderr, "Error writing to '%s'!\n", fullname_out);
			goto exit_err;
		}
	}
	fclose(fh_out); fh_out = NULL;
	zipfile_close(zh); zh = NULL;

	goto exit_ok;

exit_err:
	if (zh) zipfile_close(zh);
	if (fh_out) fclose(fh_out);
	if (fullname_out[0]) unlink(fullname_out);
	ret = 1;
	goto exit_common;

exit_ok:
	strcpy(p_extracted_namebuf, fullname_out);
	ret = 0;
	// fallthru
exit_common:
	if (p_iobuf) free((void *)p_iobuf);
	return ret;
}


// from miniunzip.c modified, needs TODO cleanup
int
_zipfile_extract_current_entry(zip_handle zh, char *dirname_out, char *p_fname_buf, int show_extract_names)
{
	//TODO implement dirname_out (extract complete zip not to current dir but to dirname_out)
	char filename_inzip[MAXPATH];
	char* filename_withoutpath;
	char* p;
	int err=UNZ_OK;
	FILE *fout=NULL;
	void* buf;
	uInt size_buf;

	if (p_fname_buf) *p_fname_buf = '\0';

	unz_file_info64 file_info;
	err = unzGetCurrentFileInfo64(zh->uf, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);

	if (err!=UNZ_OK) {
		fprintf(stderr, "error %d with zipfile in unzGetCurrentFileInfo\n",err);
		return err;
	}

	size_buf = ZIP_IOBUF_SIZE;
	buf = (void*)malloc(size_buf);
	if (buf==NULL) {
		fprintf(stderr, "Error allocating memory\n");
		return UNZ_INTERNALERROR;
	}

	p = filename_withoutpath = filename_inzip;
	while ((*p) != '\0') {
		if (((*p)=='/') || ((*p)=='\\')) filename_withoutpath = p+1;
		p++;
	}

	if ((*filename_withoutpath)=='\0') {
		if(dirname_out) {
			char	l_filename_inzip[MAXPATH];
			sprintf(l_filename_inzip,"%s/%s", dirname_out, filename_inzip);
			sprintf(plog_global, "Creating directory: %s\n",l_filename_inzip); log_it(plog_global);
			//(void)mkdir(l_filename_inzip, 0777);
			mkdir(l_filename_inzip);
		}

		else {
			sprintf(plog_global, "Creating directory: %s\n",filename_inzip); log_it(plog_global);
			//(void)mkdir(filename_inzip, 0777);
			mkdir(filename_inzip);
		}
	}
	else {
		const char* write_filename;
		if(dirname_out) {
			char	l_write_filename[MAXPATH];
			sprintf(l_write_filename,"%s/%s", dirname_out, filename_inzip);
			write_filename = l_write_filename;
		}
		else write_filename = filename_inzip;

		err = unzOpenCurrentFilePassword(zh->uf, NULL);
		if (err!=UNZ_OK) {
			fprintf(stderr, "Error %d with zipfile in unzOpenCurrentFilePassword\n",err);
		}

		if (err==UNZ_OK) {
			fout=fopen64(write_filename,"wb");

			/* some zipfile don't contain directory alone before file */
			if ((fout==NULL) && (filename_withoutpath!=(char*)filename_inzip)) {
				char c=*(filename_withoutpath-1);
				*(filename_withoutpath-1)='\0';
				if(dirname_out) {
					char	l_write_filename[MAXPATH];
					sprintf(l_write_filename,"%s/%s", dirname_out, filename_inzip);
					sprintf(plog_global, "Creating directory (from path): %s\n",l_write_filename); log_it(plog_global);
					//(void)mkdir(l_write_filename, 0777);
					mkdir(l_write_filename);
					*(filename_withoutpath-1)=c;
					sprintf(l_write_filename,"%s/%s", dirname_out, filename_inzip);
					fout=fopen64(l_write_filename,"wb");
				}
				else {
					sprintf(plog_global, "Creating directory (from path): %s\n",write_filename); log_it(plog_global);
					//(void)mkdir(write_filename, 0777);
					mkdir(write_filename);
					*(filename_withoutpath-1)=c;
					fout=fopen64(write_filename,"wb");
				}
			}

			if (fout==NULL) {
				fprintf(stderr, "Error opening %s\n",write_filename);
			}
		}

		if (fout!=NULL) {
			sprintf(plog_global, " Extracting: %s\n",write_filename); log_it(plog_global);
			do {
				err = unzReadCurrentFile(zh->uf,buf,size_buf);
				if (err<0) {
					fprintf(stderr, "Error %d with zipfile in unzReadCurrentFile\n",err);
					break;
				}
				if (err>0)
					if (fwrite(buf,err,1,fout)!=1) {
						fprintf(stderr, "Error in writing extracted file\n");
						err=UNZ_ERRNO;
						break;
					}
			} while (err>0);
			if (fout) fclose(fout);

			if (err==0)
				//TODO change_file_date(write_filename, file_info.dosDate, file_info.tmu_date);
				;
		}

		if (err==UNZ_OK) {
			err = unzCloseCurrentFile(zh->uf);
			if (err!=UNZ_OK) {
				fprintf(stderr, "Error %d with zipfile in unzCloseCurrentFile\n",err);
			}
		}
		else unzCloseCurrentFile(zh->uf); /* don't lose the error */
	}

	free(buf);

	//TODO maybe export pathname, filename
	return err;
}


int
zipfile_extract_all(char *fname_zip, char *dirname_out, char *p_extracted_dir, char *p_extracted_fname, int show_extract_names)
{
	//TODO implement dirname_out (extract complete zip not to current dir but to dirname_out)
	int	ret = 0;
	uLong	i;
	unz_global_info64	gi;
	int	err;

	zip_handle	zh = NULL;
	char	*p_fname_buf = NULL;

	if (p_extracted_dir) *p_extracted_dir = '\0';

	if (!(zh = zipfile_open(fname_zip))) {
		fprintf(stderr, "Error opening zip file '%s'!\n", fname_zip);
		goto exit_err;
	}

    err = unzGetGlobalInfo64(zh->uf,&gi);
    if (err!=UNZ_OK)
        fprintf(stderr, "Error %d with zipfile in unzGetGlobalInfo \n",err);

    for (i=0;i<gi.number_entry;i++)
    {
        if (_zipfile_extract_current_entry(zh,dirname_out,p_fname_buf,show_extract_names) != UNZ_OK)
            break;

        if ((i+1)<gi.number_entry)
        {
            err = unzGoToNextFile(zh->uf);
            if (err!=UNZ_OK)
            {
                fprintf(stderr, "Error %d with zipfile in unzGoToNextFile\n",err);
                break;
            }
        }
    }
	zipfile_close(zh); zh = NULL;

	goto exit_ok;

exit_err:
	if (zh) zipfile_close(zh);
	ret = 1;
	goto exit_common;

exit_ok:
	ret = 0;
	// fallthru
exit_common:
	return ret;
}
