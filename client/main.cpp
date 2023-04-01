//
// blocking_udp_echo_client.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2021 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <asio.hpp>
#include <packets.hpp>

using asio::ip::udp;

enum { max_length = sizeof(Packet) };

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 3)
    {
      std::cerr << "Usage: blocking_udp_echo_client <host> <port>\n";
      return 1;
    }

    asio::io_context io_context;

    udp::socket s(io_context, udp::endpoint(udp::v4(), 0));

    udp::resolver resolver(io_context);
    udp::resolver::results_type endpoints =
      resolver.resolve(udp::v4(), argv[1], argv[2]);

    std::cout << "Enter message: ";
    char requestTxt[max_length];
    std::cin.getline(requestTxt, max_length);
    size_t request_length = std::strlen(requestTxt);
    Packet request;
    request.type = PacketType::PRINT;
    std::memcpy(request.data, requestTxt, request_length);
    char requestBuffer[max_length];
    requestBuffer[0] = static_cast<char>(request.type);
    std::memcpy(requestBuffer + 1, request.data, sizeof(request.data));
    s.send_to(asio::buffer(requestBuffer, request_length), *endpoints.begin());

    char reply[max_length];
    udp::endpoint sender_endpoint;
    size_t reply_length = s.receive_from(
        asio::buffer(reply, max_length), sender_endpoint);
    if (reply[0] == static_cast<char>(PacketType::PRINT_ACK)) {
        std::cout << "Reply is: ";
        std::cout.write(reply + 1, reply_length - 1);
        std::cout << "\n";
    }
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}