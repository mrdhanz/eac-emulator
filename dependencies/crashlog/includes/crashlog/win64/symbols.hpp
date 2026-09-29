#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <utility>

#include "crashlog/address.hpp"

namespace crashlog::win64::symbols {
void loadAllSymbols();

/**
 * Parse the address to find the base module name and its offset
 * 0x7FFAB73C00 -> "kernel32.dll", 0x73c00
 */
std::pair<std::string, uint64_t> moduleInfo(uintptr_t address);

/**
 * Parse the address to find the symbol name
 */
std::optional<std::string> symbol(uintptr_t address);

/**
 * Parse the address to find the line information
 */
std::optional<LineInfo> line(uintptr_t address);
};	// namespace crashlog::win64::symbols
