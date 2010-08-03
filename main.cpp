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
#endif
#include <iostream>
#include <fstream>
#include <list>

#ifdef WIN32
#include <windows.h>
#endif
#include <SDL.h>

#include "c_interface.h"

using namespace std;

int main(int argc, char* argv[])
{
	lightspark_system_state lightspark_state;
	
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

