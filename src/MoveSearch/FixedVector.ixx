export module Chess.MoveSearch:FixedVector;

export import std;

export namespace chess {
	template<typename T>
	class FixedVector {
	private:
		std::vector<T> m_data;
	public:
		explicit FixedVector(std::vector<T>&& data) : m_data{ std::move(data) } {}

		auto begin() const {
			return m_data.begin();
		}
		auto end() const {
			return m_data.end();
		}
		const T& operator[](size_t index) const {
			return m_data[index];
		}
		size_t size() const {
			return m_data.size();
		}
	};
}