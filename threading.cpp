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

#include <assert.h>

#include "threading.h"
#include "exceptions.h"
#include "logger.h"

using namespace lightspark;

//NOTE: thread jobs can be run only once
IThreadJob::IThreadJob():destroyMe(false),executing(false),aborting(false)
{
	amp_semaphore_create(&terminated, AMP_DEFAULT_ALLOCATOR, 0);
}

IThreadJob::~IThreadJob()
{
	if(executing)
		amp_semaphore_wait(terminated);
	sem_destroy(&terminated);
}

void IThreadJob::run()
{
	try
	{
		assert(thisJob);
		execute();
	}
	catch(JobTerminationException& ex)
	{
		LOG(LOG_NOT_IMPLEMENTED,"Job terminated");
	}

	amp_semaphore_signal(terminated);
}

void IThreadJob::stop()
{
	if(executing)
	{
		aborting=true;
		this->threadAbort();
	}
}

Mutex::Mutex(const char* n):name(n),foundBusy(0)
{
	amp_semaphore_create(&sem,AMP_DEFAULT_ALLOCATOR,1);
}

Mutex::~Mutex()
{
	if(name)
		LOG(LOG_TRACE,"Mutex " << name << " waited " << foundBusy << " times");
	sem_destroy(&sem);
}

void Mutex::lock()
{
	if(name)
	{
		//If the semaphore can be acquired immediately just return
		if(sem_trywait(&sem)==0)
			return;

		//Otherwise log the busy event and do a real wait
		foundBusy++;
	}

	amp_semaphore_wait(sem);
}

void Mutex::unlock()
{
	amp_semaphore_signal(sem);
}

Semaphore::Semaphore(uint32_t init)//:blocked(0),maxBlocked(max)
{
	amp_semaphore_create(&sem,AMP_DEFAULT_ALLOCATOR,init);
}

Semaphore::~Semaphore()
{
	//On destrucion unblocks the blocked thread
	signal();
	sem_destroy(&sem);
}

void Semaphore::wait()
{
	amp_semaphore_wait(sem);
}

bool Semaphore::try_wait()
{
	return sem_trywait(&sem)==0;
}

void Semaphore::signal()
{
	amp_semaphore_signal(sem);
}
