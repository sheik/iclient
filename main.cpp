/* IRC client class
 * could be used to make a bot
 * or a full blown IRC client
 * Jeff Aigner 2009
 */

#include "iclient.cpp"

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

