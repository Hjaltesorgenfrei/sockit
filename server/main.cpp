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
#include <functional>

using asio::ip::udp;

struct Connection
{
  udp::endpoint endpoint;
  uint64_t guid;
};

typedef std::function<bool(Packet*, udp::endpoint)> handle_func;
struct Handler {
  PacketType type;
  handle_func handler;
};

class server
{
public:
  server(asio::io_context &io_context, short port)
      : _socket(io_context, udp::endpoint(udp::v6(), port)), heartbeat_timer(io_context, std::chrono::seconds(heartbeat_interval))
  {
    add_handler(PACKET_TYPE_CONNECTION_REQUEST, [this](Packet* packet, udp::endpoint senderEndpoint) {
      PacketConnectionRequest *request = (PacketConnectionRequest *)packet;
        std::cout << "Connection Request: " << request->client_guid << std::endl;
        std::cout << "Connection Sequence: " << request->connect_sequence << std::endl;
        PacketConnectionAccepted response;
        response.client_guid = request->client_guid;
        response.connect_sequence = request->connect_sequence + 1;
        
        // If the guid is already in the list, remove it
        for (auto it = _connections.begin(); it != _connections.end(); ++it)
        {
          if (it->guid == request->client_guid)
          {
            _connections.erase(it);
            break;
          }
        }
        Connection connection;
        connection.endpoint = senderEndpoint;
        connection.guid = request->client_guid;
        _connections.push_back(connection);
        do_send(sizeof(PacketConnectionAccepted), response, connection);
        return true;
    });
    heart_beat();
    do_receive();
  }

  void heart_beat() {
    // Make sure we send a heartbeat every 5 seconds
    heartbeat_timer.async_wait([this](std::error_code ec) {
      std::cout << "Sending heartbeat" << std::endl;
      if (!ec)
      {
        PacketHeartbeat heartbeat;
        for (auto it = _connections.begin(); it != _connections.end(); ++it)
        {
          do_send(sizeof(PacketHeartbeat), heartbeat, *it);
        }
      }
      heartbeat_timer.expires_at(heartbeat_timer.expiry() + std::chrono::seconds(heartbeat_interval));
      heart_beat();
    });
  }

  void do_receive()
  {
    std::shared_ptr<udp::endpoint> senderEndpoint = std::make_shared<udp::endpoint>();
    _socket.async_receive_from(
        asio::buffer(_receiveData, max_length), *senderEndpoint,
        [this, senderEndpoint](std::error_code ec, std::size_t bytes_recvd)
        {
          std::cout << "Sender: " << *senderEndpoint << std::endl;
          if (!ec && bytes_recvd > 0)
          {
            Packet* packet = (Packet *)_receiveData;
            handle_packet(packet, *senderEndpoint);
          }
          else {
            std::cout << "Error: " << ec.message() << std::endl;
          }
          do_receive();
        });
  }

  void handle_packet(Packet *packet, udp::endpoint senderEndpoint)
  {
    std::cout << "Received packet: " << packet_type_string(packet->type) << std::endl;
    for (auto it = _handlers.begin(); it != _handlers.end(); ++it)
    {
      if (it->type == packet->type)
      {
        if (it->handler(packet, senderEndpoint))
        {
          break;
        }
      }
    }
  }

  void do_send(std::size_t length, Packet &packet, Connection connection)
  {
    std::shared_ptr<std::vector<char>> data = std::make_shared<std::vector<char>>(length);
    memcpy(data->data(), &packet, length);
    _socket.async_send_to(
        asio::buffer(data->data(), length), connection.endpoint,
        [data, connection](std::error_code ec, std::size_t /*bytes_sent*/)
        {
          if (ec)
          {
            std::cout << "Error Sending: " << ec.message() << std::endl;
            std::cout << "Connection Endpoint: " << connection.endpoint << std::endl;
          }
        });
  }

private:
  void add_handler(PacketType type, handle_func handler)
  {
    _handlers.push_back({ type, handler });
  }

  udp::socket _socket;
  std::vector<Connection> _connections;
  asio::steady_timer heartbeat_timer;
  enum
  {
    max_length = 1024,
    heartbeat_interval = 5,
  };
  char _receiveData[max_length];
  char _sendData[max_length];
  std::vector<Handler> _handlers;
};

int main(int argc, char *argv[])
{
  try
  {
    const char * port = "5000";
    if (argc != 2)
    {
      std::cerr << "Usage: async_udp_echo_server <port>\n";
    }
    else {
      port = argv[1];
    }

    asio::io_context io_context;

    server s(io_context, std::atoi(port));

    io_context.run();
  }
  catch (std::exception &e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  std::cout << "Server Exited" << std::endl;
  return 0;
}