#include "./fetch.h"
#include "./Log.h"
#include <http_stl.h>
#include <sstream>

std::string fetchGet(const std::string &url) {
  http::stl::session_t sess("chief-client/" CLIENT_VERSION_STRING);
  http::stl::connection_t conn(sess, url.c_str());

  http::stl::request_t req("GET", url.c_str());
  http::stl::response_t res = conn.send(req);

  if (res.failed())
    throw std::runtime_error(res.error());

  std::stringstream ss;

  res.read(ss);

  return {std::istreambuf_iterator<char>(ss), {}};
}