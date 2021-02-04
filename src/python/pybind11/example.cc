#define _GLIBCXX_USE_CXX11_ABI 0

#include "pybind11/pybind11.h"
// #include "pybind11/stl.h"

#include "XML_Parse.h"
#include "processor.h"
// #include "xmlParser.h"
#include "example.hh"

namespace py = pybind11;


Mcpat::Mcpat(){
    std::cout<<"mcpat constructor" << std::endl;
}

void
Mcpat::init(){
    std::cout<<"mcpat init" << std::endl;
    xml = new ParseXML();
}


void 
pybind_init_mcpat_internal(py::module &m_native)
{
    py::module m = m_native.def_submodule("mcpat_internal");
    // m.def("test", &mcpat_internal::test, "an add function");

    py::class_<Mcpat>(m, "Mcpat")
        .def(py::init())
        .def("init", &Mcpat::init);
        // .def("getName", &Pet::getName);

}