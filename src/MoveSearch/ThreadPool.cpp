module;

#include <cassert>

module Chess.MoveSearch:ThreadPool;

import std;

namespace {
	void pushFreeTaskIndex(size_t index);

	class Task {
	private:
		struct State {
			std::mutex mutex;
			std::condition_variable cv;
			std::move_only_function<void()> work;
			bool hasWork = false;
			bool cancelled = false;
		};
		size_t m_index;
		std::unique_ptr<State> m_state;
	public:
		explicit Task(size_t index) : m_index{ index }, m_state { std::make_unique<State>() }
		{
		}

		Task(Task&&) noexcept = default;
		
		~Task() {
			cancel();
		}

		void run() {
			using namespace std::literals;
			while (true) {
				std::unique_lock l{ m_state->mutex };
				m_state->cv.wait(l, [this] { return m_state->hasWork || m_state->cancelled; });
				if (m_state->cancelled) {
					break;
				}
				auto workCopy = std::move(m_state->work);
				l.unlock();

				workCopy();

				l.lock();
				m_state->hasWork = false;
				pushFreeTaskIndex(m_index);
			}
		}

		bool post(std::move_only_function<void()> work) {
			{
				std::scoped_lock lock{ m_state->mutex };
				m_state->work = std::move(work);
				m_state->hasWork = true;
			}

			m_state->cv.notify_one();
			
			return true;
		}

		void cancel() {
			{
				std::scoped_lock l{ m_state->mutex };
				m_state->cancelled = true;
			}
			m_state->cv.notify_one();
		}
	};

	class TaskManager {
	private:
		std::mutex m_mutex;
		std::condition_variable m_freeTaskCV;
		std::condition_variable m_joinedCV;
		std::vector<size_t> m_freeTaskIDs;
		std::vector<Task> m_tasks;
		std::vector<std::jthread> m_threads;
		bool m_joining = true;

		bool isTaskFree() const {
			return !m_freeTaskIDs.empty();
		}

		size_t getFreeTaskID() {
			assert(isTaskFree());
			auto back = m_freeTaskIDs.back();
			m_freeTaskIDs.pop_back();
			return back;
		}
		size_t waitForFreeTaskIndex(std::unique_lock<std::mutex>& l) {
			if (isTaskFree()) {
				return getFreeTaskID();
			}
			m_freeTaskCV.wait(l, [this] { return isTaskFree(); });
			return getFreeTaskID();
		}
	public:
		TaskManager() {
			auto threadCount = std::thread::hardware_concurrency();
			m_freeTaskIDs.reserve(threadCount);
			m_tasks.reserve(threadCount);

			Task task{ 0 };

			for (decltype(threadCount) i = 0; i < threadCount; i++) {
				m_tasks.emplace_back(static_cast<size_t>(i));
				m_freeTaskIDs.emplace_back(i);
			}
			for (decltype(threadCount) i = 0; i < threadCount; i++) {
				m_threads.emplace_back([i, this] {
					m_tasks[i].run();
				});
			}
		}

		~TaskManager() {
			for (auto& task : m_tasks) {
				task.cancel();
			}
		}

		void join() {
			std::unique_lock l{ m_mutex };
			m_joinedCV.wait(l, [this] {
				return m_freeTaskIDs.size() == std::thread::hardware_concurrency();
			});
		}

		void updateFreeTaskID(size_t index) {
			auto allTasksDone = false;
			{
				std::scoped_lock l{ m_mutex };
				m_freeTaskIDs.push_back(index);
				allTasksDone = (m_freeTaskIDs.size() == std::thread::hardware_concurrency());
			}
			if (allTasksDone) {
				m_joinedCV.notify_one();
			}
			m_freeTaskCV.notify_all();
		}

		void post(std::move_only_function<void()> work) {
			std::unique_lock l{ m_mutex };
			auto taskIndex = waitForFreeTaskIndex(l);
			m_tasks[taskIndex].post(std::move(work));
		}
	};

	TaskManager taskManager;

	void pushFreeTaskIndex(size_t index) {
		taskManager.updateFreeTaskID(index);
	}
}

namespace chess {
	namespace threadpool {
		void postTaskImpl(std::move_only_function<void()> func) {
			taskManager.post(std::move(func));
		}
		void join() {
			taskManager.join();
		}
	}
}