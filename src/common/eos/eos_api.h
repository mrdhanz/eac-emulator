#pragma once

#include <cstdint>

#define EOS_Success 0
#define EOS_InvalidParameters 1
#define EOS_LimitExceeded 3
#define EOS_NS_Online 2

#define EOS_EResult int
#define EOS_ENetworkStatus int
#define EOS_Bool int32_t
#define EOS_TRUE 1
#define EOS_FALSE 0

typedef uintptr_t EOS_HPlatform;
typedef uintptr_t EOS_HConnect;
typedef uintptr_t EOS_HAntiCheatClient;
typedef uintptr_t EOS_HAchievements;
typedef uintptr_t EOS_NotificationId;
typedef uintptr_t EOS_ProductUserId;
typedef uintptr_t EOS_ContinuanceToken;
typedef uintptr_t EOS_ELoginStatus;
typedef uintptr_t EOS_EAntiCheatClientMode;

#define EOS_EXTERN_C extern "C"
#define EOS_CALL __stdcall
#define EOS_DECLARE_FUNC(ret_type) EOS_EXTERN_C ret_type EOS_CALL
#define EOS_DECLARE_CALLBACK(CallbackName, ...) EOS_EXTERN_C typedef void(EOS_CALL * CallbackName)(__VA_ARGS__)
