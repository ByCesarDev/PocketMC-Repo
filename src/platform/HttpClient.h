#ifndef HTTPCLIENT_H__
#define HTTPCLIENT_H__

#include <string>
#include <vector>

namespace HttpClient {

bool download(const std::string& url, std::vector<unsigned char>& outBody);

// HTTP POST — sends jsonBody as application/json, returns response in outBody
// optionally pass extra headers as "Key: Value\r\n" strings (e.g. Authorization header)
bool post(const std::string& url, const std::string& jsonBody, std::vector<unsigned char>& outBody, const std::string& extraHeaders = "");

} // namespace HttpClient

#endif /* HTTPCLIENT_H__ */
