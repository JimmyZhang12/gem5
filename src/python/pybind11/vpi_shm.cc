#include "pybind11/pybind11.h"
#include "pybind11/stl.h"

#include <semaphore.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdio>

#include "base/statistics.hh"
#include "base/stats/text.hh"
#include "sim/stat_control.hh"
#include "sim/stat_register.hh"
#include "vpi_shm.h"

namespace py = pybind11;

namespace vpi_shm {

int shm_fd = 0;

mapped* shm_ptr = NULL;

char name_buff[256] = "";

void
init_shm(mapped* p)
{
    sem_init(&p->pv.sem, 1, 1);
    p->pv.new_data = NO_NEW_DATA;
    p->pv.data.v_set = 0;
    p->pv.data.curr_r_load = 0;
    p->pv.data.sim_over = 0;

    sem_init(&p->vp.sem, 1, 1);
    p->vp.new_data = NO_NEW_DATA;
    p->vp.data.curr_v = 0;
    p->vp.data.sim_done = 0;
}

int
create_shm(int should_init, char* name)
{
    if (name == NULL)
    {
        fprintf(stderr,"bad pointer to name\n");
        exit(-1);
    }
    strncpy(name_buff, name, 256);
    if (should_init == INIT)
    {
        shm_fd = shm_open(name_buff, O_CREAT | O_RDWR, 0666);
        if (shm_fd == -1)
        {
            fprintf(stderr,"Failed to create & open /dev/shm/%s\n", name_buff);
            exit(-1);
        }
        if (ftruncate(shm_fd, SHM_LENGTH) != 0)
        {
            fprintf(stderr,"ftruncate failed\n");
            shm_unlink(name_buff);
            exit(-1);
        }
        shm_ptr = (mapped*)mmap(NULL, SHM_LENGTH, PROT_WRITE | PROT_READ,
                                MAP_SHARED, shm_fd, 0);
        if (shm_ptr == NULL)
        {
            fprintf(stderr,"mmap failed\n");
            shm_unlink(name_buff);
            exit(-1);
        }
        // Initialize Data:
        init_shm(shm_ptr);
    }
    else
    {
        shm_fd = shm_open(name_buff, O_RDWR, 0666);
        if (shm_fd == -1)
        {
            fprintf(stderr,"Failed to open /dev/shm/%s\n", name_buff);
            exit(-1);
        }
        shm_ptr = (mapped*)mmap(NULL, SHM_LENGTH, PROT_WRITE | PROT_READ,
                                MAP_SHARED, shm_fd, 0);
        if (shm_ptr == NULL)
        {
            fprintf(stderr,"mmap failed\n");
            shm_unlink(name_buff);
            exit(-1);
        }
    }
    return 0;
}

void
destroy_shm()
{
    munmap(shm_ptr, SHM_LENGTH);
    shm_unlink(name_buff);
}

double
get_voltage()
{
    double ret = 0;
    while (1)
    {
        sem_wait(&shm_ptr->vp.sem);
        if (shm_ptr->vp.new_data == NEW_DATA)
        {
            ret = shm_ptr->vp.data.curr_v;
            shm_ptr->vp.new_data = NO_NEW_DATA;
            sem_post(&shm_ptr->vp.sem);
            return ret;
        }
        sem_post(&shm_ptr->vp.sem);
    }
    return ret;
}

void
set_driver_signals(double v_set, double r, uint32_t term)
{
    // Wait for the verilog simulation to consume the previous data:
    while (1)
    {
        sem_wait(&shm_ptr->pv.sem);
        if (shm_ptr->pv.new_data == NO_NEW_DATA)
        {
            //printf("Sending V:%lf R:%lf\n", voltage_setpoint, resistance);
            shm_ptr->pv.data.v_set = v_set;
            shm_ptr->pv.data.curr_r_load = r;
            shm_ptr->pv.data.sim_over = term;
            shm_ptr->pv.new_data = NEW_DATA;
            sem_post(&shm_ptr->pv.sem);
            return;
        }
        sem_post(&shm_ptr->pv.sem);
    }
}

} // namespace vpi_shm

void
pybind_init_vpi_shm(py::module &m_native)
{
    py::module m = m_native.def_submodule("vpi_shm");
    m.def("create_shm", &vpi_shm::create_shm,
          "Create/Open and Map a shared mem region");
    m.def("destroy_shm", &vpi_shm::destroy_shm,
          "Destroy and UnMap a shared mem region");
    m.def("set_driver_signals", &vpi_shm::set_driver_signals,
          "Send voltage and resistance to simulation");
    m.def("get_voltage", &vpi_shm::get_voltage,
          "Get the instantaneous voltage value from the sim");
}
