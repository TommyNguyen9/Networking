#pragma once

#include "net_common.h"
#include "net_message.h"
#include "net_tsqueue.h"
#include "net_connection.h"

namespace olc
{
	namespace net
	{
		template <typename T>
		class client_interface
		{
			client_interface() : m_socket(m_context)
			{
				// Initisalise socket with io context
			}

			virtual ~client_interface()
			{
				// If client is destroyed, always try and disconnect from server
				Disconnect();
			}

		public:
			// Connect to server with ip address & port
			bool Connect(const std::string& host, const uint16_t port)
			{
				try
				{

				}
				catch (std::exception& e)
				{
					std:cerr << "Client Exception: " << e.what() << "\n";
					return false;
				}

				return true;
			}

			// Disconnect from server:
			void Disconnect()
			{

			}

			// Check if client is still connected to the server:
			bool IsConnected()
			{
				if (m_connection)
					return m_connection->IsConnected();
				else
					return false;
			}

			// Retrieve queue messages from server:
			tsqueue<owned_message<T>>& Incoming()
			{
				return m_qMessagesIn;
			}



		protected:
			// Context handles data transfer
			asio::io_context m_context;
			std::thread thrContext;

			// Hardware socket:
			asio::ip::tcp::socket m_socket;

			// Single instance of a "connection" object
			std::unique_ptr<connection<T>> m_connection;

		private:
			// Thread safe queue of incoming messages from server
			tsqueue<owned_message<T>> m_qMessagesIn;
		};
	}
}