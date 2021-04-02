#include "cpu/power/predictors/IdealSensorHarvardMitigation.hh"

#include <algorithm>
#include <cassert>
#include <cstdlib>

#include "arch/isa_traits.hh"
#include "arch/types.hh"
#include "arch/utility.hh"
#include "base/trace.hh"
#include "config/the_isa.hh"
#include "debug/HarvardPowerPred.hh"
#include "python/pybind11/vpi_shm.h"
#include "sim/stat_control.hh"

IdealSensorHarvardMitigation::IdealSensorHarvardMitigation(const Params *params): 
  PPredUnit(params),
  throttle_duration(params->throttle_duration),
  threshold(params->threshold)
{
    cycles_since_pred = 0;
    table.resize(params->table_size, params->signature_length);
    history.resize(params->signature_length);
  
    throttle_on_restore = params->throttle_on_restore;
    hysteresis = params->hysteresis;
    t_count = 0;
    e_count = 0;

}

void
IdealSensorHarvardMitigation::regStats(){
    PPredUnit::regStats();
}

void
IdealSensorHarvardMitigation::tick(void){

  table.tick();
  
  bool ve = false;
  bool prediction_harvard = false;
  bool prediction_idealsensor = false;

  if (hr_updated) {
    PPred::Entry snapshot = history.get_entry();
    if (table.find_variable_signature_len(snapshot)){
      DPRINTF(HarvardPowerPred, "PRED HIGH:  row=%4d: %s\n", table.last_find_index, table[table.last_find_index].to_str().c_str());
      if (table[table.last_find_index].delay == 0){
        prediction_harvard = true;
        total_pred_action++;
        cycles_since_pred = 0;
        DPRINTF(HarvardPowerPred, "            snapshot: %s\n", snapshot.to_str().c_str());
        DPRINTF(HarvardPowerPred, "     HistoryRegister: %s\n", history.to_str().c_str());
      }
      else{
        prediction_delay.push_back(table[table.last_find_index].delay);
        DPRINTF(HarvardPowerPred, "             Delayed: %d\n", table[table.last_find_index].delay);
      }
    }
    else {
      total_pred_inaction++;
    }
    hr_updated = false;
    total_preds++;
  }

  auto it = prediction_delay.begin();
  while (it != prediction_delay.end()){
    (*it)--;
    if (*it <= 0){
      prediction_harvard = true; 
      DPRINTF(HarvardPowerPred, "PRED HIGH on delay\n");
      cycles_since_pred = 0;
      it = prediction_delay.erase(it);
    }
    else{
      it++;
    }
  }

  //IdealSensor
  if (supply_voltage < threshold && supply_voltage_prev > threshold){
    prediction_idealsensor = true; 
  } 
  //IdealSensor

  if (supply_voltage < emergency && supply_voltage_prev > emergency){
    ve = true;
  } 

  bool prediction = prediction_harvard || prediction_idealsensor;
  update_stats(prediction, ve);

  if (ve) {
    if (is_ve_missed()){
      int events_to_drop_var = 0;
      int lead_time_cnt = 0;
      for (int i=0; i<history.entry_head_time.size(); i++){
        lead_time_cnt += history.entry_head_time[i];
        if (lead_time_cnt > LEAD_TIME_MIN){
          events_to_drop_var = i;
          break;
        }
      }
      PPred::Entry snapshot = history.get_entry_drop_front(events_to_drop_var);

      int delay = (lead_time_cnt > LEAD_TIME_CAP)?(lead_time_cnt-LEAD_TIME_MIN+1):0;
      snapshot.delay = delay;
      
      int index = table.insert(snapshot);

      DPRINTF(HarvardPowerPred, "VE MISSED:     row=%4d: %s\n", index, table[index].to_str().c_str());
      DPRINTF(HarvardPowerPred, "     cycles since pred: %d\n", cycles_since_pred);
      DPRINTF(HarvardPowerPred, "              snapshot: %s\n", snapshot.to_str().c_str());
      DPRINTF(HarvardPowerPred, "         event_dropped: %d\n", events_to_drop_var);
      DPRINTF(HarvardPowerPred, "                 Delay: %d\n", delay);
      DPRINTF(HarvardPowerPred, "       HistoryRegister: %s\n", history.to_str().c_str());

    }
    else{
      DPRINTF(HarvardPowerPred, "VE CAUGHT: pred %d cycles ago\n", cycles_since_pred);
    }
  }

  cycles_since_pred++;
  history.tick();

  return;
}

IdealSensorHarvardMitigation*
IdealSensorHarvardMitigationParams::create(){
  return new IdealSensorHarvardMitigation(this);
}


