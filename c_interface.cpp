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

#include "abc.h"

#include <fstream>
#include <sys/resource.h>
#include "compat.h"
#include "streams.h"

using namespace std;
using namespace lightspark;

TLSDATA DLL_PUBLIC SystemState* sys=NULL;
TLSDATA DLL_PUBLIC RenderThread* rt=NULL;
TLSDATA DLL_PUBLIC ParseThread* pt=NULL;

extern int count_reuse;
extern int count_alloc;

extern "C"
{

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

void lightspark_system_state_defaults(lightspark_system_state* lightspark_state)
{
	lightspark_state->log_level = LOG_NOT_IMPLEMENTED;
	lightspark_state->filename = NULL;
	lightspark_state->url = NULL;
	lightspark_state->paramsFileName = NULL;
	lightspark_state->useInterpreter = 1;
	lightspark_state->useJit = 0;

	lightspark_state->context_istream = NULL;
	lightspark_state->zlibfilter = NULL;
}


void init_lightspark(lightspark_system_state* lightspark_state)
{
#ifndef WIN32
	struct rlimit rl;
	getrlimit(RLIMIT_AS,&rl);
	rl.rlim_cur=1500000000;
	rl.rlim_max=rl.rlim_cur;
	//setrlimit(RLIMIT_AS,&rl);
#endif

	Log::initLogging((LOG_LEVEL)lightspark_state->log_level);
	
	lightspark_state->zlibfilter = new zlib_file_filter(lightspark_state->filename);
	lightspark_state->context_istream=new istream((zlib_file_filter*)lightspark_state->zlibfilter);
	
	((istream*)lightspark_state->context_istream)->exceptions ( istream::eofbit | istream::failbit | istream::badbit );
	cout.exceptions( ios::failbit | ios::badbit);
	cerr.exceptions( ios::failbit | ios::badbit);
	
	if(sys != NULL || pt != NULL)
	{
		//I don't think this is a good way of dealing with this
		LOG(LOG_ERROR,"lightspark.SystemState instance already exists");
		exit(-1);
	}
	
	pt = new ParseThread(NULL,*(istream*)lightspark_state->context_istream);
	
	//NOTE: see SystemState declaration
	sys=new SystemState(pt);
	
	//Set a bit of SystemState using parameters
	if(lightspark_state->url)
		sys->setUrl(lightspark_state->url);

	//Set a bit of SystemState using lightspark_state
	//One of useInterpreter or useJit must be enabled
	if(!(lightspark_state->useInterpreter || lightspark_state->useJit))
	{
		LOG(LOG_ERROR,"No execution model enabled");
		exit(-1);
	}
	sys->useInterpreter=lightspark_state->useInterpreter;
	sys->useJit=lightspark_state->useJit;
	
	if(lightspark_state->paramsFileName)
		sys->parseParametersFromFile(lightspark_state->paramsFileName);

	sys->setOrigin(lightspark_state->filename);

	//TODO : add something for SDL
	sys->setParamsAndEngine(SDL, NULL);
	sys->downloadManager=new CurlDownloadManager();
	//Start the parser
	sys->addJob(pt);

	sys->wait();
	pt->wait();
}

void destroy_lightspark(lightspark_system_state* lightspark_state)
{
	if(lightspark_state->context_istream) delete ((istream*)lightspark_state->context_istream); lightspark_state->context_istream = NULL;
	if(lightspark_state->zlibfilter) delete ((zlib_file_filter*)lightspark_state->zlibfilter); lightspark_state->zlibfilter = NULL;
	
	if(sys) delete sys; sys = NULL; //Correct?
	if(pt) delete pt; pt = NULL;
}

} // extern "C"