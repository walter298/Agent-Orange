export module Chess.ArenaAllocator;

export namespace chess {
	namespace arena {
		void* allocImpl(size_t size, size_t alignment);

		template<typename T>
		T* alloc() {
			return static_cast<T*>(allocImpl(sizeof(T), alignof(T)));
		}

		void init(size_t size);
		void reset();
	}
}