#include "server.hpp"
#include <charconv>
#include <optional>
#include <string_view>
#include <system_error>
#include <span>


template<typename T>
std::optional<T> from_chars(std::string_view sv) noexcept {
  T out;
  auto end = sv.data()+sv.size();
  auto res = std::from_chars(sv.data(),end,out);
  if(res.ec==std::errc{}&&res.ptr==end)
    return out;
  return {};
}	

int main(std::span<const char* const> args) {
  auto port = from_chars<std::uint16_t>(args[1]);
  auto t = from_chars<unsigned>(args[2]);
  unsigned threads;
  threads = *t;
  boost::asio::io_context io_context;
  server srv(io_context, *port);
  srv.async_accept();
  ba::static_thread_pool tp{threads-1};
  for(unsigned i=1;i<threads;++i)
    ba::execution::execute(tp.get_executor(),[&]{
      io_context.run();
    });
  io_context.run();
  return 0;
}
