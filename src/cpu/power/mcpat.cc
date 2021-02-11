
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
#include <list>


Mcpat::Mcpat(){
    std::cout<<"mcpat constructor" << std::endl;
}


void
Mcpat::init(std::string xml_dir){
    std::cout<<"    mcpat init:" << xml_dir << std::endl;
    xml = new ParseXML();
    xml->parse(xml_dir);
    proc.init(xml);
    print_power(proc);

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
    
    std::cout<<"mcpat proc_serial:" << std::endl;
    proc_serial_xml = new ParseXML();
    proc_serial_xml->parse(xml_dir);
    proc_serial.init(proc_serial_xml,true);
    print_power(proc_serial);

    std::string output_path_serial = output_path + "/out_mcpat_serial.txt";
    save_output(output_path_serial, proc_serial);



    std::cout<<"mcpat internal:" << std::endl;
    update_stats();
    proc.reset();
    proc.compute();
    print_power(proc);

    std::string output_path_internal = output_path + "/out_mcpat_internal.txt";
    save_output(output_path_internal, proc);

}

void
Mcpat::update_stats(){

    //legacy stats
    list<Stats::Info *>& statlist = Stats::statsList();
    list<Stats::Info *>::iterator it;
    for (it = statlist.begin(); it != statlist.end(); ++it){
        set_mcpat_stat((*it));
    }

    //new stats
    Root* root = Root::root();
    update_stats_helper(root, "");

    //stats with >1 dependencies
    proc.XML->sys.core[0].busy_cycles = stat_storage.numCycles-stat_storage.idleCycles;

    proc.XML->sys.core[0].rename_writes = 
        stat_storage.RenamedOperands*stat_storage.int_rename_lookups/
        (1+stat_storage.RenameLookups);
    proc.XML->sys.core[0].fp_rename_writes = 
        stat_storage.RenamedOperands*stat_storage.fp_rename_lookups/
        (1+stat_storage.RenameLookups);

    proc.XML->sys.L2->read_accesses = 
        stat_storage.l2_ReadExReq_accesses + 
        stat_storage.l2_ReadCleanReq_accesses + 
        stat_storage.l2_ReadSharedReq_accesses;
    proc.XML->sys.L2->read_misses = 
        stat_storage.l2_ReadCleanReq_misses + 
        stat_storage.l2_ReadExReq_misses;
    proc.XML->sys.L2->write_accesses =
        stat_storage.l2_WritebackDirty_accesses + 
        stat_storage.l2_WritebackClean_accesses;
    proc.XML->sys.L2->write_misses =
        stat_storage.l2_WritebackClean_accesses -
        stat_storage.l2_WritebackDirty_hits;

    proc.XML->sys.L3->read_accesses = 
        stat_storage.l3_ReadExReq_accesses + 
        stat_storage.l3_ReadCleanReq_accesses + 
        stat_storage.l3_ReadSharedReq_accesses;
    proc.XML->sys.L3->read_misses = 
        stat_storage.l3_ReadCleanReq_misses + 
        stat_storage.l3_ReadExReq_misses;
    proc.XML->sys.L3->write_accesses =
        stat_storage.l3_WritebackDirty_accesses + 
        stat_storage.l3_WritebackClean_accesses;
    proc.XML->sys.L3->write_misses =
        stat_storage.l3_WritebackClean_accesses -
        stat_storage.l3_WritebackDirty_hits;

}

void
Mcpat::update_stats_helper(Stats::Group* group, std::string path){
    const std::vector< Stats::Info * >&  stats = group->getStats();
    std::vector<Stats::Info *>::const_iterator it;
    for (it = stats.begin(); it != stats.end(); ++it){
        set_mcpat_stat((*it));
    }
    
    const std::map< std::string, Stats::Group * > & childGroups = group->getStatGroups();    
    for (auto const& x : childGroups){
        std::string child_path = path + x.first + ".";
        update_stats_helper(x.second, child_path);
    }
}

void
Mcpat::reset(){ proc.reset();}

void
Mcpat::print_power(Processor &proc_t){
    bool long_channel = xml->sys.longer_channel_device;
    double gate_leakage = proc_t.power.readOp.gate_leakage;
    double sub_leakage = long_channel ? proc_t.power.readOp.power_gated_with_long_channel_leakage
                : proc_t.power.readOp.power_gated_leakage;
    double runtime_dynamic = proc_t.rt_power.readOp.dynamic;
    double power = gate_leakage + sub_leakage + runtime_dynamic;

    //this is a hack!
    proc_t.l3array[0].rt_power.reset();

    std::cout << "  Gate Leakage = " << proc_t.power.readOp.gate_leakage
        << " W" << std::endl;
    std::cout << "  Subthreshold Leakage with power gating = "
        << (long_channel
                ? proc_t.power.readOp.power_gated_with_long_channel_leakage
                : proc_t.power.readOp.power_gated_leakage)
        << " W" << std::endl;
    std::cout << "  Runtime Dynamic = " << proc_t.rt_power.readOp.dynamic
        << " W" << std::endl;
    std::cout << " total power = " << power
        << " W" << std::endl;
}

void
Mcpat::save_output(std::string fname, Processor &proc_t)
{
    std::ofstream out(fname);
    std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
    std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!
    proc_t.displayEnergy(2, 5);
    std::cout.rdbuf(coutbuf); //reset to standard output again
}

