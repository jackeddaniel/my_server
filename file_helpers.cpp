#include "http.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

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

void test_func() {
    string s = html_to_string("index.html");
    cout<<s;

}
    
