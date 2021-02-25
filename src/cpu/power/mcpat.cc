
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
#include <chrono> 


std::unordered_map<std::string, Stats::Info*> Mcpat::name_to_stat;
std::unordered_set<std::string> Mcpat::stat_names;

Mcpat::Mcpat(PPredUnit* _powerPred){
    powerPred = _powerPred;
}


void
Mcpat::init(std::string xml_dir){
    xml = new ParseXML();
    xml->parse(xml_dir);
    proc.init(xml);

    //TODO make this compile time
    for(std::unordered_map<std::string, int>::iterator it = stat_map.begin(); it != stat_map.end(); ++it) {
        stat_names.insert(it->first);
    }


    list<Stats::Info *>& statlist = Stats::statsList();
    list<Stats::Info *>::iterator it;
    for (it = statlist.begin(); it != statlist.end(); ++it){
        std::string name = (*it)->name;
        if (stat_names.find(name) != stat_names.end())
            name_to_stat[name] = (*it);
    }

    // new stats
    Root* root = Root::root();
    init_stat_map_helper(root, "");

}

void
Mcpat::init_stat_map_helper(Stats::Group* group, std::string path){
    const std::vector< Stats::Info * >&  stats = group->getStats();

    std::vector<Stats::Info *>::const_iterator it;
    for (it = stats.begin(); it != stats.end(); ++it){
        std::string name = path + (*it)->name;
        if (stat_names.find(name) != stat_names.end())
            name_to_stat[name] = (*it);
    }
    
    const std::map< std::string, Stats::Group * > & childGroups = group->getStatGroups();    
    for (auto const& x : childGroups){
        std::string child_path = path + x.first + ".";
        init_stat_map_helper(x.second, child_path);
    }
}


void
Mcpat::init_wrapper(std::string xml_dir, std::string output_path){
    // Processor proc_serial;
    // std::string serial = output_path + "/mcpat_serial.txt";
    // std::ifstream ifs(serial.c_str());
    // if (ifs.good()) {
    //     boost::archive::text_iarchive ia(ifs);
    //     ia >> proc_serial;
    // } else {
    //     std::cerr << "Archive " << serial << " cannot be used\n";
    //     assert(false);
    // }
    // proc_serial_xml = new ParseXML();
    // proc_serial_xml->parse(xml_dir);
    // proc_serial.init(proc_serial_xml,true);

    update_stats();
    proc.reset();
    proc.compute();
    power = get_power(proc);

    // std::cout<<"mcpat internal:" << std::endl;
    // print_power(proc);
    // std::string output_path_internal = output_path + "/out_mcpat_internal.txt";
    // save_output(output_path_internal, proc);
    // proc.XML->print();
    // std::cout<<"mcpat proc_serial:" << std::endl;
    // print_power(proc_serial);
    // std::string output_path_serial = output_path + "/out_mcpat_serial.txt";
    // save_output(output_path_serial, proc_serial);

}
void 
Mcpat::run_with_xml(std::string xml_dir, std::string output_path){

    proc_serial_xml = new ParseXML();
    proc_serial_xml->parse(xml_dir);
    proc.XML = proc_serial_xml;
    proc.reset();
    proc.compute();
    power = get_power(proc);

}


void
Mcpat::update_stats(){
    for(std::unordered_map<std::string, Stats::Info*>::iterator it = name_to_stat.begin(); it != name_to_stat.end(); ++it) {
        it->second->prepare();
        set_mcpat_stat(it->second, it->first);
    }

    //stats with >1 dependencies
    proc.XML->sys.core[0].busy_cycles = 
        proc.XML->sys.core[0].total_cycles - 
        proc.XML->sys.core[0].idle_cycles;

    proc.XML->sys.core[0].rename_writes = 
        (stat_storage.cpu_rename_RenamedOperands - stat_storage_prev.cpu_rename_RenamedOperands) * 
        (proc.XML->sys.core[0].rename_reads) / 
        ((stat_storage.cpu_rename_RenameLookups - stat_storage_prev.cpu_rename_RenameLookups)+1);

    proc.XML->sys.core[0].fp_rename_writes = 
        (stat_storage.cpu_rename_RenamedOperands - stat_storage_prev.cpu_rename_RenamedOperands) * 
        (proc.XML->sys.core[0].fp_rename_reads) /
        ((stat_storage.cpu_rename_RenameLookups - stat_storage_prev.cpu_rename_RenameLookups)+1);
        
    proc.XML->sys.L2->read_accesses = 
        (stat_storage.l2_ReadExReq_accesses - stat_storage_prev.l2_ReadExReq_accesses) + 
        (stat_storage.l2_ReadCleanReq_accesses - stat_storage_prev.l2_ReadCleanReq_accesses) + 
        (stat_storage.l2_ReadSharedReq_accesses - stat_storage_prev.l2_ReadSharedReq_accesses);

    proc.XML->sys.L2->read_misses = 
        (stat_storage.l2_ReadCleanReq_misses - stat_storage_prev.l2_ReadCleanReq_misses) + 
        (stat_storage.l2_ReadExReq_misses - stat_storage_prev.l2_ReadExReq_misses);

    proc.XML->sys.L2->write_accesses =
        (stat_storage.l2_WritebackDirty_accesses - stat_storage_prev.l2_WritebackDirty_accesses) + 
        (stat_storage.l2_WritebackClean_accesses - stat_storage_prev.l2_WritebackClean_accesses);

    proc.XML->sys.L2->write_misses =
        (stat_storage.l2_WritebackClean_accesses - stat_storage_prev.l2_WritebackClean_accesses) -
        (stat_storage.l2_WritebackDirty_hits - stat_storage_prev.l2_WritebackDirty_hits);

    proc.XML->sys.L3->read_accesses = 
        (stat_storage.l3_ReadExReq_accesses - stat_storage_prev.l3_ReadExReq_accesses) + 
        (stat_storage.l3_ReadCleanReq_accesses - stat_storage_prev.l3_ReadCleanReq_accesses) + 
        (stat_storage.l3_ReadSharedReq_accesses - stat_storage_prev.l3_ReadSharedReq_accesses);

    proc.XML->sys.L3->read_misses = 
        (stat_storage.l3_ReadCleanReq_misses - stat_storage_prev.l3_ReadCleanReq_misses) + 
        (stat_storage.l3_ReadExReq_misses - stat_storage_prev.l3_ReadExReq_misses);

    proc.XML->sys.L3->write_accesses =
        (stat_storage.l3_WritebackDirty_accesses - stat_storage_prev.l3_WritebackDirty_accesses) + 
        (stat_storage.l3_WritebackClean_accesses - stat_storage_prev.l3_WritebackClean_accesses);

    proc.XML->sys.L3->write_misses =
        (stat_storage.l3_WritebackClean_accesses - stat_storage_prev.l3_WritebackClean_accesses) -
        (stat_storage.l3_WritebackDirty_hits - stat_storage.l3_WritebackDirty_hits);
    
    proc.XML->sys.NoC[0].total_accesses =
        (stat_storage.membus_pkt_count - stat_storage_prev.membus_pkt_count) + 
        (stat_storage.tol2bus_pkt_count - stat_storage_prev.tol2bus_pkt_count) + 
        (stat_storage.tol3bus_pkt_count - stat_storage_prev.tol3bus_pkt_count); 


}



void
Mcpat::reset(){ 
    // for(std::unordered_map<std::string, Stats::Info*>::iterator it = name_to_stat.begin(); it != name_to_stat.end(); ++it) {
    //     it->second->reset();
    // }
    proc.reset();
}

void
Mcpat::print_power(Processor &proc_t){
    bool long_channel = xml->sys.longer_channel_device;
    double gate_leakage = proc_t.power.readOp.gate_leakage;
    double sub_leakage = long_channel ? proc_t.power.readOp.power_gated_with_long_channel_leakage
                : proc_t.power.readOp.power_gated_leakage;
    double runtime_dynamic = proc_t.rt_power.readOp.dynamic;
    double power = gate_leakage + sub_leakage + runtime_dynamic;

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
Mcpat::print_power(){
    print_power(proc);
}


double
Mcpat::get_power(Processor &proc_t){
    bool long_channel = xml->sys.longer_channel_device;
    double gate_leakage = proc_t.power.readOp.gate_leakage;
    double sub_leakage = long_channel ? proc_t.power.readOp.power_gated_with_long_channel_leakage
                : proc_t.power.readOp.power_gated_leakage;
    double runtime_dynamic = proc_t.rt_power.readOp.dynamic;

    return gate_leakage + sub_leakage + runtime_dynamic;
}

void
Mcpat::save_output(std::string fname, Processor &proc_t){
    std::ofstream out(fname);
    std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
    std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!
    proc_t.displayEnergy(2, 5);
    std::cout.rdbuf(coutbuf); //reset to standard output again
}

void
Mcpat::save_output(std::string output_path){
    std::string output_path_internal = output_path + "/out_mcpat_internal.txt";
    save_output(output_path_internal, proc);
}


