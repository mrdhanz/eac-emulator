#include "bootstrapper.h"

#include <MinHook.h>
#include <common/constants.h>
#include <common/utils.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Log.h>
#include <shlwapi.h>

#include "api/api_handler_registry.h"
#include "eos/eos.h"
#include "eos/eos_platform.h"
#include "handlers/handler_registry.h"
#include "servers/http_server.h"
#include "servers/websocket_server.h"

int bootstrapper::Dummy_WinMain() {
	handler_registry::init();
	api_handler_registry::init();

	websocket_server::launch(tcp_port);
	http_server::run(http_port);

	while (true) {
		if (!websocket_server::has_connection()) {
			websocket_server::wait_for_connection();
			PLOGI.printf("Client reconnected. Resuming EOS loop");
		}

		Sleep(1000 / 30);  // emulate 30 fps

		websocket_server::tick();
		if (eos::is_eos_initialized() && eos_platform::is_platform_created()) {
			eos_platform::tick();
		}
	}
}

void bootstrapper::hook_winmain() {
	MH_Initialize();

	HMODULE unityPlayerHandle = GetModuleHandleA(UNITY_PLAYER_MODULE_NAME);
	if (unityPlayerHandle == nullptr) {
		unityPlayerHandle = LoadLibraryA(UNITY_PLAYER_MODULE_NAME);
	}
	if (unityPlayerHandle == nullptr) {
		PLOGF.printf("Failed to find or load %s", UNITY_PLAYER_MODULE_NAME);
		return;
	}

	void* unityMain = GetProcAddress(unityPlayerHandle, "UnityMain");
	if (unityMain == nullptr) {
		PLOGF.printf("Failed to find UnityMain in %s", UNITY_PLAYER_MODULE_NAME);
		return;
	}
	PLOGD.printf("UnityPlayer.dll WinMain=%llx", unityMain);

	void* originalFunc;
	if (MH_CreateHook(unityMain, &Dummy_WinMain, &originalFunc) != MH_OK || MH_EnableHook(unityMain) != MH_OK) {
		PLOGF.printf("Failed to create hook of UnityMain");
		return;
	}
	PLOGI.printf("Created WinMain function hook");
	PLOGI.printf("Waiting for the main process...");
}

void bootstrapper::main(int tcp_port, int http_port) {
	bootstrapper::tcp_port = tcp_port;
	bootstrapper::http_port = http_port;

	auto eos_sdk_path = utils::detect_eos_sdk_path();
	if (!eos_sdk_path.has_value()) {
		PLOGF.printf("Failed to locate %s!", EOS_SDK_MODULE_NAME);
		return;
	}
	PLOGI.printf("Detected EOS SDK module at: %s", eos_sdk_path->string().c_str());

	HMODULE eos_module = LoadLibraryW(eos_sdk_path->c_str());
	if (eos_module == nullptr) {
		eos_module = LoadLibraryExW(eos_sdk_path->c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
	}
	if (eos_module == nullptr) {
		PLOGF.printf("Failed to perform LoadLibrary on %s!", eos_sdk_path->string().c_str());
		return;
	}
	PLOGD.printf("EOSSDK module loaded successfully");

	hook_winmain();
}
