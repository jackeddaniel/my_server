#include "http.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <unordered_map>

using namespace std;

unordered_map<string, string> mime_types = { 
    {".html" ,"text/html"},
    {".css", "text/css"},
    {".js", "application/js"},
    {".png", "image/png"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".ico", "image/x-icon"}
};

string html_to_string(const string& path) {
    ifstream file(path);

    if(!file.is_open()) {
        return "";
    }

    ostringstream buffer; 
    buffer << file.rdbuf();

    string file_contents = buffer.str();
    return file_contents;
}

int valid_path(const string& req_path) {
    int stack_top = 0;
    if(req_path.empty() || req_path[0] != '/') return 0;
    if(req_path.size() > 2 && req_path[1] == '.' && req_path[2] == '.') return 0;

    int i = 0;

    while(i < req_path.size()) {
        while(i < req_path.size() - 1 && req_path[i] != '/') i++;
        if(i >= req_path.size()) break;

        if(i+2 < req_path.size() && (req_path[i+1] == '.' && req_path[i+2] == '.')) {
            stack_top--;
            if(stack_top < 0) {
                return 0;
            }
        } else {
            stack_top++;
        }
        i++;
    }
    return 1;
}

int path_builder(const string& req_path, string& path) {
    if(!valid_path(req_path)) return 0;
    string parent = "static";
    filesystem::path combined_path = parent + req_path;
    
    if(filesystem::exists(combined_path)) {
        path = combined_path;
        return 1;
    } else {
        return 0;
    }
}

string infer_content_type(const string& path) {
    //this assumes the path is valid and the string is safe to parse

    int i = path.size() - 1;

    while(i > 0 && path[i] != '.') i--;


    string ext = path.substr(i);
    if(mime_types.find(ext) == mime_types.end()) return "";

    return mime_types[ext];
}

void test_func() {
    string s = html_to_string("index.html");

}
/*
int main() {

    int res;

    string path = "../catamaran/../../vo/../../xoxo/../xoo.py";

    cout<<infer_content_type(path)<<endl;
    
}
*/
