/**************************************************************************
    Lightspark, a free flash player implementation

    Copyright (C) 2009,2010  Alessandro Pignotti (a.pignotti@sssup.it)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#ifndef _ATOMIC_H
#define _ATOMIC_H

#if defined(__MACOSX__)
#	include <libkern/OSAtomic.h>
#elif defined(SN_TARGET_PS3)
#	include <cell/atomic.h>
#elif defined(_XBOX_VER)
#	include <Xtl.h>
#	define _InterlockedExchangeAdd InterlockedExchangeAdd
#elif defined(_WIN32)
#	include <intrin.h>
#else
#	
#endif

namespace lightspark
{

//Note that this function does not return the pre-add value because not
//all platforms support this behavior.
inline void ls_fetch_and_add(volatile int32_t& ref, int32_t val)
{
#if defined(__GNUC__)
#	if defined(SN_TARGET_PS3)
	cellAtomicAdd32(&ref, val);
#	elif defined(__MACOSX__)
	OSAtomicAdd32(val, (int32_t*)&ref);
#	else
	__sync_fetch_and_add(&ref, val);
#	endif
#elif defined(_MSC_VER)
	_InterlockedExchangeAdd((volatile long*)&ref, val);
#else
	//Compile error should prevent us from ever getting here.
#endif
}

};

#endif