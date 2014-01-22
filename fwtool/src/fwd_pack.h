//
// Sony NEX camera firmware toolbox
//
// written (reverse-engineered) by Paul Bartholomew, released under the GPL
// (originally based on "pr.exe" from nex-hack.info, with much more since then)
//
// Copyright (C) 2012-2013, nex-hack project
//
// This file "fwd_pack.h" is part of fwtool (http://www.nex-hack.info)
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

#ifndef FWD_PACK_H
#define FWD_PACK_H

#ifdef FWT_CONSOLE
    // build exe console only
    extern	int	do_unpack(const char *exefile_in, const char *dest_name, const int fwt_level);
    extern	int	do_repack(const char *dirname_in, const int fwt_level, const int fwt_majorver, const int fwt_minorver);
#else
    // build dll for fwtoolGUI
    #include <windows.h>

    // To use these exported functions of dll, include this header in your project.

    #ifdef BUILD_DLL
        #define DLL_EXPORT __declspec(dllexport)
    #else
        #define DLL_EXPORT __declspec(dllimport)
    #endif


    #ifdef __cplusplus
        extern "C"
        {
    #endif

    DLL_EXPORT int do_unpack(const char *exefile_in, const char *dest_name, const int fwt_level);
    DLL_EXPORT int do_repack(const char *dirname_in, const int fwt_level, const int fwt_majorver, const int fwt_minorver);

    #ifdef __cplusplus
        }
    #endif
#endif


int find_firmware_dat_in_zipfile(const char *fname_zip, char *p_fname_outbuf, int sz_fname_outbuf,
		char *p_fbase_outbuf, int sz_fbase_outbuf, char *p_fdir_outbuf, int sz_fdir_outbuf,
		char *p_fbasedir_outbuf, int sz_fbasedir_outbuf);

#endif // FWD_PACK_H
