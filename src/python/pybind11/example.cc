#define _GLIBCXX_USE_CXX11_ABI 1

#include "pybind11/pybind11.h"
// #include "pybind11/stl.h"

// #include <sys/mman.h>
// #include <sys/shm.h>
// #include <sys/stat.h>
// #include <unistd.h>
// #include <stdio.h>
// #include <cstdio>

// #include "base/statistics.hh"
// #include "base/stats/text.hh"
// #include "sim/stat_control.hh"
// #include "sim/stat_register.hh"

// #include "example.hh"
#include "mcpat_src/XML_Parse.h"
// #include "mcpat_src/globalvar.h"
// #include "mcpat_src/options.h"
// #include "mcpat_src/processor.h"
// #include "mcpat_src/version.h"
// #include "mcpat_src/xmlParser.h"

namespace py = pybind11;

namespace mcpat_internal{
    using namespace std;
    void
    test(std::string filepath) {
        ParseXML *p1 = new ParseXML();
        p1->parse(filepath);
        // Processor proc;

        // proc.init(p1);
        // proc.displayEnergy(2, 1);
        // proc.compute(p1);

    }

}


void 
pybind_init_mcpat_internal(py::module &m_native)
{
    py::module m = m_native.def_submodule("mcpat_internal");
    m.def("test", &mcpat_internal::test, "an add function");
}