#pragma once

#ifdef _WIN64
#define CRASHLOG_WIN64
#endif

#if defined(CRASHLOG_WIN64) && !defined(CRASHLOG_NO_WINDOWS_H)
#include <Windows.h>  // IWYU pragma: keep
#endif

#include "crashlog/exception.hpp"  // IWYU pragma: keep

namespace crashlog {
void initialize();

#ifdef CRASHLOG_WIN64
crashlog::Exception parse(EXCEPTION_POINTERS* ptr);
#endif
};	// namespace crashlog
