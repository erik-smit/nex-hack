//
// Sony NEX camera firmware toolbox
//
// written (reverse-engineered) by Paul Bartholomew, released under the GPL
// (originally based on "pr.exe" from nex-hack.info, with much more since then)
//
// Copyright (C) 2012-2013, nex-hack project
//
// This file "endian.c" is part of fwtool (http://www.nex-hack.info)
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
#include "endian.h"

// read a big-endian 16-bit value from buffer
u16
readBE16(u8 *ptr)
{
	return (((u16)(ptr[0]) << 8) | ptr[1]);
}

// read a big-endian 32-bit value from buffer
u32
readBE32(u8 *ptr)
{
	return (((u32)readBE16(&ptr[0]) << 16) | readBE16(&ptr[2]));
}

// read a little-endian 16-bit value from buffer
u16
readLE16(u8 *ptr)
{
	return (((u16)(ptr[1]) << 8) | ptr[0]);
}

// read a little-endian 32-bit value from buffer
u32
readLE32(u8 *ptr)
{
	return (((u32)readLE16(&ptr[2]) << 16) | readLE16(&ptr[0]));
}

void
BE16toHost(u8 *ptr)
{
	u16	val;

	val = readBE16(ptr);
	*(u16 *)ptr = val;
}

void
BE32toHost(u8 *ptr)
{
	u32	val;

	val = readBE32(ptr);
	*(u32 *)ptr = val;
}

void
LE16toHost(u8 *ptr)
{
	u16	val;

	val = readLE16(ptr);
	*(u16 *)ptr = val;
}

void
LE32toHost(u8 *ptr)
{
	u32	val;

	val = readLE32(ptr);
	*(u32 *)ptr = val;
}

/*


void
HosttoBE16 (u16 val, u8 *ptr)
{
	ptr[1] = (val & 0x00ff);
	ptr[0] = (val & 0xff00) >> 8;
}

void
HosttoBE32 (u32 val, u8 *ptr)
{
	ptr[3] = (val & 0xff);
	ptr[2] = (val & 0xff00) >> 8;
	ptr[1] = (val & 0xff0000) >> 16;
	ptr[0] = (val & 0xff000000) >> 24;
}

void
HosttoLE16(u16 val, u8 *ptr)
{
	ptr[0] = (val & 0x00ff);
	ptr[1] = (val & 0xff00) >> 8;   
}

void
HosttoLE32(u32 val, u8 *ptr)
{
	ptr[0] = (u8)((val & 0x000000ffL));
	ptr[1] = (u8)((val & 0x0000ff00L) >> 8);
	ptr[2] = (u8)((val & 0x00ff0000L) >> 16);
	ptr[3] = (u8)((val & 0xff000000L) >> 24);
}
*/
