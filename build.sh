#!/bin/bash

scons -j32 ./build/X86/gem5.debug
scons -j32 ./build/X86/gem5.opt
#scons -j32 ./build/X86_MESI_Three_Level/gem5.debug RUBY=True PROTOCOL=MESI_Three_Level SLICC_HTML=True
#scons -j32 ./build/X86_MESI_Three_Level/gem5.opt RUBY=True PROTOCOL=MESI_Three_Level SLICC_HTML=True
