#pragma once
#include <asio/asio.hpp>
#include <iostream>

namespace net
{
	void netErr(const asio::error_code& ec);
}
