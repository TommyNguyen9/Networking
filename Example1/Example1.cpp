#include <chrono>
#include <iostream>

#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif
#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <thread>

namespace asio_ns = asio;
std::vector<char> vBuffer(20 * 1024);

void GrabSomeData(asio::ip::tcp::socket& socket)
{
	socket.async_read_some(asio::buffer(vBuffer.data(), vBuffer.size()),
		[&](std::error_code ec, std::size_t length)
		{
			if (!ec)
			{
				std::cout << "\n\nRead " << length << " bytes\n\n";

				for (int i = 0; i < length; i++)
					std::cout << vBuffer[i];

				GrabSomeData(socket);
			}
		}
	);
}   

int main() {

	try
	{

		asio::error_code ec;

		// Creating a context - platform specific interface.
		asio::io_context context;

		// Keep context alive:
		auto work_guard = asio::make_work_guard(context);

		// Start the context
		std::thread thrContext = std::thread([&]() {
			try
			{
				context.run();

			}
			catch (std::exception& e)
			{
				std::cout << "Context exception: " << e.what() << "\n";
			}

			});

		// Get address of somewhere we want to connect to: - endpoint is an address.
		asio::ip::tcp::endpoint endpoint(asio::ip::make_address("51.38.81.49", ec), 80);

		// Creating a socket:
		asio::ip::tcp::socket socket(context);

		// Tell socket to try & connect
		socket.connect(endpoint, ec);

		if (!ec)
		{
			std::cout << "Connected!" << std::endl;
			//GrabSomeData(socket);
		}
		else
		{
			std::cout << "Failed to connect to address:\n" << ec.message() << std::endl;
		}

		if (!ec)
		{
			GrabSomeData(socket);

			std::string sRequest =
				"GET /index.html HTTP/1.1\r\n"
				"Host: example.com\r\n"
				"Connection: close\r\n\r\n";

			socket.write_some(asio::buffer(sRequest.data(), sRequest.size()), ec);

			if (ec)
			{
				std::cout << "Write failed: " << ec.message() << "\n";
			}

			// Program does something else. Asio handles data transfer in background
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(2000ms);
		}
		else
		{
			std::cout << "Failed to connect: " << ec.message() << "\n";
		}

		if (socket.is_open())
		{
			socket.close();

		}

		work_guard.reset();
		context.stop();

		if (thrContext.joinable())
		{
			thrContext.join();
		}
	}

	catch (const std::exception& e)
	{
		std::cout << "Main exception:" << e.what() << "\n";
	}
		return 0;

}