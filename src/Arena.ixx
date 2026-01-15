export module Chess.Arena;

export import std;
export import BS.thread_pool;

namespace chess {
	namespace arena {
		export class MemoryRegion {
		private:
			void* m_begin = nullptr;
			void* m_nextObjectBegin = nullptr;
			size_t m_capacity = 0;
			size_t m_space = 0;
		public:
			MemoryRegion() = default;
			MemoryRegion(void* begin, size_t capacity) noexcept
				: m_begin{ begin }, m_nextObjectBegin{ begin }, m_capacity{ capacity }, m_space{ capacity }
			{
			}

			inline std::byte* allocate(size_t bytes, size_t alignment = alignof(std::max_align_t)) noexcept {
				if (std::align(alignment, bytes, m_nextObjectBegin, m_space)) {
					auto temp = m_nextObjectBegin;
					m_nextObjectBegin = static_cast<std::byte*>(m_nextObjectBegin) + bytes;
					m_space -= bytes;
					return static_cast<std::byte*>(temp);
				} else {
					std::abort();
				}
				std::unreachable();
			}

			void* getOffset() const {
				return m_nextObjectBegin;
			}
			void resetToOffset(void* newOffset) {
				m_nextObjectBegin = newOffset;
				m_space = m_capacity - static_cast<size_t>(static_cast<std::byte*>(newOffset) - static_cast<std::byte*>(m_begin));
			}

			void reset() noexcept {
				m_space = m_capacity;
				m_nextObjectBegin = m_begin;
			}

			size_t getTotalCapacity() const noexcept {
				return m_capacity;
			}

			size_t getSpace() const noexcept {
				return m_space;
			}

			size_t bytesAvailable() const noexcept {
				return m_space;
			}
		};

		export MemoryRegion* getMemoryRegion();
		export void init();
		export void resetThread();
		export void resetAllThreads();
		export void registerThread(std::jthread::id id);
		
		void* allocateImpl(size_t byteCount, size_t alignment);

		export template<typename T>
		struct Allocator {
			using value_type = T;

			Allocator() = default;

			template<typename U>
			Allocator(Allocator<U>) {}

			T* allocate(size_t n) const {
				return static_cast<T*>(allocateImpl(n * sizeof(T), alignof(T)));
			}

			void deallocate(T*, size_t) const {}

			template<typename U>
			struct rebind {
				using other = Allocator<U>;
			};

			friend bool operator==(const Allocator&, const Allocator&) {
				return true;
			}
		};

		export template<typename T>
		using Vector = std::vector<T, Allocator<T>>;
	}
}