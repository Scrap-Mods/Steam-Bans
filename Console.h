#pragma once

#include "windows_include.hpp"

#include "ConColors.hpp"

#include <sstream>
#include <iomanip>
#include <string>
#include <mutex>

#define CREATE_CON_OUTPUT_TYPE(type, arg, func_text) \
template<>                                              \
struct ConsoleOutputType<type>                          \
{                                                       \
	inline static void Output(arg) func_text            \
}                                                                                                                

#define CREATE_CON_NUMBER_TYPE(type)                       \
template<>                                                    \
struct ConsoleOutputType<type>                                \
{                                                             \
	inline static void Output(const type& number)             \
	{                                                         \
		Console::Output<std::string>(std::to_string(number)); \
	}                                                         \
}

#if defined(CON_ENABLE_CONSOLE_WRITE) || !defined(CON_ENABLE_VS_CONSOLE_OUTPUT)
#define CON_WRITE_CONSOLE_ASCII(handle, msg_ptr, msg_sz) WriteConsoleA(handle, msg_ptr, msg_sz, NULL, NULL)
#define CON_WRITE_CONSOLE_WIDE(handle, msg_ptr, msg_sz) WriteConsoleW(handle, msg_ptr, msg_sz, NULL, NULL)
#define CON_WRITE_DEBUG_STR_ASCII(...) ((void*)0)
#define CON_WRITE_DEBUG_STR_WIDE(...) ((void*)0)
#elif defined(CON_ENABLE_VS_CONSOLE_OUTPUT)
#define CON_WRITE_CONSOLE_ASCII(...) ((void*)0)
#define CON_WRITE_CONSOLE_WIDE(...) ((void*)0)
#define CON_WRITE_DEBUG_STR_ASCII(msg) OutputDebugStringA(msg)
#define CON_WRITE_DEBUG_STR_WIDE(msg) OutputDebugStringW(msg)
#endif

#define CON_WRITE_ASCII(handle, msg_ptr, msg_sz) \
	CON_WRITE_CONSOLE_ASCII(handle, msg_ptr, msg_sz); \
	CON_WRITE_DEBUG_STR_ASCII(msg_ptr)

#define CON_WRITE_WIDE(handle, msg_ptr, msg_sz) \
	CON_WRITE_CONSOLE_WIDE(handle, msg_ptr, msg_sz); \
	CON_WRITE_DEBUG_STR_WIDE(msg_ptr)

#if defined(CON_LOG_PREFIX)
#define CON_LOG_PREFIX_ARG CON_LOG_PREFIX,
#else
#define CON_LOG_PREFIX_ARG
#define CON_LOG_PREFIX
#endif

class __ConsoleOutputHandler;

template<class T>
struct ConsoleOutputType;

class Console
{
	template<class>
	friend struct ConsoleOutputType;

	friend __ConsoleOutputHandler;

public:
	inline static void Endl()
	{
		Console::Output<EngineConColor>(0b1110_fg);
		Console::Output<const char*>("\n");
	}

	//Creates a new debug console
	static bool Create(const wchar_t* title);
	//Attaches to an existing debug console
	static bool Attach();
	void Destroy();

	static __ConsoleOutputHandler Out;


	static HANDLE Handle;
private:
	template<class T>
	inline static void Output(const T arg)
	{
		ConsoleOutputType<T>::Output(arg);
	}

	Console() = default;
	~Console() = default;
};

//---------------CONSOLE TYPE DEFINITIONS--------------

template<class T>
struct ConsoleOutputType<T*>
{
	inline static void Output(T* ptr)
	{
		const std::uintptr_t m_PointerValue = reinterpret_cast<std::uintptr_t>(ptr);

		std::stringstream m_stream;
		m_stream << std::setfill('0') << std::setw(sizeof(std::uintptr_t) * 2) << std::hex << m_PointerValue;

		Console::Output(m_stream.str());
	}
};

template<>
struct ConsoleOutputType<const wchar_t*>
{
	inline static void Output(const wchar_t* arg)
	{
		CON_WRITE_WIDE(Console::Handle, arg, static_cast<DWORD>(wcslen(arg)));
	}
};

template<>
struct ConsoleOutputType<const char*>
{
	inline static void Output(const char* arg)
	{
		CON_WRITE_ASCII(Console::Handle, arg, static_cast<DWORD>(strlen(arg)));
	}
};

template<>
struct ConsoleOutputType<std::wstring>
{
	inline static void Output(const std::wstring& msg)
	{
		CON_WRITE_WIDE(Console::Handle, msg.data(), static_cast<DWORD>(msg.size()));
	}
};

template<>
struct ConsoleOutputType<std::string>
{
	inline static void Output(const std::string& msg)
	{
		CON_WRITE_ASCII(Console::Handle, msg.data(), static_cast<DWORD>(msg.size()));
	}
};

CREATE_CON_NUMBER_TYPE(unsigned long long);
CREATE_CON_NUMBER_TYPE(long long);
CREATE_CON_NUMBER_TYPE(unsigned long);
CREATE_CON_NUMBER_TYPE(long);
CREATE_CON_NUMBER_TYPE(unsigned int);
CREATE_CON_NUMBER_TYPE(int);
CREATE_CON_NUMBER_TYPE(unsigned short);
CREATE_CON_NUMBER_TYPE(short);
CREATE_CON_NUMBER_TYPE(unsigned char);
CREATE_CON_NUMBER_TYPE(char);

CREATE_CON_NUMBER_TYPE(float);
CREATE_CON_NUMBER_TYPE(double);

CREATE_CON_OUTPUT_TYPE(void (*)(), void (*func_ptr)(), { func_ptr(); });
CREATE_CON_OUTPUT_TYPE(EngineConColor, const EngineConColor& color, { SetConsoleTextAttribute(Console::Handle, static_cast<WORD>(color)); });
CREATE_CON_OUTPUT_TYPE(bool, const bool& bool_val, { ConsoleOutputType<std::string>::Output(bool_val ? "true" : "false"); });

//-------------CONSOLE OUTPUT HANDLER------------------

class __ConsoleOutputHandler
{
	friend class Console;

	std::mutex m_ConsoleMutex = std::mutex();

public:
	template<typename ...ArgList>
	inline void operator()(const ArgList& ...arg_list)
	{
		//lock the function to output messages from different threads properly
		std::lock_guard g_guard(m_ConsoleMutex);

		this->variadic_func(arg_list...);
	}

private:
	template<typename CurArg>
	inline void variadic_func(const CurArg& cur_arg)
	{
		Console::Output(cur_arg);
	}

	template<typename CurArg, typename ...ArgList>
	inline void variadic_func(const CurArg& cur_arg, const ArgList& ...arg_list)
	{
		this->variadic_func(cur_arg);
		this->variadic_func(arg_list...);
	}

	//Remove copy constructors
	__ConsoleOutputHandler(const __ConsoleOutputHandler&) = delete;
	__ConsoleOutputHandler(__ConsoleOutputHandler&&) = delete;

	__ConsoleOutputHandler() = default;
	~__ConsoleOutputHandler() = default;
};

#if defined(CON_ENABLE_CONSOLE_WRITE) || !defined(CON_ENABLE_VS_CONSOLE_OUTPUT)
#define CreateDebugConsole(title) Console::Create(title)
#define AttachToDebugConsole() Console::Attach()
#else
#define CreateDebugConsole(...) ((void*)0)
#define AttachToDebugConsole() ((void*)0)
#endif

#define DebugOut(...)  Console::Out(CON_LOG_PREFIX_ARG __VA_ARGS__)
#define DebugOutL(...) Console::Out(CON_LOG_PREFIX_ARG __VA_ARGS__, Console::Endl)

#define DebugWarningL(...) Console::Out(0b1101_fg, CON_LOG_PREFIX "WARNING: ", __FUNCTION__, "(", __LINE__, ") -> ", __VA_ARGS__, Console::Endl)
#define DebugErrorL(...)   Console::Out(0b1001_fg, CON_LOG_PREFIX "ERROR: "  , __FUNCTION__, "(", __LINE__, ") -> ", __VA_ARGS__, Console::Endl)
