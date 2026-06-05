#include "http.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>

using namespace std;

string html_to_string(const string& path) {
    ifstream file(path);

    if(!file.is_open()) {
        cout<<"file not found"<<endl;
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
    cout<<combined_path<<endl;
    
    if(filesystem::exists(combined_path)) {
        path = combined_path;
        return 1;
    } else {
        return 0;
    }
}

void test_func() {
    string s = html_to_string("index.html");
    cout<<s;

}
/*
int main() {

    int res;

    string path = "../catamaran/../../vo/../../xoxo/../xoox";
    res = valid_path(path);

    if(res) cout<<"Path valid"<<endl;
    if(!res) cout<<"Path invalid"<<endl;
    
}
*/
