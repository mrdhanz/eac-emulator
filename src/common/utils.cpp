#include "utils.h"

#include <Windows.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Appenders/RollingFileAppender.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Init.h>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <memory>
#include <thread>
#include <vector>

namespace {
	constexpr size_t log_file_max_size = 5 * 1024 * 1024;
	constexpr int log_file_generation_count = 3;

	void reset_log_files(const std::string& log_file_path) {
		const std::filesystem::path path(log_file_path);
		std::error_code error_code;
		std::filesystem::remove(path, error_code);

		const auto stem = path.stem().string();
		const auto extension = path.extension().string();
		for (int file_index = 1; file_index < log_file_generation_count; ++file_index) {
			const auto rotated_file_name = stem + "." + std::to_string(file_index) + extension;
			std::filesystem::remove(path.parent_path() / rotated_file_name, error_code);
		}
	}
}

void utils::create_console() {
	AllocConsole();

	FILE* newStdout;
	FILE* newStdin;
	freopen_s(&newStdout, "CONOUT$", "w", stdout);
	freopen_s(&newStdin, "CONIN$", "r", stdin);

	SetConsoleTitleA("EAC Emulator");
}

void utils::init_logger(bool use_colored, plog::Severity severity, const std::optional<std::string>& log_file_path) {
	static plog::ConsoleAppender<plog::TxtFormatter> consoleAppender;
	static plog::ColorConsoleAppender<plog::TxtFormatter> coloredConsoleAppender;
	static std::unique_ptr<plog::RollingFileAppender<plog::TxtFormatter>> fileAppender;
	static std::string currentLogFilePath;
	static bool fileAppenderAttached = false;

	plog::IAppender* appender = use_colored ? &coloredConsoleAppender : &consoleAppender;
	auto& logger = plog::init(severity, appender);
	if (!log_file_path.has_value()) {
		return;
	}

	if (fileAppender == nullptr || currentLogFilePath != log_file_path.value()) {
		currentLogFilePath = log_file_path.value();
		reset_log_files(currentLogFilePath);
		fileAppender = std::make_unique<plog::RollingFileAppender<plog::TxtFormatter>>(currentLogFilePath.c_str(), log_file_max_size, log_file_generation_count);
		fileAppenderAttached = false;
	}

	if (!fileAppenderAttached) {
		logger.addAppender(fileAppender.get());
		fileAppenderAttached = true;
	}
}

void utils::sleep(unsigned int ms) {
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
std::optional<std::string> utils::read_file(const std::string& path) {
	std::ifstream stream = std::ifstream(path);
	if (!stream) {
		stream.close();
		return std::nullopt;
	}

	std::string content = std::string(std::istreambuf_iterator(stream), std::istreambuf_iterator<char>());
	stream.close();
	return std::optional{content};
}
void utils::write_file(const std::string& path, const std::string& content) {
	std::ofstream stream = std::ofstream(path);
	stream << content;
	stream.close();
}

std::filesystem::path utils::get_current_process_path() {
	char buffer[MAX_PATH];
	DWORD len = GetModuleFileNameA(NULL, buffer, MAX_PATH);
	if (len == 0) {
		return std::filesystem::path();
	}
	if (len == MAX_PATH) {
		std::vector<char> long_buffer(32768);
		len = GetModuleFileNameA(NULL, long_buffer.data(), static_cast<DWORD>(long_buffer.size()));
		if (len > 0) {
			return std::filesystem::path(long_buffer.data());
		}
	}
	return std::filesystem::path(buffer);
}

std::string utils::get_current_process_name() {
	const auto path = get_current_process_path();
	return path.filename().string();
}

bool utils::is_game_process() {
	const auto exe_path = get_current_process_path();
	if (exe_path.empty()) {
		return false;
	}

	const auto exe_name = exe_path.filename().string();
	std::string lower_name = exe_name;
	std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), [](unsigned char c) {
		return static_cast<char>(std::tolower(c));
	});

	// Ignore known crash handlers, setup utilities, and anti-cheat launchers
	if (lower_name.find("crashhandler") != std::string::npos ||
		lower_name.find("crashpad") != std::string::npos ||
		lower_name.find("easyanticheat") != std::string::npos ||
		lower_name.find("start_protected_game") != std::string::npos ||
		lower_name.find("eac") != std::string::npos ||
		lower_name.find("eos") != std::string::npos ||
		lower_name.find("launch") != std::string::npos ||
		lower_name.find("install") != std::string::npos) {
		return false;
	}

	const auto exe_dir = exe_path.parent_path();
	const auto exe_stem = exe_path.stem().string();

	std::error_code ec;

	// 1. Direct Unity standalone match: <GameName>_Data directory exists next to <GameName>.exe
	const auto matching_data_dir = exe_dir / (exe_stem + "_Data");
	if (std::filesystem::is_directory(matching_data_dir, ec)) {
		return true;
	}

	// 2. Unity runtime loaded in the current process
	if (GetModuleHandleA("UnityPlayer.dll") != nullptr) {
		return true;
	}

	// 3. UnityPlayer.dll exists in the executable's directory
	if (std::filesystem::exists(exe_dir / "UnityPlayer.dll", ec)) {
		return true;
	}

	// 4. Any other *_Data directory exists in the executable's directory (e.g. if the executable was renamed)
	for (const auto& entry : std::filesystem::directory_iterator(exe_dir, ec)) {
		if (entry.is_directory(ec)) {
			const auto dirname = entry.path().filename().string();
			if (dirname.size() > 5 && dirname.compare(dirname.size() - 5, 5, "_Data") == 0) {
				return true;
			}
		}
	}

	// 5. Check if the EOS SDK module exists in or under the directory
	if (detect_eos_sdk_path().has_value()) {
		return true;
	}

	return true;
}

std::optional<std::filesystem::path> utils::detect_eos_sdk_path(const std::string& module_name) {
	return find_module_path(module_name);
}

std::optional<std::filesystem::path> utils::find_module_path(const std::string& module_name) {
	const auto exe_path = get_current_process_path();
	if (exe_path.empty()) {
		return std::nullopt;
	}

	const auto exe_dir = exe_path.parent_path();
	const auto exe_stem = exe_path.stem().string();
	std::error_code ec;

	auto is_target_file = [&](const std::filesystem::path& path) {
		if (!std::filesystem::is_regular_file(path, ec)) {
			return false;
		}
		const auto filename = path.filename().string();
		return _stricmp(filename.c_str(), module_name.c_str()) == 0;
	};

	// 1. Check <exe_dir>/<exe_stem>_Data/Plugins/x86_64/<module_name>
	auto candidate = exe_dir / (exe_stem + "_Data") / "Plugins" / "x86_64" / module_name;
	if (is_target_file(candidate)) {
		return candidate;
	}

	// 2. Check <exe_dir>/<exe_stem>_Data/Plugins/<module_name>
	candidate = exe_dir / (exe_stem + "_Data") / "Plugins" / module_name;
	if (is_target_file(candidate)) {
		return candidate;
	}

	// 3. Scan any directory ending with _Data for Plugins/**/<module_name>
	for (const auto& entry : std::filesystem::directory_iterator(exe_dir, ec)) {
		if (entry.is_directory(ec)) {
			const auto dirname = entry.path().filename().string();
			if (dirname.size() > 5 && dirname.compare(dirname.size() - 5, 5, "_Data") == 0) {
				const auto plugins_dir = entry.path() / "Plugins";
				if (std::filesystem::is_directory(plugins_dir, ec)) {
					auto p1 = plugins_dir / "x86_64" / module_name;
					if (is_target_file(p1)) {
						return p1;
					}
					auto p2 = plugins_dir / module_name;
					if (is_target_file(p2)) {
						return p2;
					}
				}
			}
		}
	}

	// 4. Check <exe_dir>/<module_name>
	candidate = exe_dir / module_name;
	if (is_target_file(candidate)) {
		return candidate;
	}

	// 5. Check <exe_dir>/Plugins/x86_64/<module_name> or <exe_dir>/Plugins/<module_name>
	candidate = exe_dir / "Plugins" / "x86_64" / module_name;
	if (is_target_file(candidate)) {
		return candidate;
	}
	candidate = exe_dir / "Plugins" / module_name;
	if (is_target_file(candidate)) {
		return candidate;
	}

	// 6. Deep recursive search under exe_dir (max depth 4)
	try {
		for (auto it = std::filesystem::recursive_directory_iterator(
				exe_dir,
				std::filesystem::directory_options::skip_permission_denied,
				ec);
			it != std::filesystem::recursive_directory_iterator();
			it.increment(ec)) {
			if (ec) {
				break;
			}
			if (it.depth() > 4) {
				it.pop();
				continue;
			}
			if (is_target_file(it->path())) {
				return it->path();
			}
		}
	} catch (...) {
		// Ignore any filesystem iteration errors
	}

	return std::nullopt;
}

