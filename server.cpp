#include "server.hpp"
#include <string>
#include <iostream>
#include <filesystem>
#include <boost/process.hpp>
//#include <boost/asio/spawn.hpp>
#include "spawn.hpp"

namespace bp = boost::process;

void prettify(std::string& dir) {
  dir.insert(0, "[mattshell:");
  dir.append("]$ ");
}

void session::start() {
  try {
   bp::environment e = boost::this_process::environment();
   bp::child c(io_context, e.at("SHELL").to_string());
   std::string currDir;
   bp::ipstream dirpipe;
   bp::system(io_context, "pwd", bp::std_out > dirpipe);
   std::getline(dirpipe, currDir);
   currDir.insert(0, "[mattshell:");
   currDir.append("]$ ");
   ce::spawn(io_context.get_executor(), [ &currDir, this, self = shared_from_this()](auto yc){
     boost::system::error_code ec;
     std::string bufDir = currDir;
     ba::async_write(socket, ba::buffer(bufDir, bufDir.size()), yc[ec]);
     for(;;) {
       std::string s;
       bp::ipstream ipipe;
       std::string line;
       std::string result;
       ba::async_read_until(socket, streambuf, "\n", yc[ec]); 
       if (ec)
         std::cout << "error\n";
       else {
         std::istream is(&streambuf);
	 std::getline(is, s);
	 streambuf.consume(s.size());
	 if (s == "exit") {
	   std::cout << "exit\n";
	   return 0;
	 }
	 else if (s.substr(0, 2) == "cd") {
           auto newDir = std::filesystem::current_path() / s.substr(3);
	   std::filesystem::current_path(newDir);
	   std::cout << std::filesystem::current_path() << '\n';
	   std::cout << std::string(newDir);
	 }
	 else {
	   bp::system(io_context, s, bp::std_out > ipipe);
       	   while (std::getline(ipipe, line) && !line.empty()) {
             result.append(line);
	     result.append("\n");
           }
         }
	 
	 std::string newPath = std::filesystem::current_path();
         prettify(newPath);
	 result.append(newPath);
       }
       ba::async_write(socket, ba::buffer(result, result.size()), yc[ec]);
       if (ec)
         std::cout << "not read\n";
       else
	 result.clear();
     }    
   }, {}, ba::bind_executor(ba::prefer(io_context.get_executor(),ba::execution::relationship.continuation), [](std::exception_ptr e){
            if(e) 
              std::rethrow_exception(e);
    	     }));
    	 
    	     
  }
  catch (bp::process_error e) {
    std::cout << "Caught exception: " << e.what();
  }
}

void server::async_accept() {
  socket.emplace(io_context);
  acceptor.async_accept(*socket, [&](boost::system::error_code error) {
				   std::make_shared<session>(std::move(*socket), io_context)->start();
                                   async_accept();
				 });
}

