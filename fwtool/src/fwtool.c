//
// Sony NEX camera firmware toolbox
//
// written (reverse-engineered) by Paul Bartholomew, released under the GPL
// (originally based on "pr.exe" from nex-hack.info, with much more since then)
//
// This utility extracts nex/slt firmware from update.exe, decrypts and unpacks
// it, and may repack end reencrypt it.
//
// History
// v0.5      (2012-11-19 oz_paulb):	initial
// v0.6alpha (2013-05-09 kenan):	added nflasha9,nflashb1 lzpt decompression
//									added options code for future expansion
// v0.6beta  (2013-05-16 kenan):	added re encryption of FDAT.dec
//									added repack of fwdata.dat
//									dropped single fwdata unpack
//									added help, usage and todo
//									rearrange code, some cleanup
//									added LZTP 0x11 unpacking
//
// Copyright (C) 2012-2013, nex-hack project
//
// This file "fwtool.c" is part of fwtool (http://www.nex-hack.info)
//
// All rights reserved.
//
// This tool includes code written and copyrighted by third parties.
// Look at individual files for details !
// It is statically linked with zlib and libarchive and uses code from
// the minizip project, PolarSSL project and Sony Corporation.
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

// fwtool contains only main for console ui and init for global logfile

#include "config.h"		// compiler and os dependant config
#include "fwt_names.h"	// version string and hardcoded names are set there
#include "fwt_util.h"
#include "fwd_pack.h"


// init globals for logfile
int	fwt_verbose_global = 0;
char	plog_global[1024] = "";
// globals for logfile


int
main(int argc, char *argv[])
{
	char	*tool_name=argv[0];
	int	i, fwt_mode=0, fwt_level=127, fwt_minorver= -1, fwt_majorver= -1;

	char	*src_name = NULL, *dest_name = NULL;

	do_version(tool_name);

	if (argc < 2) {
		do_usage(tool_name);
		exit(EXIT_FAILURE);
	}
	else {
		for (i=1;i<argc;i++) {
			if ((*argv[i]) == '-') {
				const char *p=argv[i]+1;

				while ((*p)!='\0') {
					char c=*(p++);;
					// mode
					if (c == 'x') {
						fwt_mode = 0;
					}
					if (c == 'c') {
						fwt_mode = 1;
					}
					if (c == 'm') {
						fwt_mode = 2;
					}
					if (c == 'h') {
						do_help(tool_name);
						exit(EXIT_SUCCESS);
					}
					if (c == 't') {
						do_todolist(tool_name);
						exit(EXIT_SUCCESS);
					}
					// options
					if (c == 'v') {
						fwt_verbose_global = 1;
					}
					if (c == 'l') {
						fwt_level = atoi(argv[i+1]);
						if(fwt_level > 127) fwt_level = 127;
						if(fwt_level < 1) fwt_level = 1;
					}
					if (c == 'j') {
						fwt_majorver = atoi(argv[i+1]);
						// major may be signed, if not, allow for 1..255
						if(fwt_majorver > FWT_MAXMAJORVER) fwt_majorver = FWT_MAXMAJORVER;
						// no beta fw 0.x ever released as update file ...
						if(fwt_majorver < 1) fwt_majorver = 1;
					}
					if (c == 'i') {
						fwt_minorver = atoi(argv[i+1]);
						// minor may be signed, if not, allow for 0..255
						if(fwt_minorver > FWT_MAXMINORVER) fwt_minorver = FWT_MAXMINORVER;
						if(fwt_minorver < 0) fwt_minorver = 0;
					}
					if (c == 'd') {
						//TODO: check format
						dest_name = convert_path_slashes(argv[i+1]);
					}
                }
            }
			else {
				//TODO: check format
				src_name = convert_path_slashes(argv[i]);
			}
		}
	}

	switch (fwt_mode) {
		case 0:
				// add .exe if not given
				if(!strstr(src_name, EXE_SUFFIX)) {
					char	l_src_name[MAXPATH];
					sprintf(l_src_name,"%s%s", src_name, EXE_SUFFIX);
					src_name = l_src_name;
				}
				if(fwt_minorver != -1) fprintf(stderr, "minor version number makes no sense in extract mode, ignored!\n");
				if(fwt_majorver != -1) fprintf(stderr, "major version number makes no sense in extract mode, ignored!\n");
				sprintf(plog_global,"_____\nfwtool %s extracting '%s' to '%s'\n\n", VERSION, src_name, dest_name); log_it(plog_global);
				exit(do_unpack(src_name, dest_name, fwt_level));
				break;
		case 1:
				if(dest_name) fprintf(stderr, "destination directory makes no sense in create mode, ignored!\n");
				sprintf(plog_global,"_____\nfwtool %s repacking '%s'\n\n", VERSION, src_name); log_it(plog_global);
				exit(do_repack(src_name, fwt_level, fwt_majorver, fwt_minorver));
				break;
		case 2:
				//TODO delete when implemented
				fprintf(stderr, "Modify not yet implemented!\n");
				exit(98);
				//
				// sprintf(plog_global,"_____\nfwtool modifying '%s'\n\n",src_name); log_it(plog_global);
				//TODO check modify config
				//TODO modify call:
				// retval = do_unpack(src_name, dest_name, fwt_level);
				//TODO apply modify
				//TODO modify call:
				// retval = do_repack(src_name, fwt_level);
				// return(retval);
				break;
		default:
				fprintf(stderr, "No mode selected, should never happen\n");
				exit(EXIT_FAILURE);
	}

}
