#pragma once

#include <plog/Severity.h>

#include <filesystem>
#include <optional>
#include <string>

class utils {
public:
	static void create_console();

	static void init_logger(bool use_colored, plog::Severity severity, const std::optional<std::string>& log_file_path = std::nullopt);

	static void sleep(unsigned int ms);

	static std::optional<std::string> read_file(const std::string& path);

	static void write_file(const std::string& path, const std::string& content);

	static std::filesystem::path get_current_process_path();

	static std::string get_current_process_name();

	static bool is_game_process();

	static std::optional<std::filesystem::path> detect_eos_sdk_path(const std::string& module_name = "EOSSDK-Win64-Shipping.dll");

	static std::optional<std::filesystem::path> find_module_path(const std::string& module_name);
};
