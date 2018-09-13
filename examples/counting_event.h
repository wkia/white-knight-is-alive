#include <mutex>

class CountingEvent
{
private:
	std::mutex m_mutex;
	std::condition_variable m_condition;
	unsigned long m_count;

public:
	CountingEvent(unsigned long initialValue) : m_count(initialValue) {}

	void notify()
	{
		std::unique_lock<decltype(m_mutex)> lock(m_mutex);
		--m_count;
		m_condition.notify_all();
	}

	template<class _Rep, class _Period>
	bool wait_for(const std::chrono::duration<_Rep, _Period>& timeout)
	{
		const auto end = std::chrono::system_clock::now() + timeout;
		std::unique_lock<decltype(m_mutex)> lock(m_mutex);
		if (!m_condition.wait_until(lock, end, [this]() { return 0 == m_count; }))
			return false;
		++m_count;
		return true;
	}
};
