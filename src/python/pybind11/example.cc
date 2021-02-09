
#include "pybind11/pybind11.h"
// #include "pybind11/stl.h"

#include "XML_Parse.h"
#include "globalvar.h"
#include "io.h"
#include "options.h"
#include "processor.h"
#include "version.h"
#include "xmlParser.h"
#include "example.hh"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <fstream>
#include <iostream>
#include <string>

namespace py = pybind11;


Mcpat::Mcpat(){
    std::cout<<"mcpat constructor" << std::endl;
}


void
Mcpat::init(std::string xml_dir, std::string serial_path){
    std::cout<<"mcpat init:" << xml_dir << std::endl;
    xml = new ParseXML();
    xml->parse(xml_dir);
    proc.init(xml);
    bool long_channel = xml->sys.longer_channel_device;

    double gate_leakage = proc.power.readOp.gate_leakage;
    double sub_leakage = long_channel ? proc.power.readOp.power_gated_with_long_channel_leakage
                : proc.power.readOp.power_gated_leakage;
    double runtime_dynamic = proc.rt_power.readOp.dynamic;

    double power = gate_leakage + sub_leakage + runtime_dynamic;
    std::cout << "Gate Leakage = " << proc.power.readOp.gate_leakage
        << " W" << std::endl;
    std::cout << "Subthreshold Leakage with power gating = "
        << (long_channel
                ? proc.power.readOp.power_gated_with_long_channel_leakage
                : proc.power.readOp.power_gated_leakage)
        << " W" << std::endl;
    std::cout << "Runtime Dynamic = " << proc.rt_power.readOp.dynamic
        << " W" << std::endl;
    std::cout << "mcpat_internal power: " << power << std::endl;
}
 
void
Mcpat::compute(){
    proc.compute();
    bool long_channel = xml->sys.longer_channel_device;

    double gate_leakage = proc.power.readOp.gate_leakage;
    double sub_leakage = long_channel ? proc.power.readOp.power_gated_with_long_channel_leakage
                : proc.power.readOp.power_gated_leakage;
    double runtime_dynamic = proc.rt_power.readOp.dynamic;

    double power = gate_leakage + sub_leakage + runtime_dynamic;
    std::cout << "Gate Leakage = " << proc.power.readOp.gate_leakage
        << " W" << std::endl;
    std::cout << "Subthreshold Leakage with power gating = "
        << (long_channel
                ? proc.power.readOp.power_gated_with_long_channel_leakage
                : proc.power.readOp.power_gated_leakage)
        << " W" << std::endl;
    std::cout << "Runtime Dynamic = " << proc.rt_power.readOp.dynamic
        << " W" << std::endl;
    std::cout << "mcpat_internal power: " << power << std::endl;

}


void
Mcpat::update(){

}

// ParseXML
// Mcpat::get_xml(){
//     return xml;
// }


void 
pybind_init_mcpat_internal(py::module &m_native)
{
    py::module m = m_native.def_submodule("mcpat_internal");
    // m.def("test", &mcpat_internal::test, "an add function");

    py::class_<Mcpat>(m, "Mcpat")
        .def(py::init()) //the constructor
        .def("init", &Mcpat::init)
        .def("update", &Mcpat::update)
        .def("compute", &Mcpat::compute);
}