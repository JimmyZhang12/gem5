#include <deque>
#include <unordered_set>

#include "base/statistics.hh"
#include "base/types.hh"
#include "cpu/inst_seq.hh"
#include "cpu/power/ppred_unit.hh"
#include "cpu/static_inst.hh"
#include "params/IdealSensor.hh"
#include "sim/probe/pmu.hh"
#include "sim/sim_object.hh"

class IdealSensor : public PPredUnit
{
  public:
    typedef IdealSensorParams Params;

    /**
     * @param params The params object, that has the size of the BP and BTB.
     */
    IdealSensor(const Params *p);

    /**
     * Registers statistics.
     */
    void regStats() override;

    /**
     * Update the IdealSensor State Machine.
     */
    void tick(void);
    int voltage_to_bucket(float voltage);


  protected:
    float threshold;
    float voltage_min;
    float voltage_max;
    int num_buckets;

  private:
    float bucket_len;
    std::deque<int> voltage_history;

    struct deque_int_hash {
      size_t operator()(const std::deque<int>& v) const {
            std::hash<int> hasher;
            size_t seed = 8562562527;
            for (int i : v) {
                seed ^= hasher(i) + 0x9e3779b9 + (seed<<6) + (seed>>2);
            }
            return seed;
        }
    };
    typedef std::unordered_set<std::deque<int>, deque_int_hash> set_vhistory;
    set_vhistory voltage_history_table;
};

