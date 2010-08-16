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

#ifndef _LIGHTSPARK_C_INTERFACE
#define _LIGHTSPARK_C_INTERFACE

#include "compat.h"

struct lightspark_system_state
{
	int log_level;
	char* filename;
	char* url;
	char* paramsFileName;
	int useInterpreter;
	int useJit;
	void* abcVm;
	void* context_istream;
	void* context;
	void* zlibfilter;
};

typedef void (*lightspark_api_func)(lightspark_system_state*);

#ifdef __cplusplus
extern "C" {
#endif

void lightspark_system_state_defaults(lightspark_system_state* lightspark_state) DLL_PUBLIC;
void init_lightspark(lightspark_system_state* lightspark_state) DLL_PUBLIC;
void run_lightspark(lightspark_system_state* lightspark_state) DLL_PUBLIC;
void destroy_lightspark(lightspark_system_state* lightspark_state) DLL_PUBLIC;

#ifdef __cplusplus
}
#endif

#endif