#include "http.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <iostream>
#include <sstream>
#include <map>

using namespace std;

http_request build_request(string req) {
    http_request request;
    istringstream stream(req); 
    int first_line = 0;
    int new_line_count = 0;

    string line;
    stringstream ss(req);

    while(getline(ss, line)) {
        if(first_line == 0) {
            istringstream firstline(line);
            string chunk;
            int fl_counter = 0;
            //parsing the first line
            while(fl_counter < 3 && firstline >> chunk) {
                if(fl_counter == 0) {
                    request.req_method = chunk;
                } else if(fl_counter == 1) {
                    request.path = chunk;
                } else { 
                    break;
                }
                fl_counter++;
            }
            first_line++;
            continue;
        }
        
        //parsing the last segment(body) which is seperated by newline
        if(new_line_count == 1) {
            request.body += line + "\n";
            continue;
        }

        if(line.empty()) {
            new_line_count++;
            continue;
        }
        
        size_t col_pos = -1;

        for(int i = 0; i < line.size(); i++) {
            if(line[i] == ':') {
                col_pos = i;
                break;
            }
        }
    
        string key = line.substr(0, col_pos);
        col_pos++;
        while(line[col_pos] == ' ') col_pos++;
        string val = line.substr(col_pos);

        request.headers[key] = val;

    }
    return request;
}

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
       rp += "\r\n";
    }
    rp += "\r\n";
    rp += resp.body;
    return rp;
}

void print_request(http_request req) {
    cout<<"req_method: "<<req.req_method<<endl;
    cout<<"path: "<<req.path<<endl;
    for(const auto& [key, val] : req.headers) {
        cout<<key<<": "<<val<<endl;
    }
    cout<<"body: "<<req.body<<endl;
}

void tester(string s) {
    istringstream ss(s);

    string line;

    while(getline(ss, line)) {
        cout<<line<<"\n";
    }

}

int main() {
    cout<<"Testing responses"<<endl;

    http_response dummy_res = create_response(200, "<html><body>hello world</body></html>", "text/html");

    string s = build_response(dummy_res);
    cout<<s<<endl;
    
    string test_req = R"http(POST /api/user HTTP/1.1
Host: localhost:3490
User-Agent: curl/8.5.0
Accept: */*
Content-Length: 54
Content-Type: application/x-www-form-urlencoded

username=alice
password=secret
role=developer
location=local)http";
    cout<<"Testing request parsing"<<endl;
    //tester(test_req);
    http_request dummy_req = build_request(test_req);
    print_request(dummy_req);

}

