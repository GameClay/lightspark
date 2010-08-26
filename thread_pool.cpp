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

#include "thread_pool.h"
#include "exceptions.h"
#include "compat.h"
#include "logger.h"
#include "swf.h"

using namespace lightspark;

extern TLSDATA SystemState* sys;
TLSDATA lightspark::IThreadJob* thisJob=NULL;

ThreadPool::ThreadPool(SystemState* s):stopFlag(false)
{
	m_sys=s;
	sem_init(&mutex,0,1);
	sem_init(&num_jobs,0,0);
	for(int i=0;i<NUM_THREADS;i++)
	{
		curJobs[i]=NULL;
		pthread_create(&threads[i],NULL,job_worker,this);
	}
}

void ThreadPool::stop()
{
	stopFlag=true;
	//Signal an event for all the threads
	for(int i=0;i<NUM_THREADS;i++)
		amp_semaphore_signal(num_jobs);

}

ThreadPool::~ThreadPool()
{
	stop();
	//Now abort any job that is still executing
	amp_semaphore_wait(mutex);
	for(int i=0;i<NUM_THREADS;i++)
	{
		if(curJobs[i])
			curJobs[i]->stop();
	}
	amp_semaphore_signal(mutex);

	for(int i=0;i<NUM_THREADS;i++)
	{
		if(pthread_join(threads[i],NULL)!=0)
			LOG(LOG_ERROR,"pthread_join failed in ~ThreadPool");
	}

	sem_destroy(&num_jobs);
	sem_destroy(&mutex);
}

void* ThreadPool::job_worker(void* t)
{
	ThreadPool* th=static_cast<ThreadPool*>(t);
	sys=th->m_sys;

	//Let's find out index (slow, but it's done only once)
	uint32_t index=0;
	for(;index<NUM_THREADS;index++)
	{
		if(pthread_equal(th->threads[index],pthread_self()))
			break;
	}

	while(1)
	{
		amp_semaphore_wait(th->num_jobs);
		if(th->stopFlag)
			pthread_exit(0);
		amp_semaphore_wait(th->mutex);
		IThreadJob* myJob=th->jobs.front();
		th->jobs.pop_front();
		th->curJobs[index]=myJob;
		myJob->executing=true;
		amp_semaphore_signal(th->mutex);

		assert(thisJob==NULL);
		thisJob=myJob;
		try
		{
			myJob->run();
		}
		catch(LightsparkException& e)
		{
			LOG(LOG_ERROR,"Exception in ThreadPool " << e.what());
			sys->setError(e.cause);
		}
		thisJob=NULL;

		amp_semaphore_wait(th->mutex);
		myJob->executing=false;
		th->curJobs[index]=NULL;
		if(myJob->destroyMe)
			delete myJob;
		amp_semaphore_signal(th->mutex);
	}
	return NULL;
}

void ThreadPool::addJob(IThreadJob* j)
{
	amp_semaphore_wait(mutex);
	jobs.push_back(j);
	amp_semaphore_signal(mutex);
	amp_semaphore_signal(num_jobs);
}

