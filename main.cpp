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

#include <time.h>
#ifndef WIN32
#include <sys/resource.h>
#include <dlfcn.h>
#endif
#include <iostream>
#include <fstream>
#include "compat.h"
#include <SDL.h>
#ifdef WIN32
#undef main
#endif

#include "c_interface.h"

using namespace std;

class SharedLib
{
public:
	SharedLib(const char* lib);
	~SharedLib();

	void* getSymbol(const char* symbolname);
private:
#ifndef WIN32
	void* libHandle;
#else
	HMODULE libHandle;
#endif
};

SharedLib::SharedLib(const char* lib)
{
#ifndef WIN32
	liblightspark = dlopen(lib, RTLD_NOW);
	if(!liblightspark)
	{		
		cerr << "Unable to load: " << lib << endl;
		exit(-1);
	}
	dlerror(); //Reset error state
#else
	libHandle = LoadLibrary("spark.dll");
	if (libHandle == NULL)
	{
		cerr << "Unable to load " << lib << " " << GetLastError() << endl;
		exit(-1);
	}
#endif
}

SharedLib::~SharedLib()
{
#ifndef WIN32
	dlclose(libHandle);
#else
	if (!FreeLibrary(libHandle))
	{
		cerr << "Unable to unload lib: " << GetLastError() << endl;
	}
#endif
}

void* SharedLib::getSymbol(const char* symbolname)
{
#ifndef WIN32
	void* ret = dlsym(libHandle, symbolname);
	const char* dlsym_error = dlerror(); 
	if (dlsym_error) 
	{ 
		cerr << "Cannot load symbol create: " << dlsym_error << endl; 
		exit(-1); 
	} 
	dlerror();
	return ret;
#else
	void* ret = GetProcAddress(libHandle, symbolname);
	if (ret == NULL)
	{
		cerr << "Unable to get symbol " << symbolname << " " << GetLastError() << endl;
		exit(-1);
	}
	return ret;
#endif
}

int main(int argc, char* argv[])
{
	SharedLib spark(LIB_SPARK_NAME);
	lightspark_api_func init_lightspark = (lightspark_api_func) spark.getSymbol("init_lightspark"); 
	lightspark_api_func run_lightspark = (lightspark_api_func) spark.getSymbol("run_lightspark");
	lightspark_api_func destroy_lightspark = (lightspark_api_func) spark.getSymbol("destroy_lightspark");
	lightspark_api_func lightspark_system_state_defaults = (lightspark_api_func) spark.getSymbol("lightspark_system_state_defaults");

	lightspark_system_state lightspark_state;

	//Initialize state structure
	lightspark_system_state_defaults(&lightspark_state);

	//Parse args
	for(int i=1;i<argc;i++)
	{
		if(strcmp(argv[i],"-u")==0 || 
			strcmp(argv[i],"--url")==0)
		{
			i++;
			if(i==argc)
			{
				lightspark_state.filename=NULL;
				break;
			}

			lightspark_state.url=argv[i];
		}
		else if(strcmp(argv[i],"-ni")==0 || 
			strcmp(argv[i],"--disable-interpreter")==0)
		{
			lightspark_state.useInterpreter=false;
		}
		else if(strcmp(argv[i],"-j")==0 || 
			strcmp(argv[i],"--enable-jit")==0)
		{
			lightspark_state.useJit=true;
		}
		else if(strcmp(argv[i],"-l")==0 || 
			strcmp(argv[i],"--log-level")==0)
		{
			i++;
			if(i==argc)
			{
				lightspark_state.filename=NULL;
				break;
			}

			lightspark_state.log_level=atoi(argv[i]);
		}
		else if(strcmp(argv[i],"-p")==0 || 
			strcmp(argv[i],"--parameters-file")==0)
		{
			i++;
			if(i==argc)
			{
				lightspark_state.filename=NULL;
				break;
			}
			lightspark_state.paramsFileName=argv[i];
		}
		else
		{
			//No options flag, so set the swf file name
			if(lightspark_state.filename) //If already set, exit in error status
			{
				lightspark_state.filename=NULL;
				break;
			}
			lightspark_state.filename=argv[i];
		}
	}


	if(lightspark_state.filename==NULL)
	{
		cout << "Usage: " << argv[0] << " [--url|-u http://loader.url/file.swf] [--disable-interpreter|-ni] [--enable-jit|-j] [--log-level|-l 0-4] [--parameters-file|-p params-file] <file.swf>" << endl;
		exit(-1);
	}

#ifndef WIN32
	struct rlimit rl;
	getrlimit(RLIMIT_AS,&rl);
	rl.rlim_cur=400000000;
	rl.rlim_max=rl.rlim_cur;
	//setrlimit(RLIMIT_AS,&rl);

#endif
	
	SDL_Init ( SDL_INIT_VIDEO |SDL_INIT_EVENTTHREAD );
	
	//Init
	init_lightspark(&lightspark_state);
	
	//Run
	//TODO Tick-based running?
	run_lightspark(&lightspark_state);
	
	//Cleanup
	destroy_lightspark(&lightspark_state);
	
	SDL_Quit();

	return 0;
}

