#ifndef TRACKNEW_HPP 
#define TRACKNEW_HPP 

#include <stdlib.h>

#include <new>
#include <cstdio>
#include <cstdint>


class TrackNew {
private:
	inline static int numMalloc = 0;
	inline static std::size_t sumSize = 0;
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
			p = ::operator new(size, std::align_val_t{align});
		}
		if (doTrace) {
			// DON'T use std::cout here because it might allocate memory
			// while we are allocating memory(core dump at best)
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

void operator delete(void* p) noexcept {
	::free(p);
}

void operator delete(void* p, std::size_t) noexcept {
	::free(p);
}

void operator delete(void* p, std::align_val_t) noexcept {
	::free(p);
}

void operator delete(void* p, std::size_t, std::align_val_t align) noexcept {
	::free(p);
}

#endif

/*

int main()
{
    // allocate some memory on the stack:
    std::array<std::byte, 200000> buf;
    for (int num : {1000, 2000, 500, 2000, 3000, 50000, 1000}) 
    {
        std::cout << "-- check with " << num << " elements:\n";
        TrackNew::reset();
        std::pmr::monotonic_buffer_resource pool{buf.data(), buf.size()};
        std::pmr::vector<std::pmr::string> coll{&pool};
        for (int i=0; i < num; ++i) 
        {
            coll.emplace_back("just a non-SSO string");
        }
        TrackNew::status();
    }
}

*/