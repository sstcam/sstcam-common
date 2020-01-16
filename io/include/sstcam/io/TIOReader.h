// Copyright 2019 Cherenkov Telescope Array Observatory
// This software is distributed under the terms of the BSD-3-Clause license.

#ifndef SSTCAM_IO_TIOREADER_H_
#define SSTCAM_IO_TIOREADER_H_

//#include "sstcam/interfaces/WaveformDataPacket.h"
#include <string>
#include <fitsio.h>


namespace sstcam {
namespace io {

class TIOReader {
public:
    explicit TIOReader(std::string path);

    void Close();

    std::string GetPath() {return path_;}

private:
    std::string path_;
    fitsfile* fits_;
    uint8_t n_event_headers_;
    uint32_t n_packets_per_event_;
//    uint16_t packet_size_;



};

}
}


#endif //SSTCAM_IO_TIOREADER_H_
