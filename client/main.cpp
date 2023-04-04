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

enum
{
  max_length = 1024
};

class client
{
public:
  client(std::string host, std::string service, asio::io_context &io_context) : _socket(io_context, udp::endpoint(udp::v4(), 0))
  {
    udp::resolver resolver(io_context);
    udp::resolver::results_type endpoints = resolver.resolve(udp::v4(), host, service);
    _endpoint = *endpoints.begin();
    connect();
  }

  void connect()
  {
    std::shared_ptr<std::vector<char>> requestBuffer = std::make_shared<std::vector<char>>(max_length);
    char testBuffer[1024];
    PacketConnectionRequest packet;
    packet.client_guid = 0xDEADBEEF;
    packet.connect_sequence = 1;

    std::memcpy(requestBuffer->data(), &packet, sizeof(PacketConnectionRequest));
    std::memcpy(testBuffer, &packet, sizeof(PacketConnectionRequest));
    auto request_length = sizeof(PacketConnectionRequest);

    _socket.async_send_to(asio::buffer(*requestBuffer), _endpoint, [requestBuffer, this](std::error_code ec, std::size_t bytes_sent)
                          {
      if (ec) {
        std::cout << "Error sending packet: " << ec.message() << std::endl;
      }
      else {
        std::cout << "Sent packet" << std::endl;
        receive();
      } });
  }

  void receive()
  {
    std::shared_ptr<std::vector<char>> responseBuffer = std::make_shared<std::vector<char>>(max_length);
    _socket.async_receive_from(
        asio::buffer(*responseBuffer),
        _endpoint,
        [this, responseBuffer](std::error_code ec, std::size_t bytes_recvd)
        {
        if (ec) {
          std::cout << "Error receiving packet: " << ec.message() << std::endl;
        }
        else {
          Packet* packet = (Packet*)responseBuffer->data();
          std::cout << "Received packet: " << packet_type_string(packet->type) << std::endl;
          switch (packet->type)
          {
          case PACKET_TYPE_CONNECTION_ACCEPTED:
          {
            PacketConnectionAccepted *response = (PacketConnectionAccepted *)packet;
            std::cout << "Connection Accepted: " << response->client_guid << std::endl;
            std::cout << "Connection Sequence: " << response->connect_sequence << std::endl;
            break;
          }
          }
        } 
        receive();
        });
  }

private:
  udp::socket _socket;
  udp::endpoint _endpoint;
};

int main(int argc, char *argv[])
{
  try
  {
    std::string host;
    std::string service;
    if (argc != 3)
    {
      std::cerr << "Usage: blocking_udp_echo_client <host> <port>\n";
      // return 1;
      host = "localhost";
      service = "5000";
    }
    else
    {
      host = argv[1];
      service = argv[2];
    }

    asio::io_context io_context;

    client c(host, service, io_context);

    io_context.run();
  }
  catch (std::exception &e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}