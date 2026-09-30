#pragma once
#include <plog/Log.h>

#include "nullable_string.h"

class read_stream {
public:
    const char* data;
    size_t size;
    size_t position;

    read_stream(const void* data, const size_t size)
        : data(static_cast<const char*>(data)), size(size), position(0) {}

    [[nodiscard]] size_t bytes_remaining() const {
        return (position <= size) ? (size - position) : 0;
    }

    const char* read(size_t rsize) {
        if (position + rsize > size) {
            PLOGF.printf("read position is oversized");
            return nullptr;
        }
        const char* ret = data + position;
        position += rsize;
        return ret;
    }

    template <typename T>
    T read_as() {
        const char* ptr = read(sizeof(T));
        if (ptr == nullptr) {
            return T{};
        }
        T val;
        memcpy(&val, ptr, sizeof(T));
        return val;
    }

	[[nodiscard]] nullable_string read_string() {
		auto is_valid = read_as<char>();
    	if (!is_valid) {
    		return {};
    	}

		auto length = read_as<int>();
    	if (length <= 0) {
    		return nullable_string("");
    	}
    	const char* start = read(static_cast<size_t>(length));
    	if (start == nullptr) {
    		return {};
    	}

    	return nullable_string(std::string(start, static_cast<size_t>(length)));
    }

    void close() const {
    }
};
