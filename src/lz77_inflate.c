/* 2007-07-26: File added by Sony Corporation */
/* With non GPL files, use following license */
/*
 * Copyright 2006,2007 Sony Corporation.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions, and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* Otherwise with GPL files, use following license */
/*
 *  Copyright 2006,2007 Sony Corporation.
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  version 2 of the  License.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 */

#define LZ77_INTERNAL
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lz77.h"

static inline int lz77_memcpy(unsigned char *src, int src_len, unsigned char *dst, int dst_len)
{
	int ret=src_len>dst_len?dst_len:src_len;

	if(ret){
		int len=ret&(~0x03);
		unsigned char *d = dst;
		unsigned char *s = src;

		if(ret>=10){
			if((((unsigned long)(dst-src))&0x03)==0){
				while((unsigned long)d&0x03) *d++=*s++;

				while(d<(dst+len)) {
					*(unsigned long *)d = *(unsigned long *)s;
					d += 4;
					s += 4;
				}
			}
			else if((((unsigned long)(dst-src))&0x01)==0){
				while((unsigned long)d&0x01) *d++=*s++;

				while(d<(dst+len)) {
					*(unsigned short *)d = *(unsigned short *)s;
					d += 2;
					s += 2;
				}
			}
		}

		/*byte copy*/
		while(d<(dst+ret)) *d++ = *s++;
	}
	return ret;
}

int lz77_inflate(unsigned char *src, int len, unsigned char *dst, int dst_len, unsigned char **sd)
{
	int type=0;
	unsigned char *s=src, *d=dst;
	unsigned char *de = dst+dst_len;
	unsigned char *se = src+len-1;
	int u_i=0;

	if(!src || len<4 || !dst)
		return -1;

	switch(*s++){
	case LZ77_COMPRESSED:
		while(s<se){
			u_i = 0;
			type = *s++;
			while(u_i++<8 && s<se){
				if(type & 0x01){//codeword
					int l = (s[0]&0xF0)>>4;
					int bd = (s[0]&0x0F)<<8;
					bd |= s[1];
					bd &= 0xFFF;
					l = index_to_size(l);
					s += 2;

					if(bd)
						d += lz77_memcpy(d-bd, l, d, (int)(de-d));
					else{
#if	!defined(__POCC__)
						lz77_printf("end of stream!\n");
#endif
						goto inflate_end;
					}
				}
				else{
					*d++ = *s++;
				}

				type >>= 1;
			}
		}

		break;
	case LZ77_RAW:
	{
		int l = s[1] | (s[2]<<8);

		s += 3;
		l = dst_len>(l)?(l):dst_len;
		memcpy(d, s, l);
		d += l;
		s += l;

#if	!defined(__POCC__)
		lz77_printf("raw block length: %d\n", l);
#endif
	}

		break;
	default:
//		lz77_warning("error compress method!\n");
		printf("error compress method: %02x!\n", s[-1]);

		break;
	};

 inflate_end:
#if	!defined(__POCC__)
	lz77_printf("inflate: %d %d %d %d %p %p\n", s-src, d-dst, len, dst_len, s, se);
#endif
	if(sd)
		*sd = s;
	return (d-dst);
}

