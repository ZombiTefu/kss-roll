/**
 * @file kss_tools.hpp
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
#ifndef KSS_TOOLS_H_
#define KSS_TOOLS_H_

#include <cstdint>
#include "loader.hpp"
#include <atomic>
#include <thread>
#include <mutex>

struct SongInfos
{
    uint8_t track = 0;
    uint32_t sample_length = 0; // duree en nb sample
    uint32_t duration_ms = 0;
    bool duration_detected = false;
};

//std::array<SongInfos, 255> detect_duration(const std::string &filename, const uint32_t silent_analysis_ms = 5000, const uint32_t max_duration_analysis_ms = 300000);
// bool detect_duration(KssFileInfos &file_infos, const uint32_t silent_analysis_ms = 5000, const uint32_t max_duration_analysis_ms = 300000);

class DurationDetection
{
    std::mutex mutex;
    std::atomic_uint16_t song_test{0};
    uint16_t songs_count{0};
    std::atomic_bool success {false};
    std::atomic_bool running {false};
    std::atomic_bool abort_flag {false};

    std::thread worker;
    KssFileInfos file_infos;

    void detect_duration(const uint32_t silent_analysis_ms, const uint32_t max_duration_analysis_ms);

public:
    ~DurationDetection();
    bool operator()(KssFileInfos &file_infos, const uint32_t silent_analysis_ms = 5000, const uint32_t max_duration_analysis_ms = 300000);
    float get_progress();
    bool is_success();
    bool is_running();
    KssFileInfos get_results();
    void abort();

};

#endif