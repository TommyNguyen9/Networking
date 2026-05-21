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
			bool ConnectToServer();
			bool Disconnect();
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
						//ReadHeader();
					}
				}
			}

		public:
			bool Send(const message<T>& msg);

		protected:
			// Unique socket to a remote for each connection
			asio::ip::tcp::socket m_socket;

			// Conect is shared with whole asio instance
			asio::io_context& m_asioContext;

			// Hold messages received from remote side of this connection.
			tsqueue<message<T>> m_qMessagesOut;

			// A reference as the "owner" of the connection has to provide a queue:
			tsqueue<owned_message<T>>& m_qMessagesIn;

			// Owner decides how some connections behave

			owner m_nOwnerType = owner::server;

			uint32_t id = 0;


		};
	}
}