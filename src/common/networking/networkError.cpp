#include "networkError.h"

namespace net
{
	void netErr(const asio::error_code& ec)
	{
		if (!ec)
			return;

		std::cerr << "NetworkError: " << ec.message();
		throw ec;
	}
}