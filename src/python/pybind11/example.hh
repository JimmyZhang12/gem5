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

    void init(std::string xml_dir, std::string serial_path);

    void compute();
    void update();
    ParseXML get_xml();
    float get_power();

    void test(std::string filepath);


    private:
    Processor proc;
    ParseXML *xml;
};





