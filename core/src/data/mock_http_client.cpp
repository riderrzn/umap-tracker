// SPDX-License-Identifier: Apache-2.0
// Mock HTTP client implementation for testing

#include "umap/data/http_client.h"

namespace umap::data {

utils::Result<HttpResponse> MockHttpClient::get(const char* url) {
    requests_.push_back({"GET", url ? url : "", "", ""});
    if (should_fail_) {
        return utils::Unexpected(utils::ErrorCode::NetworkError);
    }
    HttpResponse resp;
    resp.status_code = response_status_;
    resp.body = response_body_;
    return resp;
}

utils::Result<HttpResponse> MockHttpClient::post(const char* url,
                                                   const char* content_type,
                                                   const char* body) {
    requests_.push_back({"POST",
                          url ? url : "",
                          content_type ? content_type : "",
                          body ? body : ""});
    if (should_fail_) {
        return utils::Unexpected(utils::ErrorCode::NetworkError);
    }
    HttpResponse resp;
    resp.status_code = response_status_;
    resp.body = response_body_;
    return resp;
}

void MockHttpClient::set_response(int status_code, const char* body) {
    response_status_ = status_code;
    response_body_ = body ? body : "";
}

void MockHttpClient::set_should_fail(bool fail) {
    should_fail_ = fail;
}

void MockHttpClient::reset() {
    requests_.clear();
    response_status_ = 200;
    response_body_ = "{}";
    should_fail_ = false;
}

}  // namespace umap::data
