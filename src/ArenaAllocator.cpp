module Chess.ArenaAllocator;

import std;
import Chess.Assert;

namespace chess {
	namespace arena {
		class MemoryRegion {
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

			inline std::byte* alloc(size_t bytes, size_t alignment = sizeof(std::max_align_t)) noexcept {
				zAssert(bytes > 0);
				if (std::align(alignment, bytes, m_nextObjectBegin, m_space)) {
					auto temp = m_nextObjectBegin;
					m_nextObjectBegin = static_cast<std::byte*>(m_nextObjectBegin) + bytes;
					m_space -= bytes;
					return static_cast<std::byte*>(temp);
				} else {
					std::abort();
				}
			}

			void clear() noexcept {
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

			MemoryRegion makeSubregion(size_t offset, size_t len) noexcept {
				zAssert(m_capacity - offset >= len);
				return { static_cast<std::byte*>(m_begin) + offset, len };
			}

			template<typename T>
			std::span<T> interpretAsSpan() {
				zAssert(reinterpret_cast<std::uintptr_t>(m_begin) % alignof(T) == 0);
				zAssert(m_capacity % sizeof(T) == 0);
				return std::span{
					static_cast<T*>(m_begin), m_capacity / sizeof(T)
				};
			}
		};

		std::unique_ptr<std::byte[]> buff;
		MemoryRegion region;

		void* allocImpl(size_t size, size_t alignment) {
			return region.alloc(size, alignment);
		}

		void init(size_t size) {
			buff = std::make_unique<std::byte[]>(size);
			region = { buff.get(), size };
		}

		void reset() {
			region.clear();
		}
	}
}