export module Chess.ArenaAllocator;

export import std;

namespace chess {
	namespace arena {
		void* allocImpl(size_t size, size_t alignment);

		export template<typename T>
		T* alloc() {
			return static_cast<T*>(allocImpl(sizeof(T), alignof(T)));
		}

		export void init(size_t size);
		export void reset();

		export template<typename T>
		struct Allocator {
			using value_type = T;

			T* allocate(size_t n) const noexcept {
				return static_cast<T*>(allocImpl(n * sizeof(T), alignof(T)));
			}

			void deallocate(T*, size_t) const noexcept {} //all memory is deallocated by reset()
		};

		export template<typename T>
		using Vector = std::vector<T>;
	}
}