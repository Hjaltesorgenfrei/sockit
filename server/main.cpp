//
// async_udp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2021 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <iostream>
#include <asio.hpp>
#include <packets.hpp>

using asio::ip::udp;

class server
{
public:
  server(asio::io_context& io_context, short port)
    : _socket(io_context, udp::endpoint(udp::v4(), port))
  {
    do_receive();
  }

  void do_receive()
  {
    _socket.async_receive_from(
        asio::buffer(_recieveData, max_length), _senderEndpoint,
        [this](std::error_code ec, std::size_t bytes_recvd)
        {
          if (!ec && bytes_recvd > 0)
          {
            if (_recieveData[0] == static_cast<char>(PacketType::PRINT)) {
                std::cout << "Received: " << _recieveData << std::endl;
                _recieveData[0] = static_cast<char>(PacketType::PRINT_ACK);
            }
            Packet recieved = { PacketType::PRINT, bytes_recvd - 1, {0} };
            char answer[] = "Hello from server! You sent: ";
            Packet response = { PacketType::PRINT_ACK, sizeof(answer) + , {0} };
            std::memcpy(response.data, answer, sizeof(answer));
            _sendData[0] = static_cast<char>(response.type);
            // Add size
            std::memcpy(_sendData + offsetof(Packet, length), &response.length, sizeof(response.length));
            std::memcpy(_sendData + offsetof(Packet, data), response.data, sizeof(response.data));
            // Add orginal data to response
            std::memcpy(_sendData + offsetof(Packet, data) + sizeof(response.data), _recieveData + 1, bytes_recvd - 1);
            do_send(sizeof(Packet));
          }
          else
          {
            do_receive();
          }
        });
  }

  void do_send(std::size_t length)
  {
    _socket.async_send_to(
        asio::buffer(_sendData, length), _senderEndpoint,
        [this](std::error_code /*ec*/, std::size_t /*bytes_sent*/)
        {
          do_receive();
        });
  }

private:
  udp::socket _socket;
  udp::endpoint _senderEndpoint;
  enum { max_length = sizeof(Packet) };
  char _recieveData[max_length];
  char _sendData[max_length];
};

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: async_udp_echo_server <port>\n";
      return 1;
    }

    asio::io_context io_context;

    server s(io_context, std::atoi(argv[1]));

    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}