#pragma once

#include "net_common.h"
#include "net_tsqueue.h"
#include "net_message.h"
#include "net_connection.h"

namespace olc
{
	namespace net
	{
		template<typename T>
		class server_interface
		{
		public:
			server_interface(uint16_t port)
				: m_asioAcceptor(m_asioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
			{

			}

			virtual ~server_interface()
			{
				Stop();
			}

			bool Start()
			{
				try
				{
					WaitForClientConnection();

					m_threadContext = std::thread([this]() { m_asioContext.run(); });

				}
				catch (std::exception& e)
				{
					// Something that has prohibited the server from listening
					std::cerr << "[SERVER] Exception: " << e.what() << "\n";
					return false;
				}

				std::cout << "[SERVER] Started!\n";
				return true;

			}

			void Stop()
			{
				// Request context to close
				m_asioContext.stop()

				// Tidy up context thread
				if (m_threadContext.joinable() m_threadContext.join();)

				// Inform a person
				std::cout << "[SERVER] Stopped!\n";

			}

			// ASYNC - Instruct asio to wait for connection
			void WaitForClientConnect()
			{
				m_asioAcceptor.async_accept(
					[this](std::error_code ec, asio::ip::tcp::socket socket)
					{
						if (!ec)
						{
							std::cout << "[SERVER] New Connection: " << socket.remote_endpoint() << "\n";

							std::shared_ptr<connection<T>> newconn =
								std::make_shared<connection<T>>(connection<T>::owner::server,
									m_asioContext, std::move(socket), m_qMessagesIn);

							// Give user server a chance to deny connection
							if (OnClientConnect(newconn))
							{


							}
							else
							{
								std::cout << "[-----] Connection Denied\n";
							}

						}
						else
						{
							// Error occured during acceptance
							std::cout << "[SERVER] New Connection Error: " << ec.message() << "\n";
						}

						WaitForClientConnection();
					});
				

			}

			// Send message to specific client
			void MessageClient(std::shared_ptr<connection<T>> client, const message<T>& msg)
			{

			}

			// Send message to all clients
			void MessageAllClients(const message<T>& msg, std::shared_ptr<connection<T>> pIgnoreClient = nullptr)
			{

			}

		protected:
			// Called when client connects, veto the connection by returning false
			virtual bool OnClientConnect(std::shared_ptr<connection<T>> client)
			{
				return false;
			}

			// Called when client has disconnected:
			virtual void OnClientDisconnect(std::shared_ptr<connection<T>> client)
			{

			}

		protected:
			// Thread safe queue for incoming message packets
			tsqueue<owned_message<T>> m_qMessagesIn;

			// Order of declaration is important
			asio::io_context m_asioContext;
			std::thread m_threadContext;

			asio::ip::tcp::acceptor m_asioAcceptor;

			// Clients identified in wider system via ID
			uint32_t nIDCounter = 10000;

		};
	}
}