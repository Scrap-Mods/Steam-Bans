#include "Console.h"

HANDLE Console::Handle = NULL;
__ConsoleOutputHandler Console::Out = __ConsoleOutputHandler();

bool Console::Create(const wchar_t* title)
{
	if (Console::Handle == NULL)
	{
		if (AllocConsole())
		{
			SetConsoleOutputCP(CP_UTF8);
			SetConsoleTitleW(title);

			Console::Handle = GetStdHandle(STD_OUTPUT_HANDLE);

			return true;
		}
	}

	return false;
}

bool Console::Attach()
{
	Console::Handle = GetStdHandle(STD_OUTPUT_HANDLE);
	return Console::Handle != nullptr;
}

void Console::Destroy()
{
	if (Console::Handle == NULL) return;

	FreeConsole();
	Console::Handle = NULL;
}
