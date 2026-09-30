#pragma once

#include <cstdint>
#include <optional>
#include <string>

#if defined(__x86_64__) || defined(__ppc64__) || defined(_M_X64) || defined(_M_AMD64) || defined(_WIN64)
#define CRASHLOG_64BIT_ADDR
#else
#define CRASHLOG_32BIT_ADDR
#endif

namespace crashlog {
#ifdef CRASHLOG_64BIT_ADDR
typedef uint64_t Address;
#else
typedef uint32_t Address;
#endif

struct LineInfo {
	std::string fileName;
	uint32_t lineNo;
};

struct AddressMetadata {
    Address address;
    std::string moduleName;
    uint64_t moduleOffset;
	std::optional<std::string> symbol;
	std::optional<LineInfo> line;
};

std::string addressToString(AddressMetadata addr);
};	// namespace crashlog
