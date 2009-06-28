#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>

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

  void handle_connect(const boost::system::error_code &err,
		      tcp::resolver::iterator endpoint_iterator)
  {
    if(!err) {
      
    } else if(endpoint_iterator != tcp::resolver::iterator()) {
      socket_.close();
      tcp::endpoint endpoint = *endpoint_iterator;
      socket_.async_connect(endpoint,
			    boost::bind(&client::handle_connect, this
					boost::asio::placeholders::error, ++endpoint_iterator));
    } else {
      std::cout << "Error: " << err.message() << "\n";
    }
  }
}

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
    if(argc != 2) {
      std::cerr << "Usage: client <host>" << std::endl;
    }

    boost::asio::io_service io_service;

    tcp::resolver resolver(io_service);
    tcp::resolver::query query(argv[1], "6667");
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    tcp::resolver::iterator end;

    tcp::socket socket(io_service);
    boost::system::error_code error = boost::asio::error::host_not_found;

    while(error && endpoint_iterator != end) {
      socket.close();
      socket.connect(*endpoint_iterator++, error);
    }
    if(error)
      throw boost::system::system_error(error);

    // Authorize with the network
    auth(&socket);

    for (;;) {
      boost::array<char, 128> buf;
      boost::system::error_code error;

      size_t len = socket.read_some(boost::asio::buffer(buf), error);

      if(error == boost::asio::error::eof)
	break;
      else if(error)
	throw boost::system::system_error(error);

      std::cout.write(buf.data(), len);

    }
  }
  catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
  return 0;
}
