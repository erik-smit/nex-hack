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

#ifndef __LZ77_H__
#define __LZ77_H__

#define lz77_warning printf

#if	!defined(__POCC__)
	#ifdef DEBUG
		#define lz77_printf printf
	#else
		#define lz77_printf(fmt,arg...)
	#endif
#endif

#define LZ77_COMPRESSED (0xF0)
#define LZ77_RAW (0x0F)

#define SIZE_BITS (4)

#define WIN_BITS (12)
#define WIN_LEN (1<<WIN_BITS)

#define MIN_LEN (3)
#define MAX_LEN (64)
#define OPT_LEN (8)

#define size_to_index(s) \
({ \
	int j; \
	if((s)<=16) j=(s)-3; \
	else if((s)>63) j=15; \
	else if((s)>31) j=14; \
	else j=13; \
	j; \
})

#if	defined(__POCC__) && defined(LZ77_INTERNAL)
	static int	_index_to_size(int i) {
		static int len_table[1<<(SIZE_BITS)] = {
			3, 4, 5, 6, 7, 8, 9, 10,
			11, 12, 13, 14, 15, 16,
			32, 64
		};
	return len_table[i];
	}
	#define index_to_size(i) _index_to_size(i)
#else
	#define index_to_size(i) \
	({ \
	static int len_table[1<<(SIZE_BITS)] = { \
		3, 4, 5, 6, 7, 8, 9, 10, \
		11, 12, 13, 14, 15, 16, \
		32, 64 \
		}; \
	len_table[i]; \
	})
#endif

struct lit_codeword {
	int type;	//0=literal,1=code word
	unsigned char lit[2];
};

struct lz77_union {
	unsigned char type;	//for per bit:0=literal 1=codeword
	unsigned char index;	//index to l_cw
	struct lit_codeword l_cw[8];
};

#define HASH_BITS (11)
#define HASH_SIZE ((1<<(HASH_BITS+1)))
#define HASH_VALUE(d1,d2,d3) (((d1)<<(HASH_BITS-8))^((d2)<<(HASH_BITS-10))^(d3))

#define WORKSPACE_HASH(buf) (buf)
#define WORKSPACE_INDEX(buf) ((void *)((u8 *)(buf)+HASH_SIZE))

#define LZ77_WORKSPACE_SIZE(winlen) (HASH_SIZE+sizeof(short)*(winlen))

int lz77_inflate(unsigned char *, int, unsigned char *, int, unsigned char **);
int lz77_deflate(unsigned char *, int, unsigned char *, int, void *, int);

#endif
