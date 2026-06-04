#pragma once

#include <unistd.h>
#include <string>
#include <map>

using namespace std;

//http response struct
struct http_response {
    int status_code;
    string status_text;
    map<string, string> headers;
    string body;
};

//http request struct
struct http_request {
    string req_method;
    string path;
    map<string, string> headers;
    string body;
};

//http_helpers.cpp
http_response create_response(int status_code, const string& body, const string& content_type);
string build_response(const http_response& resp);

//file_helpers.cpp
string html_to_string(const string& path);
