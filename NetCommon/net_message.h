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

			// Push POD like data into the message buffer:
			template<typename DataType> 
			friend message<T>& operator << (message<T>& msg, const DataType& data)
			{
				// Check type of data being pushed is copyable:
				static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed into vector!");

				// Cache current size of vector:
				size_t i = msg.body.size();

				// Resize body vector:
				msg.body.resize(msg.body.size() + sizeof(DataType));

				// Copy data into newly allocated vector space
				std::memcpy(msg.body.data() + i, &data, sizeof(DataType));

				// Recalculate message size:
				msg.header.size = msg.size();

				// Return target message so it can be chained
				return msg;
			};

			// Forward declare connection
			template <typename T>
			class connection;

			template <typename T> 
			struct owned_message
			{
				std::shared_ptr<connection<T>> remote = nullptr;
				message<T> msg;

				// String maker:

				friend std::ostream& operator<<(std::ostream& os, const owned_message<T>& msg)
				{
					os << msg.msg;
					return os;
				}
			};

			template<typename DataType>
			friend message<T>& operator >> (message<T>& msg, DataType& data)
			{

				static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed into vector!");

				//Cache location to end of vector where pulled data starts:
				size_t i = msg.body.size() - sizeof(DataType);

				//Physically copy data from vector in user variable:
				std::memcpy(&data, msg.body.data() + i, sizeof(DataType));

				// Reduce size of vector:

				msg.body.resize(i);

				return msg;
			}
			
		}; 
	}
}
