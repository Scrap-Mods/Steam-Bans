#pragma once

#include "windows_include.hpp"
#include <cstdint>
#include <cstddef>

template  <typename T>
class VFTableHook
{
    std::uintptr_t* m_address;
    T m_originalFunction;
public:
    VFTableHook()
    {
        m_address = nullptr;
        m_originalFunction = nullptr;
    }
    ~VFTableHook() = default;

    void Hook(void* vftable, std::size_t index, void* function)
    {
        std::uintptr_t* vtable = reinterpret_cast<std::uintptr_t*>(vftable);
        m_address = &vtable[index];
        m_originalFunction = reinterpret_cast<T>(vtable[index]);

        DWORD oldProtect;
        VirtualProtect(m_address, sizeof(std::uintptr_t), PAGE_EXECUTE_READWRITE, &oldProtect);
        *m_address = reinterpret_cast<std::uintptr_t>(function);
        VirtualProtect(m_address, sizeof(std::uintptr_t), oldProtect, &oldProtect);
    }

    bool IsHooked() const
    {
        if (m_address == nullptr)
            return false;

        return *m_address != m_originalFunction;
    }

    void Unhook()
    {
        if (m_address == nullptr)
            return;

        DWORD oldProtect;
        VirtualProtect(m_address, sizeof(std::uintptr_t), PAGE_EXECUTE_READWRITE, &oldProtect);
        *m_address = reinterpret_cast<std::uintptr_t>(m_originalFunction);
        VirtualProtect(m_address, sizeof(std::uintptr_t), oldProtect, &oldProtect);
    }

    T GetOriginalFunction() const
    {
        return m_originalFunction;
    }
};

