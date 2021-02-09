
#include "XML_Parse.h"
#include "globalvar.h"
#include "io.h"
#include "options.h"
#include "processor.h"
#include "version.h"
#include "xmlParser.h"
#include "mcpat.hh"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <fstream>
#include <iostream>
#include <string>

Mcpat::Mcpat(){
    std::cout<<"mcpat constructor" << std::endl;
}


void
Mcpat::init(std::string xml_dir){
    std::cout<<"    mcpat init:" << xml_dir << std::endl;
    xml = new ParseXML();
    xml->parse(xml_dir);
    proc.init(xml);
    bool long_channel = xml->sys.longer_channel_device;

    double gate_leakage = proc.power.readOp.gate_leakage;
    double sub_leakage = long_channel ? proc.power.readOp.power_gated_with_long_channel_leakage
                : proc.power.readOp.power_gated_leakage;
    double runtime_dynamic = proc.rt_power.readOp.dynamic;

    double power = gate_leakage + sub_leakage + runtime_dynamic;
    std::cout << "  Gate Leakage = " << proc.power.readOp.gate_leakage
        << " W" << std::endl;
    std::cout << "  Subthreshold Leakage with power gating = "
        << (long_channel
                ? proc.power.readOp.power_gated_with_long_channel_leakage
                : proc.power.readOp.power_gated_leakage)
        << " W" << std::endl;
    std::cout << "  Runtime Dynamic = " << proc.rt_power.readOp.dynamic
        << " W" << std::endl;
    std::cout << "  mcpat_internal power: " << power << std::endl;
}
 
void
Mcpat::compute(std::string output_path){
    proc.reset();
    proc.compute();
    bool long_channel = xml->sys.longer_channel_device;

    double gate_leakage = proc.power.readOp.gate_leakage;
    double sub_leakage = long_channel ? proc.power.readOp.power_gated_with_long_channel_leakage
                : proc.power.readOp.power_gated_leakage;
    double runtime_dynamic = proc.rt_power.readOp.dynamic;

    double power = gate_leakage + sub_leakage + runtime_dynamic;
    std::cout << "      Gate Leakage = " << proc.power.readOp.gate_leakage
        << " W" << std::endl;
    std::cout << "      Subthreshold Leakage with power gating = "
        << (long_channel
                ? proc.power.readOp.power_gated_with_long_channel_leakage
                : proc.power.readOp.power_gated_leakage)
        << " W" << std::endl;
    std::cout << "      Runtime Dynamic = " << proc.rt_power.readOp.dynamic
        << " W" << std::endl;
    std::cout << "      mcpat_internal power: " << power << std::endl;


    output_path = output_path + "/out_mcpat_internal.txt";
    std::cout << output_path << '\n';

    std::ofstream out(output_path);
    std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
    std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!
    proc.displayEnergy(2, 5);
    std::cout.rdbuf(coutbuf); //reset to standard output again
}

void
Mcpat::init_wrapper(std::string xml_dir, std::string output_path){
    std::cout<<"    mcpat init_wrapper:" << std::endl;
    xml = new ParseXML();
    xml->parse(xml_dir);
    proc.reset();

    std::string serial = output_path + "/mcpat_serial.txt";
    Processor proc_serial;

    std::ifstream ifs(serial.c_str());
    if (ifs.good()) {
        boost::archive::text_iarchive ia(ifs);
        ia >> proc_serial;
    } else {
        std::cerr << "Archive " << serial << " cannot be used\n";
        assert(false);
    }
    std::cout<<"    mcpat proc_serial:" << std::endl;
    proc_serial.init(xml,true);
    proc.init(xml,true);

    bool long_channel = xml->sys.longer_channel_device;
    double gate_leakage = proc_serial.power.readOp.gate_leakage;
    double sub_leakage = long_channel ? proc_serial.power.readOp.power_gated_with_long_channel_leakage
                : proc_serial.power.readOp.power_gated_leakage;
    double runtime_dynamic = proc_serial.rt_power.readOp.dynamic;
    double power = gate_leakage + sub_leakage + runtime_dynamic;

    std::cout << "  Gate Leakage = " << proc_serial.power.readOp.gate_leakage
        << " W" << std::endl;
    std::cout << "  Subthreshold Leakage with power gating = "
        << (long_channel
                ? proc_serial.power.readOp.power_gated_with_long_channel_leakage
                : proc_serial.power.readOp.power_gated_leakage)
        << " W" << std::endl;
    std::cout << "  Runtime Dynamic = " << proc_serial.rt_power.readOp.dynamic
        << " W" << std::endl;
    std::cout << "  mcpat_internal power: " << power << std::endl;

    output_path = output_path + "/out_mcpat_internal.txt";
    std::cout << output_path << '\n';

    get_power();

    std::ofstream out(output_path);
    std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
    std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!
    proc.displayEnergy(2, 5);
    std::cout.rdbuf(coutbuf); //reset to standard output again

}

void
Mcpat::reset(){
    proc.reset();
}

void
Mcpat::get_power(){
    bool long_channel = xml->sys.longer_channel_device;
    double gate_leakage = proc.power.readOp.gate_leakage;
    double sub_leakage = long_channel ? proc.power.readOp.power_gated_with_long_channel_leakage
                : proc.power.readOp.power_gated_leakage;
    double runtime_dynamic = proc.rt_power.readOp.dynamic;
    double power = gate_leakage + sub_leakage + runtime_dynamic;

    std::cout << "  Gate Leakage = " << proc.power.readOp.gate_leakage
        << " W" << std::endl;
    std::cout << "  Subthreshold Leakage with power gating = "
        << (long_channel
                ? proc.power.readOp.power_gated_with_long_channel_leakage
                : proc.power.readOp.power_gated_leakage)
        << " W" << std::endl;
    std::cout << "  Runtime Dynamic = " << proc.rt_power.readOp.dynamic
        << " W" << std::endl;
    std::cout << "  mcpat_internal power: " << power << std::endl;
}
