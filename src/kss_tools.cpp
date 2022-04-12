/**
 * @file kss_tools.cpp
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
#include "kss_tools.hpp"
#include <kssplay.h>
#include <iostream>
#include <memory>
#include <vector>
#include <atomic>
#include <functional>
#include <thread>
#include <cstring>

static KSS* KSS_copy(KSS *kss)
{
	KSS *kss_copy {nullptr};
	if(kss)
	{
		kss_copy = KSS_new(kss->data, kss->size); 
		if(kss_copy)
		{
			kss_copy->type = kss->type;
			kss_copy->stop_detectable = kss->stop_detectable;
			kss_copy->loop_detectable = kss->loop_detectable;
//			kss_copy->title[KSS_TITLE_MAX]; /* the title */
//			kss_copy->idstr[8];             /* ID */
//			  uint8_t *data;                /* the KSS binary */
//			  uint32_t size;                /* the size of KSS binary */
//			  uint8_t *extra;               /* Infomation text for KSS info dialog */

			kss_copy->kssx = kss->kssx; /* 0:KSCC, 1:KSSX */
			kss_copy->mode = kss->mode; /* 0:MSX 1:SMS 2:GG */

			kss_copy->fmpac = kss->fmpac;
			kss_copy->fmunit = kss->fmunit;
			kss_copy->sn76489 = kss->sn76489;
			kss_copy->ram_mode = kss->ram_mode;
			kss_copy->msx_audio = kss->msx_audio;
			kss_copy->stereo = kss->stereo;
			kss_copy->pal_mode = kss->pal_mode;

			kss_copy->DA8_enable = kss->DA8_enable;

			kss_copy->load_adr = kss->load_adr;
			kss_copy->load_len = kss->load_len;
			kss_copy->init_adr = kss->init_adr;
			kss_copy->play_adr = kss->play_adr;

			kss_copy->bank_offset = kss->bank_offset;
			kss_copy->bank_num = kss->bank_num;
			kss_copy->bank_mode = kss->bank_mode;
			kss_copy->extra_size = kss->extra_size;
			kss_copy->device_flag = kss->device_flag;
			kss_copy->trk_min = kss->trk_min;
			kss_copy->trk_max = kss->trk_max;

			//  uint8_t vol[EDSC_MAX];
			std::memcpy(kss_copy->vol, kss->vol, sizeof(kss->vol));

			// info_num + KSSINFO pourraient être utilisé pour automatiser les transitions !?
			// kss_copy->info_num = kss->info_num;
			//KSSINFO *info;
		}
	}
	return kss_copy;
}

void kss_deleter(KSS *kss)
{
    // std::cout << "kss_deleter - ";
    if (kss)
    {
        KSS_delete(kss);
        // std::cout << "deleted\n";
    }
    // else
    //     std::cout << "nop\n";
}

void kssplay_deleter(KSSPLAY *kssplay)
{
    // std::cout << "kssplay_deleter - ";
    if (kssplay)
    {
        KSSPLAY_delete(kssplay);
        // std::cout << "deleted\n";
    }
    // else
    //     std::cout << "nop\n";
}

std::unique_ptr<KSSPLAY, decltype(&kssplay_deleter)> get_new_kssplay(uint32_t rate, uint32_t nch)
{
    return {KSSPLAY_new(rate, nch, 16), &kssplay_deleter};
}


// std::array<SongInfos, 255> detect_duration(const std::string &filename, const uint32_t silent_analysis_ms, const uint32_t max_duration_analysis_ms)
// {
//     std::array<SongInfos, 255> results;
//     constexpr int threads_count = 8;    
//     std::atomic_uint16_t song_test {0};

//     constexpr uint32_t channels = 1; // 2 ok mais inutile;
//     constexpr uint32_t rate = 22050; // 44100; // 11025;
//     const uint32_t max_silent_sample_count = static_cast<uint64_t>(silent_analysis_ms) * static_cast<uint64_t>(rate /* * channels*/) / 1000;
//     const uint32_t max_analysis_sample_count = static_cast<uint64_t>(max_duration_analysis_ms) * static_cast<uint64_t>(rate  /* * channels*/) / 1000;

//     constexpr uint32_t sample_count = 10000;
//     constexpr uint32_t autio_data_count = sample_count * channels;    

//     constexpr uint32_t cpu_speed = 0;
    
//     std::vector<std::unique_ptr<KSS, decltype(&kss_deleter)>> kss_list;
//     {
//         kss_list.emplace_back(KSS_load_file((char *) filename.c_str()), &kss_deleter);
//         if(!kss_list[0])
//             return results;

//         // prepare kss copies
//         for (auto i = 1; i < threads_count; ++i)
//             kss_list.emplace_back(KSS_copy(kss_list[0].get()), &kss_deleter);
//     }

//     auto ttt = [&](KSS *kss)
//     {
//         bool done = false;
//         auto kssplay = get_new_kssplay(rate, channels);
//         KSSPLAY_set_data(kssplay.get(), kss);
//         std::vector<int16_t> audio_data(autio_data_count);

//         while (!done)
//         {
//             uint16_t value = song_test++;
//             done = value > 254;
//             if (!done)
//             {
//                 auto &infos = results[value];
//                 infos.track = value + 1;
//                 KSSPLAY_reset(kssplay.get(), infos.track, cpu_speed);
//                 uint32_t current_sample_count = 0;
//                 bool proceed = true;

//                 while (proceed)
//                 {
//                     KSSPLAY_calc(kssplay.get(), audio_data.data(), sample_count);
//                     current_sample_count += sample_count;

//                     if (kssplay->silent >= max_silent_sample_count)
//                     {
//                         proceed = false;
//                         infos.sample_length = current_sample_count - kssplay->silent;
//                         infos.duration_detected = true;
//                     }

//                     if (proceed)
//                     {
//                         proceed = current_sample_count < max_analysis_sample_count;
//                         if (!proceed)
//                             infos.sample_length = current_sample_count;
//                     }
//                 }

//                 infos.duration_ms = static_cast<uint64_t>(infos.sample_length) * 1000 / rate;
//             }
//         }
//     };

//     std::vector<std::thread> runners;
//     for (int i = 0; i < threads_count; ++i)
//         runners.emplace_back(ttt, kss_list[i].get());

//     for (int i = 0; i < threads_count; ++i)
//         runners[i].join();

//     for(auto i = 0; i < results.size(); ++i)
//     {
//        // std::cout << i << " : " << results[i][0] << " " << results[i][1] << " " << results[i][2] << "\n";
//        SongInfos &infos = results[i];
//        if (infos.duration_detected)
//        {
//            if (infos.duration_ms > 10 /* infos.sample_length */) // ou duration_ms > 10
//                std::cout << " trck:" << static_cast<int>(infos.track) << " l:" << infos.sample_length << " d:" << infos.duration_ms << " ms\n";
//        }
//        else
//            std::cout << " trck:" << static_cast<int>(infos.track) << " l:" << infos.sample_length << " d:" << infos.duration_ms <<" ms - infinite !\n";
//     }
//     return results;
// }

// bool detect_duration(KssFileInfos &file_infos, const uint32_t silent_analysis_ms, const uint32_t max_duration_analysis_ms) 
// {
//     constexpr uint32_t channels = 1; 
//     constexpr uint32_t rate = 11025; 
//     constexpr uint32_t sample_count = 10000;
//     constexpr uint32_t autio_data_count = sample_count * channels;    
//     constexpr uint32_t cpu_speed = 0;
//     constexpr int threads_count = 8;    

//     const uint32_t max_silent_sample_count = static_cast<uint64_t>(silent_analysis_ms) * static_cast<uint64_t>(rate /* * channels*/) / 1000;
//     const uint32_t max_analysis_sample_count = static_cast<uint64_t>(max_duration_analysis_ms) * static_cast<uint64_t>(rate  /* * channels*/) / 1000;

//     if(file_infos.file_type == KssFileInfos::NONE)
//         return false;

//     std::atomic_uint16_t song_test {0};
//     const auto songs_count = file_infos.tracks.size();

//     if(!songs_count)
//         return true;
    
//     // prepare copies
//     std::vector<std::vector<std::unique_ptr<KSS, decltype(&kss_deleter)>>> kss_lists;
//     {
//         for(auto file_num = 0; file_num < file_infos.kss_files.size(); ++ file_num)
//         {
//             auto &kss_list = kss_lists.emplace_back();
//             kss_list.emplace_back(KSS_load_file((char *) file_infos.kss_files[file_num].c_str()), &kss_deleter);
//             if(!kss_list[0])
//                 return false;

//             // prepare kss copies
//             for (auto i = 1; i < threads_count; ++i)
//                 kss_list.emplace_back(KSS_copy(kss_list[0].get()), &kss_deleter);    
//         }
//     }

//     auto ttt = [&](int id)
//     {
//         bool done = false;
//         auto kssplay = get_new_kssplay(rate, channels);
//         std::vector<int16_t> audio_data(autio_data_count);

//         while (!done)
//         {
//             uint16_t value = song_test++;
//             done = value >= songs_count;
//             if (!done)
//             {
//                 auto &infos = file_infos.tracks[value];

//                 // init KSSPLAY
//                 KSSPLAY_set_data(kssplay.get(), kss_lists[infos.file_index][id].get());
//                 KSSPLAY_reset(kssplay.get(), infos.track_number, cpu_speed);
//                 uint32_t current_sample_count = 0;
//                 bool proceed = true;

//                 while (proceed)
//                 {
//                     KSSPLAY_calc(kssplay.get(), audio_data.data(), sample_count);
//                     current_sample_count += sample_count;

//                     if (kssplay->silent >= max_silent_sample_count)
//                     {
//                         proceed = false;
//                         infos.samples_size = current_sample_count - kssplay->silent;
//                         infos.duration_detected = true;
//                     }

//                     if (proceed)
//                     {
//                         proceed = current_sample_count < max_analysis_sample_count;
//                         if (!proceed)
//                             infos.samples_size = current_sample_count;
//                     }
//                 }

//                 infos.duration_ms = static_cast<uint64_t>(infos.samples_size) * 1000 / rate;
//                 infos.duration_analysed = true;
//             }
//         }
//     };

//     std::vector<std::thread> runners;
//     for (int i = 0; i < threads_count; ++i)
//         runners.emplace_back(ttt, i);

//     for (int i = 0; i < threads_count; ++i)
//         runners[i].join();

//     return true;
// }

DurationDetection::~DurationDetection()
{
    if(worker.joinable())
        worker.join();
}

bool DurationDetection::operator()(KssFileInfos &file_infos, const uint32_t silent_analysis_ms, const uint32_t max_duration_analysis_ms)
{
    std::lock_guard<std::mutex> lock(mutex);   
    if(file_infos.file_type == KssFileInfos::NONE || !file_infos.tracks.size())
        return false;

    if(worker.joinable())
        return false;
    
    this->file_infos = file_infos;
    success = false;
    running = true;

    song_test  = 0;
    songs_count = file_infos.tracks.size();


    worker = std::thread(std::bind(&DurationDetection::detect_duration, this, silent_analysis_ms, max_duration_analysis_ms));
    return true;
}


void DurationDetection::detect_duration(const uint32_t silent_analysis_ms, const uint32_t max_duration_analysis_ms) 
{
    // std::lock_guard<std::mutex> lock(mutex);  
    constexpr uint32_t channels = 1; 
    constexpr uint32_t rate = 11025; 
    constexpr uint32_t sample_count = 10000;
    constexpr uint32_t autio_data_count = sample_count * channels;    
    constexpr uint32_t cpu_speed = 0;
    constexpr int threads_count = 8;    

    const uint32_t max_silent_sample_count = static_cast<uint64_t>(silent_analysis_ms) * static_cast<uint64_t>(rate /* * channels*/) / 1000;
    const uint32_t max_analysis_sample_count = static_cast<uint64_t>(max_duration_analysis_ms) * static_cast<uint64_t>(rate  /* * channels*/) / 1000;

    
    // prepare copies
    std::vector<std::vector<std::unique_ptr<KSS, decltype(&kss_deleter)>>> kss_lists;
    {
        for(std::size_t file_num = 0; file_num < file_infos.kss_files.size(); ++ file_num)
        {
            auto &kss_list = kss_lists.emplace_back();
            kss_list.emplace_back(KSS_load_file((char *) file_infos.kss_files[file_num].c_str()), &kss_deleter);
            if(!kss_list[0])
            {
                running = false;
                return;
            }

            // prepare kss copies
            for (auto i = 1; i < threads_count; ++i)
                kss_list.emplace_back(KSS_copy(kss_list[0].get()), &kss_deleter);    
        }
    }

    auto ttt = [&](int id)
    {
        bool done = false;
        auto kssplay = get_new_kssplay(rate, channels);
        std::vector<int16_t> audio_data(autio_data_count);

        while (!done && !abort_flag)
        {
            uint16_t value = song_test++;
            done = value >= songs_count;
            if (!done)
            {
                auto &infos = file_infos.tracks[value];

                // init KSSPLAY
                KSSPLAY_set_data(kssplay.get(), kss_lists[infos.file_index][id].get());
                KSSPLAY_reset(kssplay.get(), infos.track_number, cpu_speed);
                uint32_t current_sample_count = 0;
                bool proceed = true;

                while (proceed && !abort_flag)
                {
                    KSSPLAY_calc(kssplay.get(), audio_data.data(), sample_count);
                    current_sample_count += sample_count;

                    if (kssplay->silent >= max_silent_sample_count)
                    {
                        proceed = false;
                        infos.samples_size = current_sample_count - kssplay->silent;
                        infos.duration_detected = true;
                    }

                    if (proceed)
                    {
                        proceed = current_sample_count < max_analysis_sample_count;
                        if (!proceed)
                            infos.samples_size = current_sample_count;
                    }
                }

                infos.duration_ms = static_cast<uint64_t>(infos.samples_size) * 1000 / rate;
                infos.duration_analysed = !abort_flag;
            }
        }
    };

    std::vector<std::thread> runners;
    for (int i = 0; i < threads_count; ++i)
        runners.emplace_back(ttt, i);

    for (int i = 0; i < threads_count; ++i)
        runners[i].join();

    success = !abort_flag;
    running = false;
}

float DurationDetection::get_progress()
{
    int count = songs_count;
    int current = song_test;
    if(!running || !count)
        return 1.0f;

    float value = static_cast<float>(current) / static_cast<float>(count);
    if(value > 1.0f)
        value = 1.0f;
    
    return value;
}

bool DurationDetection::is_success()
{
    return success;
}
bool DurationDetection::is_running()
{
    return running;
}

KssFileInfos DurationDetection::get_results()
{
    if(!running && success)
    {
        if(worker.joinable())
            worker.join();
        return file_infos;
    }
    return KssFileInfos{};
}

void DurationDetection::abort()
{
    abort_flag = true;
    if(worker.joinable())
        worker.join();
    abort_flag = false;
}
