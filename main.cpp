#include <iostream>
#include "nanojson.h"
#include <vector>

struct Person : public nanojson::object<Person>
{
    def(std::string, name);
    def(int, age);
};

struct JSONSample : public nanojson::object<JSONSample>
{
    def(std::string, hoge);
    def(std::vector<Person>, list);
};

int main(int argc, const char * argv[])
{
    JSONSample json;
    nanojson::reader reader("test.json");

    json = reader.parse<JSONSample>();
    std::cout << "hoge = " << json.hoge << std::endl;
    for(std::vector<Person>::iterator it = json.list.begin(); it != json.list.end(); ++it)
        std::cout << "    name: " << it->name << ", age: " << it->age << std::endl;

    return 0;
}
