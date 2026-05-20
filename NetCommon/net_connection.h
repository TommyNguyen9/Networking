#pragma once

#include "net_common.h"
#include "net_tsqueue.h"
#include "net_message.h"

namespace olc
{
	namespace net
	{
		template<typename T>
		class connection : public std::enable_shared_from_this<connection<T>>
		{
		public:
			connection()
			{
			}

			virtual ~connection()
			{
			}

		public:
			bool ConnectToServer();
			bool Disconnect();
			bool IsConnected() const;

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
			tsqueue<owned_message>& m_qMessagesIn;


		};
	}
}