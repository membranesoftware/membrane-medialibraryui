/*
* Copyright 2018-2022 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
#include "SDL2/SDL.h"
#include "Buffer.h"
#include "SharedBuffer.h"

SharedBuffer::SharedBuffer ()
: Buffer ()
, refcount (0)
, refcountMutex (NULL)
{
	refcountMutex = SDL_CreateMutex ();
}

SharedBuffer::~SharedBuffer () {
	if (refcountMutex) {
		SDL_DestroyMutex (refcountMutex);
		refcountMutex = NULL;
	}
}

void SharedBuffer::retain () {

	SDL_LockMutex (refcountMutex);
	++refcount;
	if (refcount < 1) {
		refcount = 1;
	}
	SDL_UnlockMutex (refcountMutex);
}

void SharedBuffer::release () {
	bool isdestroyed;

	isdestroyed = false;
	SDL_LockMutex (refcountMutex);
	--refcount;
	if (refcount <= 0) {
		refcount = 0;
		isdestroyed = true;
	}
	SDL_UnlockMutex (refcountMutex);
	if (isdestroyed) {
		delete (this);
	}
}
