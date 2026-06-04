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


int path_builder(const string& req_path, string& path) {
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
    string path = "/index.html";
    string op_path;
    if(path_builder(path,op_path) == 0) {
        cout<<"error"<<endl;
    }

    cout<<op_path<<endl;
    
}
*/
