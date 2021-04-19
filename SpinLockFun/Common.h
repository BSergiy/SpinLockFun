#pragma once

#ifdef _MSC_VER > 1921
	#define ___CPU_DELAY ::_mm_pause()
#else
	#define ___CPU_DELAY std::this_thread::yield()
#endif


