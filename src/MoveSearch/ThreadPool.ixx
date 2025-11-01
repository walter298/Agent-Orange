export module Chess.MoveSearch:ThreadPool;

export import std;

namespace chess {
	namespace threadpool {
		void postTaskImpl(std::move_only_function<void()> func);

		//prevent posting throwable functions
		export template<typename Func>
		void post(Func&& func) requires(std::is_nothrow_invocable_v<std::decay_t<Func>>) {
			postTaskImpl(std::forward<Func>(func));
		}

		export void join();
	}
}