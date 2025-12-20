export module Chess.SafeInt;

import std;

import Chess.Assert;

namespace chess {
	export template<std::unsigned_integral T>
	class SafeUnsigned {
	private:
		static constexpr T MAX_VALUE = std::numeric_limits<T>::max();
		T m_value = 0;
	public:
		explicit constexpr SafeUnsigned(T t) : m_value{ t } {}

		constexpr auto& get(this auto&& self) {
			return self.m_value;
		}

		constexpr SafeUnsigned& operator=(SafeUnsigned t) {
			m_value = t.m_value;
			return *this;
		}
		constexpr SafeUnsigned& operator++() {
			zAssert(MAX_VALUE - m_value > 0);
			++m_value;
			return *this;
		}
		constexpr SafeUnsigned& operator--() {
			zAssert(m_value > 0);
			--m_value;
			return *this;
		}
		constexpr SafeUnsigned& operator+=(SafeUnsigned t) {
			zAssert(MAX_VALUE - m_value >= t.m_value);
			m_value += t.m_value;
			return *this;
		}
		constexpr SafeUnsigned& operator*=(SafeUnsigned t) {
			zAssert(MAX_VALUE / m_value >= t.m_value);
			m_value *= t.m_value;
			return *this;
		}
		constexpr SafeUnsigned& operator/=(SafeUnsigned t) {
			m_value / t.m_value;
			return *this;
		}
		constexpr auto operator<=>(const SafeUnsigned& t) const = default;

		constexpr friend SafeUnsigned operator+(SafeUnsigned a, SafeUnsigned b) {
			zAssert(MAX_VALUE - a.m_value >= b.m_value);
			return SafeUnsigned{ static_cast<T>(a.m_value + b.m_value) }; //cast to stop integer promotion from kicking in
		}
		constexpr friend SafeUnsigned operator-(SafeUnsigned a, SafeUnsigned b) {
			zAssert(a.m_value >= b.m_value);
			return SafeUnsigned{ static_cast<T>(a.m_value - b.m_value) }; 
		}
		constexpr friend SafeUnsigned operator*(SafeUnsigned a, SafeUnsigned b) {
			zAssert(MAX_VALUE / a.m_value >= b.m_value);
			return SafeUnsigned{ static_cast<T>(a.m_value * b.m_value) }; 
		}
		constexpr friend SafeUnsigned operator/(SafeUnsigned a, SafeUnsigned b) {
			return SafeUnsigned{ static_cast<T>(a.m_value / b.m_value) };
		}
	};

	template<std::unsigned_integral U, std::integral C>
	constexpr SafeUnsigned<U> convertImpl(C v) {
		zAssert(std::cmp_greater_equal(v, 0) && std::cmp_less_equal(v, std::numeric_limits<U>::max()));
		return SafeUnsigned{ static_cast<U>(v) };
	};

	export SafeUnsigned<std::uint8_t> operator""_su8(unsigned long long v) {
		return convertImpl<std::uint8_t>(v);
	}
}