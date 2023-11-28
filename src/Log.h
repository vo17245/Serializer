#pragma once
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"


extern const char* GetLogName();
class Log
{
public:
	~Log() {}
	static Log& Get()
	{
		static Log log;
		return log;
	}
	inline const std::shared_ptr<spdlog::logger>& GetLogger()const { return m_Logger; }
	
	template<typename... Args>
	static void Info(spdlog::format_string_t<Args...> fmt, Args &&... args)
	{
		Log::Get().GetLogger()->info(fmt, std::forward<Args>(args)...);
	}
	template<typename... Args>
	static void Debug(spdlog::format_string_t<Args...> fmt, Args &&... args)
	{
		Log::Get().GetLogger()->debug(fmt, std::forward<Args>(args)...);
	}
	template<typename... Args>
	static void Warn(spdlog::format_string_t<Args...> fmt, Args &&... args)
	{
		Log::Get().GetLogger()->warn(fmt, std::forward<Args>(args)...);
	}
	template<typename... Args>
	static void Error(spdlog::format_string_t<Args...> fmt, Args &&... args)
	{
		Log::Get().GetLogger()->error(fmt, std::forward<Args>(args)...);
	}
	template<typename... Args>
	static void Critical(spdlog::format_string_t<Args...> fmt, Args &&... args)
	{
		Log::Get().GetLogger()->critical(fmt, std::forward<Args>(args)...);
	}
private:
	Log()
	{
		spdlog::set_pattern("%^[%T] [%n] %v%$");
		m_Logger = spdlog::stdout_color_mt(GetLogName());
		m_Logger->set_level(spdlog::level::debug);
	}
	std::shared_ptr<spdlog::logger> m_Logger;
	
};

template<typename... Args>
inline void DebugLogFunc(const char* file, int line, const char* fmt, Args&&... args)
{
	static_assert(sizeof(std::thread::id)==sizeof(unsigned int));
	auto msg = fmt::format(fmt::runtime(fmt), std::forward<Args>(args)...);
	std::thread::id thread_id=std::this_thread::get_id();
	unsigned int uint_thread_id=*((unsigned int*)(&thread_id));
	std::string debugMsg = fmt::format("[thread {}][{}:{}] {}",uint_thread_id, file, line, msg);
	Log::Debug("{}",debugMsg.c_str());
}
template<typename... Args>
inline void DebugLogErrorFunc(const char* file, int line, const char* fmt, Args&&... args)
{
	static_assert(sizeof(std::thread::id)==sizeof(unsigned int));
	auto msg = fmt::format(fmt::runtime(fmt), std::forward<Args>(args)...);
	std::thread::id thread_id=std::this_thread::get_id();
	unsigned int uint_thread_id=*((unsigned int*)(&thread_id));
	std::string debugMsg = fmt::format("[thread {}][{}:{}] {}", uint_thread_id,file, line, msg);
	Log::Error("{}", debugMsg.c_str());
}



#ifdef ENABLE_DEBUG_LOG
	#define DEBUG_LOG(...) DebugLogFunc(__FILE__,__LINE__,__VA_ARGS__)
	#define DEBUG_LOG_ERROR(...) DebugLogErrorFunc(__FILE__,__LINE__,__VA_ARGS__)
#else
	#define DEBUG_LOG(...) ((void)0)
	#define DEBUG_LOG_ERROR(...) ((void)0)
#endif



