/*
* Copyright 2018-2021 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
*
* 3. Neither the name of the copyright holder nor the names of its contributors
* may be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/
#include "Config.h"
#include <stdlib.h>
#include <list>
#include "SDL2/SDL.h"
#include "StdString.h"
#include "Log.h"
#include "TaskGroup.h"

TaskGroup *TaskGroup::instance = NULL;

TaskGroup::TaskGroup ()
: isStopped (false)
, contextListMutex (NULL)
{
	contextListMutex = SDL_CreateMutex ();
}

TaskGroup::~TaskGroup () {
	if (contextListMutex) {
		SDL_DestroyMutex (contextListMutex);
		contextListMutex = NULL;
	}
}

bool TaskGroup::run (TaskGroup::RunContext fn, TaskGroup::EndCallbackContext endCallback) {
	TaskGroup::TaskContext ctx;

	if (isStopped) {
		return (false);
	}
	ctx.fn = fn;
	ctx.endCallback = endCallback;

	SDL_LockMutex (contextListMutex);
	contextList.push_back (ctx);
	SDL_UnlockMutex (contextListMutex);

	return (true);
}

int TaskGroup::executeTask (void *taskContextPtr) {
	TaskGroup::TaskContext *ctx;

	ctx = (TaskGroup::TaskContext *) taskContextPtr;
	ctx->fn.fn (ctx->fn.fnData);
	ctx->isRunning = false;

	return (0);
}

void TaskGroup::update (int msElapsed) {
	std::list<TaskGroup::TaskContext>::iterator i, end;
	std::list<TaskGroup::TaskContext> cblist;
	int result;
	bool found;

	SDL_LockMutex (contextListMutex);
	i = contextList.begin ();
	end = contextList.end ();
	while (i != end) {
		if (! i->thread) {
			i->isRunning = true;
			i->thread = SDL_CreateThread (TaskGroup::executeTask, "TaskGroup::executeTask", (void *) &(*i));
		}
		++i;
	}

	while (true) {
		found = false;
		i = contextList.begin ();
		end = contextList.end ();
		while (i != end) {
			if (i->thread && (! i->isRunning)) {
				found = true;
				SDL_WaitThread (i->thread, &result);
				i->thread = NULL;
				if (i->endCallback.callback) {
					cblist.push_back (*i);
				}
				contextList.erase (i);
				break;
			}
			++i;
		}
		if (! found) {
			break;
		}
	}
	SDL_UnlockMutex (contextListMutex);

	i = cblist.begin ();
	end = cblist.end ();
	while (i != end) {
		i->endCallback.callback (i->endCallback.callbackData, i->fn.fnData);
		++i;
	}
}

void TaskGroup::stop () {
	isStopped = true;
}

bool TaskGroup::isStopComplete () {
	bool result;

	if (! isStopped) {
		return (false);
	}
	SDL_LockMutex (contextListMutex);
	result = contextList.empty ();
	SDL_UnlockMutex (contextListMutex);

	return (result);
}

void TaskGroup::waitThreads () {
	std::list<TaskGroup::TaskContext>::iterator i, end;
	int result;

	SDL_LockMutex (contextListMutex);
	i = contextList.begin ();
	end = contextList.end ();
	while (i != end) {
		if (i->thread) {
			SDL_WaitThread (i->thread, &result);
			i->thread = NULL;
		}
		++i;
	}
	contextList.clear ();
	SDL_UnlockMutex (contextListMutex);
}