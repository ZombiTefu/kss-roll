/**
 * @file loader.cpp
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

#include "loader.hpp"
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <filesystem>
#include <gtkmm.h>
#include <majimix/majimix.hpp>
#include <kss/kss.h>

static std::string to_lower(std::string s)
{
	transform(s.begin(), s.end(), s.begin(), ::tolower);
	return s;
}

static std::string find_file(const std::filesystem::path& parent_path, const std::string& filename) 
{
	std::filesystem::directory_entry file {parent_path / filename};
	if(file.exists() && file.is_regular_file())
		return file.path();

	std::string filename_lowercase = to_lower(filename);
	for (auto const &f : std::filesystem::directory_iterator{parent_path})
    {
		if(f.is_regular_file() && filename_lowercase == to_lower(f.path()))
			return f.path();
	}
	return "";
}

static int get_number(const std::string& n) 
{
	static std::regex rnum {"(?:\\s*)(\\$*[a-fA-F\\d]+)"};
	std::smatch sm;
	if (regex_search(n, sm, rnum))
	{
		std::string value = sm[1];
		int base = 10;
		if(value[0] == '$')
		{
			base = 16;
			value = value.substr(1);
		}
		return std::stoi(value, nullptr, base);
	}
	return 0;
}

// hh:mm:ss
static uint32_t to_duration_sec(const std::string& d) {
	std::istringstream iss(d);
	std::string value;
	uint32_t v = 0;
	while (std::getline(iss, value, ':'))
		v = v * 60 + get_number(value);
	return v;
}

static auto get_m3u_line_elements(std::string line)
{
	static std::regex rtc("^\\s*([^#]*[^\\s#])");  // trim comment
    static std::regex r1("(.+?)::");
    static std::regex rn("(.*?),");
	std::vector<std::string> elements;
    std::regex *r = &r1;
    std::smatch sm;
	if(regex_search(line, sm, rtc))
    {
        line = sm[1]; // group 1 (.+?) 
        while (regex_search(line, sm, *r))
        {
            elements.push_back(sm[1]);
            line = sm.suffix();
            r = &rn;
        }
		elements.push_back(line);
    }
	return elements;
}

static bool load_m3u(const std::string& filename, KssFileInfos &file_infos) 
{
	std::filesystem::path path{filename};
	std::string extension = to_lower(path.extension());
	if (extension != ".m3u")
		return false;
	
	
	std::ifstream myfile (filename);

	if (myfile.is_open()) 
	{	
		std::map<std::string, int> files_names_map;
		for(std::string line; std::getline (myfile, line); ) 
		{
			// I try to convert from SHIFT-JIS encoding into UTF-8 to support Japanese characters
			// It would probably be more relevant to ask the user to choose ...
			try {
				std::string sutf = Glib::convert(line, "UTF8", "shift-jis");
				line = sutf;
			}  catch(Glib::ConvertError& e) {}

			if(line.size())
			{
				auto line_data = get_m3u_line_elements(line);
				if(line_data.size() >= 4) 
				{
					// we have at least the filename (pos 0) , the type (KSS in pos 1) , the track number (pos 2) and the name (pos 3)					
					// set filename index
					if(files_names_map.insert({line_data[0], files_names_map.size()}).second)
						file_infos.kss_files.push_back(line_data[0]);
					
					Track &track = file_infos.tracks.emplace_back();
					track.file_index = files_names_map.at(line_data[0]);
					track.track_number = get_number(line_data[2]);
					track.track_name = line_data[3];
					if(line_data.size() > 4) 
					{
						track.duration_sec = to_duration_sec(line_data[4]);
						if(line_data.size() > 5) 
						{
							track.fade_noimp = line_data[5];
							if(line_data.size() > 6)
								track.loop_time_sec_noimp = to_duration_sec(line_data[6]);
						}
					}
				}
				else
				{
					std::cerr << line << std::endl;
				}
			}	
		}

		// XXX zip support ?

		// verify kss files 
		int i = 0;
		for(auto &kss_filename : file_infos.kss_files)
		{
			kss_filename = find_file(path.parent_path(), kss_filename);
			if(kss_filename.size() == 0)
			{
				// invalidate tracks
				for(auto& track : file_infos.tracks) 
				{
					if(track.file_index == i)
						track.file_index = -1;
				}
			}
			++i;
		}
		/* NO
		bool need_increment_track_id = false;
		for(auto& track : file_infos.tracks)
			if(need_increment_track_id = (track.track_number == 0))
				break;
		if(need_increment_track_id)
			for(auto& track : file_infos.tracks)
				++track.track_number;
		*/
		std::vector<Track> tracks_updated;
		for(auto & track : file_infos.tracks)
		{
			if(track.file_index != -1)
				tracks_updated.emplace_back(std::move(track));
		}
		std::swap(tracks_updated, file_infos.tracks);

		file_infos.file_type = (file_infos.tracks.size() > 0 ? KssFileInfos::M3U : KssFileInfos::NONE);
		return file_infos.file_type == KssFileInfos::M3U;
	} 
	else 
	{
		std::cout << "Unable to open "<< filename << std::endl;
	}
	return false;
}

static bool is_valid_kss_file(const std::string &filename) 
{
	bool valid = false;
	KSS *kss = KSS_load_file((char *) filename.c_str());
	if(kss)
	{
		KSS_delete(kss);
		kss = nullptr;
		valid = true;
	}
	return valid;
}

static bool load_kss(const std::string& filename, KssFileInfos &file_infos)
{
	if (is_valid_kss_file(filename))
	{
		file_infos.file_type = KssFileInfos::KSS;
		file_infos.kss_files.push_back(filename);
		for (int i = 1; i < 255; ++i)
		{
			Track& t = file_infos.tracks.emplace_back();
			t.file_index = 0;
			t.track_number = i;
		}
		return true;
	}
	return false;
}

KssFileInfos get_file_infos(const std::string& filename)
{
	namespace fs = std::filesystem;
	fs::path path = filename;
	fs::directory_entry file {path};

	KssFileInfos file_infos;

	if (file.exists() && file.is_regular_file())
		if (!load_kss(filename, file_infos))
			load_m3u(filename, file_infos);
	
	return file_infos;
}

