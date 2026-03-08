#include <glaze/glaze.hpp>
#include <print>
#include <iostream>
#include <sstream>


struct A{
    int i;
    double d;
    std::string s;
};

int main(){
    A a{42, 3.14, "Hello, World!"};
    std::string buffer;
    auto res = glz::write_json(a, buffer);
    if(res){
        std::ostringstream ss;
        ss << res;
        std::println("Error: {}", ss.str());
        return 1;
    }
    std::println("{}", buffer);
    return 0;
}
