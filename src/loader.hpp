/**
 * @file loader.hpp
 * @author François Jacobs
 * @date 2022-04-12
 *
 * @section kssroll_lic LICENSE
 *
 * The MIT License (MIT)
 *
 * @copyright Copyright © 2022 - François Jacobs
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the “Software”), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef LOADER_H_
#define LOADER_H_

#include <string>
#include <vector>

struct Track {
    uint8_t track_number = 0;
    // std::string filename; 
    int file_index;
    std::string track_name;
    uint32_t duration_sec = 0;
    uint32_t loop_time_sec_noimp = 0;
    std::string fade_noimp;

    bool duration_analysed = false;
    bool duration_detected = false;
    uint32_t duration_ms = 0;
    uint32_t samples_size = 0;
};

struct KssFileInfos {
    enum {NONE=-1, KSS, M3U};
    int file_type = NONE;
    std::vector<std::string> kss_files;
    std::vector<Track> tracks;
};

extern KssFileInfos get_file_infos(const std::string& filename);



#endif /* LOADER_H_ */
