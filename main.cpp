#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
// blah blah

using boost::asio::ip::tcp;

class iclient
{
public:

  iclient(boost::asio::io_service &io_service,
	  const std::string &server, const std::string &path)
    : resolver_(io_service),
      socket_(io_service)
  {
    std::ostream request_stream(&request_);
    request_stream << "GET " << path << " HTTP/1.0\r\n";
    request_stream << "Host: " << server << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Connect: close\r\n\r\n";

    tcp::resolver::query query(server, "http");
    resolver_.async_resolve(query,
			    boost::bind(&iclient::handle_resolve, this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::iterator));
  }

private:
  void handle_resolve(const boost::system::error_code &err,
		      tcp::resolver::iterator endpoint_iterator)
  {
    if(!err) {
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
      boost::asio::async_write(socket_, request_,
			       boost::bind(&iclient::handle_write_request, this,
					   boost::asio::placeholders::error));
    } else if(endpoint_iterator != tcp::resolver::iterator()) {
      socket_.close();
      tcp::endpoint endpoint = *endpoint_iterator;
      socket_.async_connect(endpoint,
			    boost::bind(&iclient::handle_connect, this,
					boost::asio::placeholders::error, ++endpoint_iterator));
    } else {
      std::cout << "Error: " << err.message() << "\n";
    }
  }

  void handle_write_request(const boost::system::error_code &err)
  {
    if(!err) {
      boost::asio::async_read_until(socket_, response_, "\r\n",
				    boost::bind(&iclient::handle_read_status_line, this,
						boost::asio::placeholders::error));
    } else {
      std::cout << "Error: " << err.message() << "\n";
    }
  }

  void handle_read_status_line(const boost::system::error_code &err)
  {
    if(!err) {
      // check code
      std::istream response_stream(&response_);
      std::string http_version;
      response_stream >> http_version;
      unsigned int status_code;
      response_stream >> status_code;
      std::string status_message;
      std::getline(response_stream, status_message);
      if(!response_stream || http_version.substr(0, 5) != "HTTP/") {
	std::cout << "Invalid response\n";
	return;
      }

      if(status_code != 200) {
	std::cout << "Reponse returned with status code ";
	std::cout << status_code << "\n";
	return;
      }

      // Read the headers, terminated by blank line.
      boost::asio::async_read_until(socket_, response_, "\r\n\r\n",
				    boost::bind(&iclient::handle_read_headers, this,
						       boost::asio::placeholders::error));
	  
    } else {
      std::cout << "Error: " << err.message() << "\n";
    }
  }

  // handle the header of the response
  void handle_read_headers(const boost::system::error_code &err)
  {
    if(!err) {
      // Process the header
      std::istream response_stream(&response_);
      std::string header;
      while(std::getline(response_stream, header) && header != "\r")
	std::cout << header << "\n";
      std::cout << "\n";

      // Write content we have to output
      if(response_.size() > 0)
	std::cout << &response_;

      // read remaining data to EOF
      boost::asio::async_read(socket_, response_,
			      boost::asio::transfer_at_least(1),
			      boost::bind(&iclient::handle_read_content, this,
					  boost::asio::placeholders::error));
	  
    } else {
      std::cout << "Error: " << err.message() << "\n";
    }
  }

  void handle_read_content(const boost::system::error_code &err)
  {
    if(!err) {
      std::cout << &response_;

      boost::asio::async_read(socket_, response_,
			      boost::asio::transfer_at_least(1),
			      boost::bind(&iclient::handle_read_content, this,
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

void auth(tcp::socket *socket) {
  boost::asio::streambuf request;
  std::ostream request_stream(&request);

  request_stream << "USER enbot enbot enbot :enbot\r\n";
  request_stream << "NICK enbot\r\n";
  request_stream << "JOIN :#bot\r\n";
  request_stream << "PRIVMSG #bot :I have connected\r\n";
  boost::asio::write(*socket, request);

}

int main(int argc, char *argv[]) {
  try {
    if(argc != 3) {
      std::cout << "Usage: async_client <server> <path>\n";
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
