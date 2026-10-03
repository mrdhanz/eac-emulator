#include "http_server.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <common/api/session_factory.h>
#include <hv/HttpServer.h>

#include <plog/Log.h>
#include <json.hpp>
#include <thread>

#include "../api/api_handler_registry.h"

void http_server::init() {
	HttpService router;
	router.POST("/task", [](HttpRequest* req, HttpResponse* resp) {
		try {
			if (req->body.empty()) {
				return resp->String(nlohmann::json({{"status", "error"}, {"message", "Empty request body"}}).dump());
			}

			nlohmann::json json = nlohmann::json::parse(req->body);
			if (!json.contains("id") || !json["id"].is_number()) {
				PLOGE.printf("Request missing numeric 'id' field: %s", req->body.c_str());
				return resp->String(nlohmann::json({{"status", "error"}, {"message", "Missing numeric 'id' field"}}).dump());
			}

			auto request_id = json["id"].get<unsigned char>();
			auto request = session_factory::create_request(request_id);
			if (request == nullptr) {
				PLOGE.printf("Invalid request id: %d", static_cast<int>(request_id));
				return resp->String(nlohmann::json({{"status", "error"}, {"message", "Invalid request id"}}).dump());
			}
			request->deserialize(json);

			auto handler = api_handler_registry::get_handler_by_id(request->get_id());
			if (handler == nullptr) {
				PLOGE.printf("No handler registered for request id: %d", static_cast<int>(request->get_id()));
				return resp->String(nlohmann::json({{"status", "error"}, {"message", "No handler registered"}}).dump());
			}
			auto response = handler(request);
			if (response == nullptr) {
				PLOGE.printf("Handler returned null response for request id: %d", static_cast<int>(request->get_id()));
				return resp->String(nlohmann::json({{"status", "error"}, {"message", "Handler returned null"}}).dump());
			}

			nlohmann::json output_json;
			response->serialize(output_json);

			return resp->String(output_json.dump());
		} catch (const std::exception& e) {
			PLOGE.printf("[HTTP] Exception in /task handler: %s", e.what());
			return resp->String(nlohmann::json({{"status", "error"}, {"message", e.what()}}).dump());
		}
	});

	hv::HttpServer server(&router);
	server.setPort(port);
	server.setThreadNum(4);
	server.run();
}

void http_server::run(int port) {
	http_server::port = port;
	std::thread(init).detach();
}
