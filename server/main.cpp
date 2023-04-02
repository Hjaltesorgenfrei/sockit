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
  server(asio::io_context &io_context, short port)
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
            Packet* packet = (Packet *)_recieveData;
            handle_packet(packet);
          }
          do_receive();
        });
  }

  void handle_packet(Packet *packet)
  {
    std::cout << "Received packet: " << packet_type_string(packet->type) << std::endl;
    switch (packet->type)
    {
      case PACKET_TYPE_CONNECTION_REQUEST:
      {
        PacketConnectionRequest *request = (PacketConnectionRequest *)packet;
        std::cout << "Connection Request: " << request->client_guid << std::endl;
        std::cout << "Connection Sequence: " << request->connect_sequence << std::endl;
        PacketConnectionAccepted response;
        response.client_guid = request->client_guid;
        response.connect_sequence = request->connect_sequence + 1;
        do_send(sizeof(PacketConnectionAccepted), response);
        break;
      }
    }
  }

  void do_send(std::size_t length, Packet &packet)
  {
    std::shared_ptr<std::vector<char>> data = std::make_shared<std::vector<char>>(length);
    memcpy(data->data(), &packet, length);
    _socket.async_send_to(
        asio::buffer(data->data(), length), _senderEndpoint,
        [data](std::error_code /*ec*/, std::size_t /*bytes_sent*/)
        {
        });
  }

private:
  udp::socket _socket;
  udp::endpoint _senderEndpoint;
  enum
  {
    max_length = 1024
  };
  char _recieveData[max_length];
  char _sendData[max_length];
};

int main(int argc, char *argv[])
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
  catch (std::exception &e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  std::cout << "Server Exited" << std::endl;
  return 0;
}