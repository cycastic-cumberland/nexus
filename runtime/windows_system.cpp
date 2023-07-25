//
// Created by cycastic on 7/22/2023.
//

#include "windows_system.h"
#include "../core/io/file_access_server.h"


#if defined(_WIN32) || defined(_WIN64)
#include <dwmapi.h>
#include <shellapi.h>
#endif

static VString format_error_message(DWORD id) {
    LPWSTR messageBuffer = NULL;
    size_t size = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, id, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, NULL);

    VString msg = VString("Error ") + itos(id) + ": " + VString(messageBuffer, size);

    LocalFree(messageBuffer);

    return msg;
}

void WindowsSystem::load_dynamic_library(const VString &p_lib_path, void *&p_library_handle,
                                         const bool &p_also_set_library_path) {
    auto path = FileAccessServer::path_fix(p_lib_path);
    if (!FileAccessServer::exists(p_lib_path)) throw SystemException("Library not exists");

    // Stolen from Godot
    typedef DLL_DIRECTORY_COOKIE(WINAPI * PAddDllDirectory)(PCWSTR);
    typedef BOOL(WINAPI * PRemoveDllDirectory)(DLL_DIRECTORY_COOKIE);

    auto add_dll_directory = (PAddDllDirectory)GetProcAddress(GetModuleHandle("kernel32.dll"), "AddDllDirectory");
    auto remove_dll_directory = (PRemoveDllDirectory)GetProcAddress(GetModuleHandle("kernel32.dll"), "RemoveDllDirectory");

    bool has_dll_directory_api = ((add_dll_directory != NULL) && (remove_dll_directory != NULL));
    DLL_DIRECTORY_COOKIE cookie = NULL;

    if (p_also_set_library_path && has_dll_directory_api) {
        cookie = add_dll_directory(path.get_base_dir().c_str());
    }

    p_library_handle = (void *)LoadLibraryExW(path.c_str(), NULL, (p_also_set_library_path && has_dll_directory_api) ? LOAD_LIBRARY_SEARCH_DEFAULT_DIRS : 0);
    if (!p_library_handle) throw SystemException(CharString("Cannot open library ") + p_lib_path.utf8() + ", error: " + format_error_message(GetLastError()).utf8() + ".");
    if (cookie) {
        remove_dll_directory(cookie);
    }
}

bool WindowsSystem::close_dynamic_library(void *p_library_handle) {
    return FreeLibrary((HMODULE)p_library_handle);
}

void WindowsSystem::get_dynamic_library_symbol_handle(void *p_library_handle, const CharString &p_name, void *&p_symbol_handle,
                                                 const bool &p_optional) {
    p_symbol_handle = (void *)GetProcAddress((HMODULE)p_library_handle, p_name.c_str());
    if (!p_symbol_handle) {
        if (!p_optional) {
            throw SystemException(CharString("Cannot resolve symbol: ") + p_name + ", error: " + itos(GetLastError()).utf8());
        } else {
            throw SystemException(CharString("Cannot resolve symbol: ") + p_name);
        }
    }
}

WindowsSystem::WindowsSystem() : System() {
    singleton = this;
}

void WindowsSystem::print_line(const VString &p_message) {
    std::wcout << p_message << L'\n';
}

WindowsSystem::~WindowsSystem() = default;
