// SPDX-License-Identifier: Apache-2.0
// HTTP client abstraction for Traccar protocol

#ifndef UMAP_DATA_HTTP_CLIENT_H_
#define UMAP_DATA_HTTP_CLIENT_H_

#include <cstdint>
#include <string>
#include <vector>
#include <utility>

#include "umap/utils/result.h"

namespace umap::data {

struct HttpResponse {
    int status_code = 0;
    std::string body;
};

class IHttpClient {
public:
    virtual ~IHttpClient() = default;

    virtual utils::Result<HttpResponse> get(const char* url) = 0;
    virtual utils::Result<HttpResponse> post(const char* url,
                                              const char* content_type,
                                              const char* body) = 0;
};

class MockHttpClient : public IHttpClient {
public:
    utils::Result<HttpResponse> get(const char* url) override;
    utils::Result<HttpResponse> post(const char* url,
                                      const char* content_type,
                                      const char* body) override;

    void set_response(int status_code, const char* body);
    void set_should_fail(bool fail);

    struct RequestRecord {
        std::string method;
        std::string url;
        std::string content_type;
        std::string body;
    };

    const std::vector<RequestRecord>& requests() const { return requests_; }
    uint32_t request_count() const { return static_cast<uint32_t>(requests_.size()); }
    void reset();

private:
    int response_status_ = 200;
    std::string response_body_ = "{}";
    bool should_fail_ = false;
    std::vector<RequestRecord> requests_;
};

}  // namespace umap::data

#endif  // UMAP_DATA_HTTP_CLIENT_H_