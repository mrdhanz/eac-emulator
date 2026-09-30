#pragma once
#include "buffer.h"

#include <vector>
#include <plog/Log.h>

#include "nullable_string.h"

class write_stream {
public:
    std::vector<char> buffer;

    const char* data() const noexcept {
        return buffer.data();
    }

    size_t size() const noexcept {
        return buffer.size();
    }

    void write(const void* data, size_t size) {
        if (data != nullptr && size > 0) {
            const auto* bytes = static_cast<const char*>(data);
            buffer.insert(buffer.end(), bytes, bytes + size);
        }
    }

    template <typename T>
    void write_as(T val) {
        write(&val, sizeof(val));
    }

	void write_string(const nullable_string& string) {
    	if (string == nullptr) {
    		write_as<char>(0); // is valid
    		return;
    	}

    	write_as<char>(1); // is valid
    	const int len = static_cast<int>(string.length());
    	write_as<int>(len);
    	if (len == 0) {
    		return;
    	}

    	write(string.c_str(), static_cast<size_t>(len));
    }

    ::buffer as_buffer() const {
        if (buffer.empty()) {
            return {
                nullptr,
                0
            };
        }

        void* copiedBuffer = malloc(buffer.size());
        if (copiedBuffer == nullptr) {
            PLOGF.printf("malloc failed");
        }

        memcpy(copiedBuffer, buffer.data(), buffer.size());

        return {
            copiedBuffer,
            buffer.size()
        };
    }
};
