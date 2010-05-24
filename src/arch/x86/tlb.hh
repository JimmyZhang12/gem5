/*
 * Copyright (c) 2007 The Hewlett-Packard Development Company
 * All rights reserved.
 *
 * The license below extends only to copyright in the software and shall
 * not be construed as granting a license to any other intellectual
 * property including but not limited to intellectual property relating
 * to a hardware implementation of the functionality of the software
 * licensed hereunder.  You may use the software subject to the license
 * terms below provided that you ensure that this notice is replicated
 * unmodified and in its entirety in all distributions of the software,
 * modified or unmodified, in source code or in binary form.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Gabe Black
 */

#ifndef __ARCH_X86_TLB_HH__
#define __ARCH_X86_TLB_HH__

#include <list>
#include <vector>
#include <string>

#include "arch/x86/pagetable.hh"
#include "arch/x86/segmentregs.hh"
#include "config/full_system.hh"
#include "mem/mem_object.hh"
#include "mem/request.hh"
#include "params/X86TLB.hh"
#include "sim/faults.hh"
#include "sim/tlb.hh"
#include "sim/sim_object.hh"

class ThreadContext;
class Packet;

namespace X86ISA
{
    class Walker;

    class TLB : public BaseTLB
    {
      protected:
        friend class Walker;

        typedef std::list<TlbEntry *> EntryList;

        uint32_t configAddress;

      public:

        typedef X86TLBParams Params;
        TLB(const Params *p);

        void dumpAll();

        TlbEntry *lookup(Addr va, bool update_lru = true);

        void setConfigAddress(uint32_t addr);

      protected:

        EntryList::iterator lookupIt(Addr va, bool update_lru = true);

#if FULL_SYSTEM
      protected:

        Walker * walker;
#endif

      public:
        void invalidateAll();

        void invalidateNonGlobal();

        void demapPage(Addr va, uint64_t asn);

      protected:
        int size;

        TlbEntry * tlb;

        EntryList freeList;
        EntryList entryList;

        Fault translateInt(RequestPtr req, ThreadContext *tc);

        Fault translate(RequestPtr req, ThreadContext *tc,
                Translation *translation, Mode mode,
                bool &delayedResponse, bool timing);

      public:

        Fault translateAtomic(RequestPtr req, ThreadContext *tc, Mode mode);
        void translateTiming(RequestPtr req, ThreadContext *tc,
                Translation *translation, Mode mode);

#if FULL_SYSTEM
        Tick doMmuRegRead(ThreadContext *tc, Packet *pkt);
        Tick doMmuRegWrite(ThreadContext *tc, Packet *pkt);
#endif

        TlbEntry * insert(Addr vpn, TlbEntry &entry);

        // Checkpointing
        virtual void serialize(std::ostream &os);
        virtual void unserialize(Checkpoint *cp, const std::string &section);
    };
}

#endif // __ARCH_X86_TLB_HH__
