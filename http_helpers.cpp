#include "http.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <iostream>
#include <map>

using namespace std;

http_response create_response(int status_code, const string& body, const string& content_type) {
    http_response response;
    response.status_code = status_code;
    response.status_text = "OK";
    response.body = body;
    response.headers["Content-Type"] = content_type;
    response.headers["Content-length"] = to_string(body.size());
    response.headers["Connection"] = "close";
    return response;
}

string build_response(const http_response& resp) {
    string rp;
    rp += "HTTP/1.1 ";
    rp += to_string(resp.status_code);
    rp += " ";
    rp += resp.status_text;
    rp += "\r\n";
    
    for(const auto& [key, value] : resp.headers) {
       rp += key;
       rp += ": ";
       rp += value;
       rp += "\t\n";
    }
    rp += "\r\n";
    rp += resp.body;
    return rp;
}

void test() {
    http_response dummy_res = create_response(200, "<html><body>hello world</body></html>", "text/html");

    string s = build_response(dummy_res);

    cout<<s<<endl;
}

