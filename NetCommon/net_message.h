#pragma once
#include "net_common.h"

namespace olc
{
	namespace net
	{
		// Message Header sent at start of all messages.
		// Template allows for enum class usage, ensuring that messages are valid at
		// compile time.

		template <typename T>
		struct message_header
		{
			T id{};
			uint32_t size = 0;
		};

		template <typename T>
		struct message
		{
			message_header<T> header{};
			std::vector<uint8_t> body;

			// return size of entire message packet in bytes:
			size_t size() const
			{
				return sizeof(message_header<T>) + body.size();
			}

			// Override for std::cout compatibility:
			friend std::ostream& operator << (std::ostream& os, const message<T>& msg)
			{
				os << "ID:" << int(msg.header.id) << " Size:" << msg.header.size;
				return os;
			}

		};
	}
}
