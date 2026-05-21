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

							// New object newconn:
							std::shared_ptr<connection<T>> newconn =
								std::make_shared<connection<T>>(connection<T>::owner::server,
									m_asioContext, std::move(socket), m_qMessagesIn);

							// Give user server a chance to deny connection
							if (OnClientConnect(newconn))
							{

								// Connection allowed. Add to container of new connections
								m_deqConnections.push_back(std::move(newconn));

								m_deqConnections.back()->ConnectToClient(nIDCounter++);

								std::cout << "[" << m_deqConnections.back()->GetID() << "] Connection Approved\n";

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

						// Prime asio context with more work:
						WaitForClientConnection();
					});
				

			}

			// Send message to specific client
			void MessageClient(std::shared_ptr<connection<T>> client, const message<T>& msg)
			{
				if (client && client->IsConnected())
				{
					client->Send(msg);
				}
				else
				{
					OnClientDisconnect(client);
					client.reset();
					m_deqConnections.erase(
						std::remove(m_deqConnections.begin(), m_deqConnections.end(), client), m_deqConnections.end());
				}
			}

			// Send message to all clients
			void MessageAllClients(const message<T>& msg, std::shared_ptr<connection<T>> pIgnoreClient = nullptr)
			{
				bool bInvalidClientExists = false;

				for (auto& client : m_deqConnections)
				{
					// Check client is connected
					if (client && client->IsConnected())
					{
						if (client != pIgnoreClient)
							client->Send(msg);
					}
					else
					{
						// Client couldnt be contacted
						OnClientDisconnect(client);
						client.reset();
						bInvalidClientExists = true;
					}
				}

				if (bInvalidClientExists)
					m_deqConnections.erase(
						std::remove(m_deqConnections.begin(), m_deqConnections.end(), nullptr), m_deqConnections.end());
					)

			}

			void Update(size_t nMaxMessages = -1)
			{
				size_t nMessageCount = 0;
				while (nMessageCount < nMaxMessages && !m_qMessageIn.empty())
				{
					// Grab front message
					auto msg = m_qMessagesIn.pop_front();

					// Pass to message handler
					OnMessage(msg.remote, msg.msg);

					nMessageCount++;
				}
			}

		protected:
			// Called when client connects, ve3
			// to the connection by returning false
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

			// Container of active validated connections
			std::deque<std::shared_ptr<connection<T>>> m_deqConnections;

			// Order of declaration is important
			asio::io_context m_asioContext;
			std::thread m_threadContext;

			asio::ip::tcp::acceptor m_asioAcceptor;

			// Clients identified in wider system via ID
			uint32_t nIDCounter = 10000;

		};
	}
}