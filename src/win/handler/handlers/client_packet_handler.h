#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "../handler_registry.h"
#include "../packet_handler.h"
#include "common/eos/eos_anticheat_types.h"
#include "common/protocol/packet.h"
#include "win/sock/packet_sender.h"

struct notify_message_to_server_callback {
	EOS_AntiCheatClient_AddNotifyMessageToServerOptions options;
	void* client_data;
	EOS_AntiCheatClient_OnMessageToServerCallback notification_fn;
};

class client_packet_handler : public packet_handler {
	std::weak_ptr<packet_sender> sender;
	handler_registry registry;

	static inline std::atomic<EOS_NotificationId> next_notification_id{ 1 };
	static inline std::mutex notify_message_to_server_callbacks_mutex;
	static inline std::unordered_map<EOS_NotificationId, notify_message_to_server_callback> notify_message_to_server_callbacks;

public:
	client_packet_handler(std::weak_ptr<packet_sender> sender);

	void on_connected() override;

	handler_registry& get_handler_registry() override;

	static EOS_NotificationId add_notify_message_to_server(notify_message_to_server_callback callback);

	static void remove_notify_message_to_server(EOS_NotificationId notification_id);

	static void replay_notify_message_to_server_bindings();

private:
	void handle_handshake(std::shared_ptr<packet> packet);

	void handle_notify_msg_to_server(std::shared_ptr<packet> packet);

	void send_packet(std::shared_ptr<packet> packet);
};
