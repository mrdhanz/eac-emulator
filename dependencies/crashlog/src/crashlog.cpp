#include "crashlog/crashlog.hpp"

#ifdef CRASHLOG_WIN64
#include "crashlog/win64/parser.hpp"
#endif

void crashlog::initialize() {
#ifdef CRASHLOG_WIN64
	crashlog::win64::initialize();
#endif
}

crashlog::Exception crashlog::parse(EXCEPTION_POINTERS* ptr) {
#ifdef CRASHLOG_WIN64
	return crashlog::win64::parse(ptr);
#else
	return crashlog::Exception();
#endif
}
