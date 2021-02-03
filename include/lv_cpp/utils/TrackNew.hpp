/*
	This header only avaliable for MSVC in Windows
	and GCC for Linux
*/
#pragma once
#ifndef TRACKNEW_HPP 
#define TRACKNEW_HPP 
#include <new>
#include <cstdio>
#include <cstdlib>
#include <cstdint>

#ifdef _MSC_VER 
#include <malloc.h> // for_aligned_malloc() and_aligned_free() 
#endif 

class TrackNew {
private:
	inline static int numMalloc = 0;
	inline static size_t sumSize = 0;
	inline static bool doTrace = false;
	inline static bool inNew = false;

public:
	static void reset() {
		numMalloc = 0;
		sumSize = 0;
	}

	static void trace(bool b) {
		doTrace = b;
	}

	static void* allocate(std::size_t size, std::size_t align, const char* call) {
		++numMalloc;
		sumSize += size;
		void* p;
		if (align == 0) {
			p = std::malloc(size);
		}
		else {
#ifdef _MSC_VER
			p = _aligned_malloc(size, align); // Windows API
#else

			p = std::aligned_alloc(align, size); // C++17 API
			/*
			#if __cplusplus >= 201703L && defined(_GLIBCXX_HAVE_ALIGNED_ALLOC)
  			using ::aligned_alloc;
			#endif
			*/
			// it doesn't work... you can use it in VS2019
			// https://stackoverflow.com/questions/29247065/compiler-cant-find-aligned-alloc-function
			
#endif // _MSC_VER
		}
		if (doTrace) {
			// DON'T use std::cout here because it might allocate memory
			// while we are allocating memory(core dump at best
			printf("#%d %s", numMalloc, call);
			printf("(%zu bytes, ", size);
			if (align > 0) {
				printf("%zu-byte aligned) ", align);
			}
			else {
				printf("def-aligned) ");
			}
			printf("=> %p(total: %zu bytes)\n)", (void*)p, sumSize);
		}
		return p;
	}

	static void status() {
		printf("%d allocations for %zu bytes\n", numMalloc, sumSize);
	}

};

[[nodiscard]]
void* operator new(std::size_t size) {
	return TrackNew::allocate(size, 0, "::new");
}

[[nodiscard]]
void* operator new(std::size_t size, std::align_val_t align) {
	return TrackNew::allocate(size, static_cast<size_t>(align), "::new aligned");
}

[[nodiscard]]
void* operator new[](std::size_t size) {
	return TrackNew::allocate(size, 0, "::new []");
}

[[nodiscard]]
void* operator new[](std::size_t size, std::align_val_t align) {
	return TrackNew::allocate(size, static_cast<size_t>(align), "::new[] aligned");
}

[[nodiscard]]
void operator delete(void* p) noexcept {
	std::free(p);
}

void operator delete(void* p, std::size_t) noexcept {
	::operator delete(p);
}

void operator delete(void* p, std::align_val_t) noexcept {
#ifdef _MSC_VER
	_aligned_free(p); // Windows API
#else
	std::free(p); // C++17 API
#endif
}

void operator delete(void* p, std::size_t, std::align_val_t align) noexcept {
	::operator delete(p, align);
}
#endif