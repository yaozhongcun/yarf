/* Copyright (c) 2017, Johnyao 
 * All rights reserved.
 * 
 * misc.cc
 *
 *  Created on: 2017Äê11ÔÂ22ÈÕ
 *      Author: YAOZHONGCUN
 */

#include "yarf/err.h"
#include "yarf/memory/misc.h"

using namespace yarf::memory;

uint32_t GetAlignmentSize(uint32_t size, uint32_t alignment_level) {
  
	if ( 8 == alignment_level )
	{
		return ((size + 7) & ~7);
	}
	else if ( 4 == alignment_level )
	{
		return ((size + 3) & ~3);
	}
	else if ( 2 == alignment_level )
	{
		return  ((size + 1) & ~1);
	}

	return size;
}

