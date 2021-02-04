#include <fcntl.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>

// #include "example.hh"
#include "XML_Parse.h"
#include "processor.h"
#include "xmlParser.h"

class Mcpat{
    public:
    Mcpat();

    void init();

    void compute();

    void test(std::string filepath);

    private:
    Processor proc;
    ParseXML *xml;
};





