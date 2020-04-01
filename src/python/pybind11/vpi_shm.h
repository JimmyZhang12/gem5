#ifndef VPI_SHM_HH
#define VPI_SHM_HH
#include <fcntl.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>

#define NEW_DATA    1
#define NO_NEW_DATA 0

#define SHM_LENGTH  sizeof(mapped)

#define INIT        1
#define NINIT       0

namespace vpi_shm {

typedef struct {
    double v_set;
    double curr_r_load;
    uint32_t sim_over;
} v_incoming_signals;

typedef struct {
    double curr_v;
    uint32_t sim_done;
} p_incoming_signals;

typedef struct {
    sem_t sem;
    uint8_t new_data;
    v_incoming_signals data;
} p_to_v;

typedef struct {
    sem_t sem;
    uint8_t new_data;
    p_incoming_signals data;
} v_to_p;

typedef struct {
    // Process to Verilog Sim
    p_to_v pv;
    // Verilog Sim to Process
    v_to_p vp;
} mapped;

int create_shm(int process, char* name);

void destroy_shm();

void set_driver_signals(double v_set, double r, uint32_t term);

double get_voltage();

} // namespace vpi_shm

#endif
