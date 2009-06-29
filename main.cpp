/* IRC client class
 * could be used to make a bot
 * or a full blown IRC client
 * Jeff Aigner 2009
 */

#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

using boost::asio::ip::tcp;



class iclient
{
public:

  iclient(boost::asio::io_service &io_service,
	  const std::string &server, const std::string &port)
    : resolver_(io_service),
      socket_(io_service)
  {

    // build the initial auth request
    std::ostream request_stream(&request_);
    request_stream << "USER enbot enbot enbot :enbot\r\n";
    request_stream << "NICK enbot\r\n";

    // initiate host resolution
    tcp::resolver::query query(server, port);
    resolver_.async_resolve(query,
			    boost::bind(&iclient::handle_resolve, this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::iterator));
  }

private:

  // handle host resolution
  void handle_resolve(const boost::system::error_code &err,
		      tcp::resolver::iterator endpoint_iterator)
  {
    if(!err) {
      // initiate connect host iteration
      tcp::endpoint endpoint = *endpoint_iterator;
      socket_.async_connect(endpoint,
			    boost::bind(&iclient::handle_connect, this,
					boost::asio::placeholders::error, ++endpoint_iterator));
    } else {
      std::cout << "Error: " << err.message() << "\n";
    }
  }



  // Handle connection to the server
  void handle_connect(const boost::system::error_code &err,
		      tcp::resolver::iterator endpoint_iterator)
  {
    if(!err) {
      // Write the Auth request
      boost::asio::async_write(socket_, request_,
			       boost::bind(&iclient::handle_write_request, this,
					   boost::asio::placeholders::error));

    } else if(endpoint_iterator != tcp::resolver::iterator()) {
      // continue host iteration
      socket_.close();
      tcp::endpoint endpoint = *endpoint_iterator;
      socket_.async_connect(endpoint,
			    boost::bind(&iclient::handle_connect, this,
					boost::asio::placeholders::error, ++endpoint_iterator));
    } else {
      std::cout << "Error: " << err.message() << "\n";
    }
  }

  // Handle line from server
  void handle_read(const boost::system::error_code &err)
  {
    if(!err) {
      std::istream response_stream(&response_);
      std::string line;
      
      // process line
      while(std::getline(response_stream, line)) {
	std::cout << line << "\n";

	//  respond to server ping
	if(line.substr(0, 4) == "PING") {
	  std::ostream request_string(&request_);

	  request_string << "PONG :" << line.substr(6, line.size()) << "\r\n";
	  std::cout << "PONG!" << "\n";

	  boost::asio::async_write(socket_, request_,
				   boost::bind(&iclient::handle_write, this,
					       boost::asio::placeholders::error));
	}
      }


      // continue read cycle
      boost::asio::async_read_until(socket_, response_, "\r\n",
			      boost::bind(&iclient::handle_read, this,
					  boost::asio::placeholders::error));
    } else {
      std::cout << "Error: " << err.message() << "\n";
    }
  }

  // handle a write by simply checking for errors
  void handle_write(const boost::system::error_code &err) {
    if(err) {
      std::cout << "Error: " << err.message() << "\n";
    }
  }

  // handle initial write request and start reading cycle
  void handle_write_request(const boost::system::error_code &err)
  {
    if(!err) {
      boost::asio::async_read_until(socket_, response_, "\r\n",
				    boost::bind(&iclient::handle_read, this,
						boost::asio::placeholders::error));
    } else {
      std::cout << "Error: " << err.message() << "\n";
    }
  }

  tcp::resolver resolver_;
  tcp::socket socket_;
  boost::asio::streambuf request_;
  boost::asio::streambuf response_;

};

int main(int argc, char *argv[]) {
  try {
    if(argc != 3) {
      std::cout << "Usage: async_client <server> <port>\n";
      return 1;
    }
    boost::asio::io_service io_service;
    iclient c(io_service, argv[1], argv[2]);
    io_service.run();
  }
  catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
  return 0;
}
