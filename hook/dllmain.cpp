#include "pch.h"

#include "../MinHook/include/MinHook.h"

void* original_fn;
void* return_addr;

uint32_t button_state = 0;

__declspec(naked)
void pdiInitialise_patch()
{
    __asm
    {
        call    [original_fn]
        mov     eax, 0x8
        jmp     [return_addr]
    }
}

extern "C"
void pdiUpdate_patch(void)
{
    constexpr auto keys = std::array{ 0x5A, 0x53, 0x58, 0x44, 0x43, 0x46, 0x56, 0x47, 0x42 };

    auto const old_state = button_state;
    button_state = 0;

    for (std::size_t i = 0; i < keys.size(); ++i)
    {
        auto const key = 0b1 << i;
        auto const state = GetAsyncKeyState(keys[i]);

        if (state && (old_state & key))
            button_state |= key;
        else if (state)
            button_state |= key | (key << 0x10);
    }
}

extern "C"
uint32_t pdiGetBtnState_patch(void)
{
    return button_state;
}

BOOL APIENTRY DllMain(HMODULE, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        if (MH_Initialize() != MH_OK)
        {
            return TRUE;
        }

        constexpr uintptr_t pdiInitialise_addr  = 0x004081ca;
        constexpr uintptr_t pdiGetBtnState_addr = 0x100010f0;
        constexpr uintptr_t pdiUpdate_addr      = 0x10001550;

        original_fn = reinterpret_cast<void*>(0x10001490);
        return_addr = reinterpret_cast<void*>(pdiInitialise_addr + 6);

        MH_CreateHook(reinterpret_cast<void*>(pdiInitialise_addr), reinterpret_cast<void*>(&pdiInitialise_patch), nullptr);
        MH_CreateHook(reinterpret_cast<void*>(pdiGetBtnState_addr), reinterpret_cast<void*>(&pdiGetBtnState_patch), nullptr);
        MH_CreateHook(reinterpret_cast<void*>(pdiUpdate_addr), reinterpret_cast<void*>(&pdiUpdate_patch), nullptr);

        MH_EnableHook(MH_ALL_HOOKS);
    }

    return TRUE;
}

