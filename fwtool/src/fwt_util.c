//
// Sony NEX camera firmware toolbox
//
// written (reverse-engineered) by Paul Bartholomew, released under the GPL
// (originally based on "pr.exe" from nex-hack.info, with much more since then)
//
// Copyright (C) 2012-2013, nex-hack project
//
// This file "fwt_util.c" is part of fwtool (http://www.nex-hack.info)
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

// fwt_util contains global logfile, slash conversion, console output

#include "config.h"
#include "fwt_names.h"
#include "fwt_util.h"

// model table (0xffffffff = model_type unknown to date)
MODEL_TYPE	model_type[NUM_MODEL_TYPE] = {
	{0x00000000, "Model Type unknown"},
	{0xffffffff, "DSLR-A560"},
	{0xffffffff, "DSLR-A580"},
	{0x10300202, "SLT-A33"},
	{0xffffffff, "SLT-A35"},
	{0x21030008, "SLT-A37"},
	{0x10300203, "SLT-A55"},
	{0x21030006, "SLT-A57"},
	{0xffffffff, "SLT-A58"},
	{0x12030012, "SLT-A65"},
	{0x12030011, "SLT-A77"},
	{0x22030010, "SLT-A99"},
	{0x10300206, "NEX-3"},
	{0x11300223, "NEX-C3"},
	{0x21030010, "NEX-F3"},
	{0xffffffff, "NEX-3N"},
	{0x10300205, "NEX-5"},
	{0x12030023, "NEX-5N"},
	{0x22030014, "NEX-5R"},
	{0xffffffff, "NEX-5T"},
	{0x22030018, "NEX-6"},
	{0x12030024, "NEX-7"},
	{0x10300211, "NEX-VG10"},
	{0xffffffff, "NEX-VG20"},
	{0xffffffff, "NEX-VG30"},
	{0xffffffff, "NEX-VG900"},
	{0x22010015, "NEX-EA50"},
	{0x11010009, "NEX-FS100"},
	{0x21010061, "NEX-FS700"},
	{0xffffffff, "SEL-16F28"},
	{0xffffffff, "SEL-20F28"},
	{0x11A08015, "SEL-24F18Z"},
	{0x11A08014, "SEL-30M35"},
	{0x11A08016, "SEL-50F18"},
	{0xffffffff, "SEL-P1650"},
	{0x10A08011, "SEL-1855"},
	{0x10A08013, "SEL-18200"},
	{0xffffffff, "SEL-18200LE"},
	{0xffffffff, "SEL-P18200"},
	{0x10A08013, "SEL-55210"},
	{0x10A00001, "LA-EA1"},
	{0xffffffff, "LA-EA2"},
	{0xffffffff, "LA-EA3"},
	{0xffffffff, "DSC-RX100"},
	{0xffffffff, "DSC-RX100M2"},
	{0xffffffff, "DSC-RX1"},
	{0xffffffff, "DSC-RX1R"}
};
// model table end

const char fwt_copyright[] ="fwtool " VERSION " Copyright 2012-2013 http://www.nex-hack.info";


void
do_version(const char *tool_name)
{
	printf("%s " VERSION "\n", tool_name);
	printf("info at http://www.nex-hack.info\n\n" \
			"BE WARNED! This is unfinished and/or incomplete and/or untested code!\n" \
			"USE AT YOUR OWN RISK!\n\n");
}


void
do_usage(const char *tool_name)
{
	printf("usage : %s [-<mode>] [-<options>] file or directory\n\n" \
			"  mode :\n" \
			"    -x  eXtract firmware (default mode)\n" \
			"    -c  Create firmware (only rebuild FDAT, reencrypt&rebuild fwdata to date)\n" \
			"    -m  Modify firmware (not implemented yet)\n" \
			"    -h  show extended Help\n" \
			"    -t  show Todo list\n" \
			"  options :\n" \
			"    -d  Destination directory (eXtract only, optional)\n" \
			"    -i  set mInor version number in create or modify mode (default increment)\n" \
			"    -j  set maJor version number in create or modify mode (default leave)\n" \
			"    -l  Level to extract to or create from (default 127)\n" \
			"    -v  be Verbose (not implemented yet)\n", tool_name);
}


void
do_help(const char *tool_name)
{
	printf("help (extended) :" \
			"usage : %s [-<mode>] [-<options>] file or directory\n\n" \
			"  mode :\n" \
			//23456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789
			"    -x  eXtract firmware (default mode, may be omitted):\n" \
			"        Argument: downloaded Windows Firmware Update *.exe file in current\n" \
			"        path, extension '.exe' may be omitted.\n" \
			"        Extraction to directory contained in zip inside *.exe to current path\n" \
			"        or destination directory if -d is given.\n" \
			"        Extract complete content, used as input to modify and create mode.\n" \

			"    -c  Create firmware (only rebuild FDAT, reencrypt&rebuild fwdata to date):\n" \
			"        Argument: <UpdaterDirectory> created with eXtract mode, this may contain\n" \
			"        a relative or full path.\n" \
			"        <UpdaterDirectory>/nexhack/level3/*.mod.* must exist, this files will be\n" \
			"        repacked, reencrypted and repacked to a valid FirmwareUpdate*.dat,\n" \
			"        further versions of fwtool will repack lower levels not implemented yet.\n" \
			"        Run <UpdaterDirectory>/FirmwareUpdater.exe to update cam with modified\n" \
			"        firmware <UpdaterDirectory>/Resource/FirmwareData_NexHack.dat.\n" \
			"        Original firmware is saved to\n" \
 			"        <UpdaterDirectory>/Resource/FirmwareData_Original.dat.save.\n" \

			"    -m  Modify firmware (not implemented yet)\n" \
			"    -h  show extended Help (this list)\n" \
			"    -t  show Todo list\n" \

			"  options :\n" \
			"    -d  Destination directory (eXtract only, optional):\n" \
			"        Extracting complete firmware updater directory not to current path but\n" \
			"        to this directory.\n" \
			"        In create mode a destination makes no sense, so this option is ignored.\n" \
			"    -i  set mInor version number in create or modify mode (default increment):\n" \
			"        Minor version number may be in the range from 0 to %d.\n" \
			"    -j  set maJor version number in create or modify mode (default leave):\n" \
			"        Major version number may be in the range from 1 to %d.\n" \
			"    -l  Level to extract to or create from (default 127):\n" \
			"        Used for modify mode to do only needed unpacking, not implemented yet.\n" \
			"    -v  be Verbose (not implemented yet)\n" \
			"  fwtool appends extended log output to fwtool_log.txt in current path.\n", tool_name, FWT_MAXMAJORVER, FWT_MAXMINORVER);
}


void
do_todolist(const char *tool_name)
{
	printf("TODO list %s "VERSION" (in that order)\n\n" \
			"  v07: implement repacking of tar, lzpt\n" \
			"  v07: implement level logic for ease of repack (create mode)\n" \
			"  v07: fully integrate fwtoolGUI into main code\n" \
			"  v07: cleanup variables, variable names and code ;)\n" \
			"  v08: implement unpacking of cramfs, cramfs-lz77, axfs, vfat, ext2\n" \
			"  v08: implement repacking of vfat, ext2, cramfs(lz77), axfs\n" \
			"  v08: implement modify config&code package format\n" \
			"  v09: implement on the fly in memory modify for end users\n" \
			"  v09: implement modify of level6 (bin fw's for sub cpu's)\n" \
			"  v09: clear many minor TODO, UGLY; do heavy testing\n" \
			"  v1.0: document what this tools do ...\n\n", tool_name);
}


char *
convert_path_slashes(char *buf)
{
	char	*p;
	if (buf) {
		while((p = strrchr(buf, '\\'))) {
			*p = '/';
		}
	}
	return buf;
}


void
log_it(char *pinfo)
{
    FILE *plog;

    if((plog = fopen(LOGFILE,"a")))
    {
        fputs(pinfo,plog);
        fflush(plog);
        fclose(plog);
    }
}


void
name_model(const unsigned int fih_model_type, MODEL_TYPE *buf)
{
	int i=0;

	memcpy((void *)buf, (void *)&model_type[0], sizeof(model_type[0]));
	buf->modt_fih_model = fih_model_type;

	for(i=0;i<NUM_MODEL_TYPE;i++) {
		if(fih_model_type == model_type[i].modt_fih_model) {
			memcpy((void *)buf, (void *)&model_type[i], sizeof(model_type[i]));
		}
	}
}
