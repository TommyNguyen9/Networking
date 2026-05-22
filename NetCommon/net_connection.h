#pragma once

#include "net_common.h"
#include "net_tsqueue.h"
#include "net_message.h"
//#include "net_client.h"

namespace olc
{
	namespace net
	{
		template<typename T>
		class connection : public std::enable_shared_from_this<connection<T>>
		{

		public:
			// Connect is owned by a server/client
			enum class owner
			{
				server, 
				client
			};


		public: // Specify owner, connect to context, transfer the socket. 
			// Provide reference to incoming message queue.
			connection(owner parent, asio::io_context& asioContext, asio::ip::tcp::socket socket,
				tsqueue<owned_message<T>>& qIn) 
				: m_asioContext(asioContext), m_socket(std::move(socket)), m_qMessagesIn(qIn)
			{
				m_nOwnerType = parent;
			}

			virtual ~connection()
			{
			}

			uint32_t GetID() const // Used system wide
			{
				return id;
			}

		public:
			void ConnectToServer(const asio::ip::tcp::resolver::results_type& endpoints)
			{
				// Only clients can connect to servers
				if (m_nOwnerType == owner::client)
				{

				}
			}


			void Disconnect() {
				if (IsConnected())
					asio::post(m_asioContext, [this]() {m_socket.close(); });
			}

			bool IsConnected() const
			{
				return m_socket.is_open();
			}

		public:
			void ConnectToClient(uint32_t uid = 0)
			{
				if (m_nOwnerType == owner::server)
				{
					if (m_socket.is_open())
					{
						id = uid;
						ReadHeader();
					}
				}
			}

			//void ConnectToServer();
			//bool Disconnect()

		public:
			void Send(const message<T>& msg)
			{
				asio::post(m_asioContext,
					[this, msg]()
					{
						bool bWritingMessage = !m_qMessagesOut.empty();
						m_qMessagesOut.push_back(msg);
						if (!bWritingMessage)
						{
							WriteHeader();
						}
						
					});
			}

		private:
			// ASYNC - Prime context ready to read message header
			void ReadHeader()
			{
				asio::async_read(m_socket, asio::buffer(&m_msgTemporaryIn.header, sizeof(message_header<T>)),
					[this](std::error_code ec, std::size_t length)
					{
						if (!ec)
						{

							if (m_msgTemporaryIn.header.size > 0)
							{
								m_msgTemporaryIn.body.resize(m_msgTemporaryIn.header.size);
								ReadBody();
							}
							else
							{
								AddToIncomingMessageQueue();
							}

						}
						else
						{
							std::cout << "[" << id << "] Read Header Fail.\n";
							m_socket.close(); // Force close the socket.
						}
					});
			}

			// Read message body (ASYNC)

			void ReadBody()
			{
				asio::async_read(m_socket, asio::buffer(m_msgTemporaryIn.body.data(), m_msgTemporaryIn.body.size()),
					[this](std::error_code ec, std::size_t length)
					{
						if (!ec)
						{
							AddToIncomingMessageQueue();
						}
						else
						{
							std::cout << "[" << id << "] Read Body Fail.\n";
							m_socket.close();
						}
					});
			}

			void WriteHeader()
			{
				asio::async_write(m_socket, asio::buffer(&m_qMessagesOut.front().header, sizeof(message_header<T>)),
					[this](std::error_code ec, std::size_t length)
					{
						if (!ec)
						{
							if (m_qMessagesOut.front().body.size() > 0)
							{
								WriteBody();
							}
							else
							{
								m_qMessagesOut.pop_front();

								if (!m_qMessagesOut.empty())
								{
									WriteHeader();
								}
							}
						}
						else
						{
							std::cout << "[" << id << "] Write Header Fail.\n";
							m_socket.close();
						}
					});
			}

			void WriteBody()
			{
				asio::async_write(m_socket, asio::buffer(m_qMessagesOut.front().body.data(), m_qMessagesOut.front().body.size()),
					[this](std::error_code ec, std::size_t length)
					{
						if (!ec)
						{
							m_qMessagesOut.pop_front();

							if (!m_qMessagesOut.empty())
							{
								WriteHeader();
							}
						}
						else
						{
							std::cout << "[" << id << "] Write Body Fail.\n";
							m_socket.close();
						}
					});
			}

			void AddToIncomingMessageQueue()
			{
				if (m_nOwnerType == owner::server)
					m_qMessagesIn.push_back({ this->shared_from_this(), m_msgTemporaryIn });
				else
					m_qMessagesIn.push_back({ nullptr, m_msgTemporaryIn });

				ReadHeader();
			}

		protected:
			// Unique socket to a remote for each connection
			asio::ip::tcp::socket m_socket;

			// Conect is shared with whole asio instance
			asio::io_context& m_asioContext;

			// Hold messages received from remote side of this connection.
			tsqueue<message<T>> m_qMessagesOut;

			// A reference as the "owner" of the connection has to provide a queue:
			tsqueue<owned_message<T>>& m_qMessagesIn;
			message<T> m_msgTemporaryIn;

			// Owner decides how some connections behave

			owner m_nOwnerType = owner::server;

			uint32_t id = 0;


		};
	}
}