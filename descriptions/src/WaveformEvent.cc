// Copyright 2020 Cherenkov Telescope Array Observatory
// This software is distributed under the terms of the BSD-3-Clause license.

#include "sstcam/descriptions/WaveformEvent.h"

namespace sstcam {
namespace descriptions {

void GetHardcodedModuleSituation(std::set<uint8_t>& active_modules,
        size_t& n_pixels, uint8_t& first_active_module_slot) {
    first_active_module_slot = *active_modules.begin();
    size_t n_active_modules = active_modules.size();
    size_t n_modules;

    // Hardcoded n_module situations
    if (n_active_modules == 1) {
        n_modules = 1; // Single module case
    }
    else if (n_active_modules > 1 && n_active_modules <= 32) {
        n_modules = 32; // SST camera case
        first_active_module_slot = 0;
    }
    else {
        n_modules = n_active_modules;
        std::cout << "WARNING: No case set up for files with N modules:"
        << n_active_modules << std::endl;
    }
    n_pixels = n_modules * N_PIXELS_PER_MODULE;
}

}}
