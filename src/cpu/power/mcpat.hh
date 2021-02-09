// #include <fcntl.h>
// #include <semaphore.h>
// #include <stdint.h>
// #include <stdio.h>
// #include <stdlib.h>
#include <string>
// #include <sys/mman.h>
// #include <sys/shm.h>
// #include <sys/stat.h>
#include <unistd.h>

#include "XML_Parse.h"
#include "processor.h"
#include "xmlParser.h"

class Mcpat{
    public:
    Mcpat();

    void init(std::string xml_dir);
    void reset();
    void compute(std::string output_path);
    void init_wrapper(std::string xml_dir, std::string output_path);
    void update();
    ParseXML get_xml();
    void get_power();

    void test(std::string filepath);

    Processor proc; //eventually make the private
    private:
    ParseXML *xml;
};





