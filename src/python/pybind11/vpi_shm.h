/*
 * Copyright (c) 2020 Andrew Smith
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

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
    double curr_load;
    double prediction;
    uint32_t enable;
    uint32_t time_to_next; // Time to next sync event in picoseconds
    uint32_t sim_over;
} v_incoming_signals;

typedef struct {
    double curr_v;
    double curr_i;
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

void set_driver_signals(double load, uint32_t term);

void set_prediction(double prediction);

void set_freq(double freq, int cycles);

void set_voltage_set(double v_set);

double get_voltage();

double get_current();

void ack_supply();

} // namespace vpi_shm

#endif
