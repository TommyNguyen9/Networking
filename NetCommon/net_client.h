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
					//// Create connection
					//m_connection = std::make_unique<connection<T>>();

					// resolve ip address into tangiable physical address
					asio::ip::tcp::resolver resolver(m_context);
					asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));

					//m_endpoints = resolver.resolve(host, std::to_string(port));

					// Tell connection object to connect to server
					m_connection->ConnectToServer(endpoints);

					// Start Context Thread:
					thrContext = std::thread([this]() {m_context.run(); });


				}
				catch (std::exception& e)
				{
					std::cerr << "Client Exception: " << e.what() << "\n";
					return false;
				}

				return true;
			}

			// Disconnect from server:
			void Disconnect()
			{
				if (IsConnected())
				{
					m_connection->Disconnect();
				}

				m_context.stop();
				if (thrContext.joinable())
					thrContext.join();

				// Destroy connection object:
				m_connection.release();
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

		public:
			// Send message to server:
			void Send(const message<T>& msg)
			{
				if (IsConnected())
					m_connection->Send(msg);
			}

			// Retrieve queue of messages from server
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