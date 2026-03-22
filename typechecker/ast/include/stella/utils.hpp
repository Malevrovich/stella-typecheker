#pragma once

#include <cstdlib>
#include <cxxabi.h>
#include <memory>
#include <string>

inline std::string tryDemangle(const char* mangled_name) noexcept {
    int status = 0;
    std::unique_ptr<char, decltype(&std::free)> demangled(
        abi::__cxa_demangle(mangled_name, nullptr, nullptr, &status), std::free);

    if (status == 0 && demangled) {
        return std::string(demangled.get());
    }
    return std::string(mangled_name);
}

inline std::string tryDemangle(const std::string& mangled_name) noexcept {
    return tryDemangle(mangled_name.c_str());
}