/**
 * @file ksswindow.cpp
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

#include <iostream>
#include <string>
#include <filesystem>
#include <glibmm.h>
#include "ksswindow.hpp"
#include "config.hpp"



KssWindow::KssWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade)
: Gtk::Window(cobject) 
{
	
	set_title(WINDOW_TITLE);
    // set_icon_from_file(Glib::build_filename("resources/gui","peng.png"));
	// set_icon_from_file((kssroll::data_path / "peng.png").string());
	set_icon(Gdk::Pixbuf::create_from_resource("/kssroll/peng.png"));

	get_widget(refGlade, "headerBar", m_header_bar);
	m_header_bar->set_subtitle("");

	// CSS
	Glib::RefPtr<Gtk::CssProvider> refCssProvider = Gtk::CssProvider::create();
	// refCssProvider->load_from_path(Glib::build_filename("resources/gui", "kssroll.css"));
	// refCssProvider->load_from_path((kssroll::data_path / "kssroll.css").string());
	refCssProvider->load_from_resource("/kssroll/kssroll.css");
	get_style_context()->add_provider_for_screen(Gdk::Screen::get_default(),refCssProvider, GTK_STYLE_PROVIDER_PRIORITY_USER);

	// define window actions
	// m_app_action_group = Gio::SimpleActionGroup::create();
	// m_app_action_group->add_action("open", sigc::mem_fun(*this, &KssWindow::on_action_open) );
	// m_app_action_group->add_action("quit", sigc::mem_fun(*this, &KssWindow::on_action_quit) );
	// m_app_action_group->add_action("about", sigc::mem_fun(*this, &KssWindow::on_action_about) );
	// m_app_action_group->add_action("menu");
	// m_app_action_group->add_action("options");
	// auto win_action_group = Gio::SimpleActionGroup::create();
	// win_action_group->add_action("menu");
	// win_action_group->add_action("options");
    // insert_action_group("win", win_action_group);

    // player actions
    m_player_action_group = Gio::SimpleActionGroup::create();
	m_player_action_play_pause = m_player_action_group->add_action_bool(ACTION_PLAYPAUSE, sigc::mem_fun(*this, &KssWindow::on_action_playpause), false);
	m_player_action_group->add_action(ACTION_STOP, sigc::mem_fun(*this, &KssWindow::on_action_stop) );
	m_player_action_group->add_action(ACTION_PREVIOUSTRACK, sigc::mem_fun(*this, &KssWindow::on_action_previoustrack) );
	m_player_action_group->add_action(ACTION_NEXTTRACK, sigc::mem_fun(*this, &KssWindow::on_action_nexttrack) );
	m_player_action_group->add_action("detect_duration", sigc::mem_fun(*this, &KssWindow::on_action_detect_duration));

    insert_action_group("player", m_player_action_group);
	set_actions_enabled(m_player_action_group, false);

	get_widget(refGlade, "btTbPlayPauseTrack", m_playpause_button);
	get_widget(refGlade, "switchFrequency", m_frequency_switch);
	get_widget(refGlade, "lbFrequency", m_frequency_label);
	get_widget(refGlade, "lbPlaytime", m_label_play_time);
	get_widget(refGlade, "vbTbVolume", m_volume_buton);
	get_object(refGlade, "zoomAdjustment", m_adjustment_zoom);
	get_widget(refGlade, "treeViewTracks", m_treeViewTracks);

	m_frequency_switch->property_active().signal_changed().connect(sigc::mem_fun(*this, &KssWindow::on_frequency_changed));
	m_volume_buton->signal_value_changed().connect(sigc::mem_fun(*this, &KssWindow::on_volume_change));
	m_adjustment_zoom->signal_value_changed().connect(sigc::bind<Glib::RefPtr<Gtk::Adjustment> > (sigc::mem_fun(*this, &KssWindow::on_zoom_change), m_adjustment_zoom));
	m_tracklist_selection_signal =  m_treeViewTracks->get_selection()->signal_changed().connect(sigc::mem_fun(*this, &KssWindow::on_tracklist_selection_changed));

	m_treeViewTracks->set_enable_search(true);

	setup_view_tracks_model();

	// majimix 
    m_majimix = majimix::pa::create_instance();
    m_majimix->set_format(44100, true, 16, 0); 
	// 5 buffers is a good option for majimix
	// high latency (100ms) => buffers size = 100 * 44100 / 5 / 100 = 882
	m_majimix->set_mixer_buffer_parameters(5, 882); 
	m_volume_buton->set_value(128.0);

    show_all();

	//this->get_application()->signal_open().connect(sigc::mem_fun(*this, &KssWindow::on_app_open));
}

KssWindow::~KssWindow()
{
	m_majimix->start_stop_mixer(false);
}

// void KssWindow::initialize(Glib::RefPtr<Gtk::Application> app)
// {
// 	set_application(app);
// 	app->signal_open().connect(sigc::mem_fun(*this, &KssWindow::on_app_open));
//  	// app->signal_shutdown().connect(sigc::mem_fun(*this, &KssWindow::on_app_shutdown));
// 	// this->on_delete_event(GdkEventAny* any_event);
// 	//this->set_property()
// 	// this->signal_close
// 	// this->property_accept_focus
// 	signal_delete_event().connect(sigc::mem_fun(*this, &KssWindow::on_delete_event));
	
// }

// bool KssWindow::on_delete_event(GdkEventAny* any_event)
// {	
// 	on_action_quit();
// 	// any_event->window)
// 	// get_application()->quit();
// 	// get_application()->window
// 	return true;
// }

// void KssWindow::on_app_shutdown() 
// {
// 	// std::cout << "on_app_shutdown()\n";
// 	// get_application()->quit();
// }

// void KssWindow::on_app_open(const Gio::Application::type_vec_files &files, const Glib::ustring &slot)
// {
// 	std::cout << "hello " << slot << "\n";
// 	duration_detection.abort();
// 	if(!files.empty())
// 	{
// 		auto file = files[0];
// 		if(file) 
// 		{
// 			// std::cout << file->get_parse_name() << "\n";
// 			// std::cout << file->get_path() << "\n";
// 			open_file(file->get_path());
// 		}
// 	}
// }

template <class T_Widget>
void KssWindow::get_widget(const Glib::RefPtr<Gtk::Builder> &builder, const Glib::ustring &name, T_Widget *&widget)
{
	builder->get_widget(name, widget);
	if (!widget)
	{
		std::string msg = "No \"" + name + "\" widget in window.ui";
		throw std::runtime_error(msg);
	}
}

template <class T_Object>
void KssWindow::get_object(const Glib::RefPtr<Gtk::Builder> &builder, const Glib::ustring &name, Glib::RefPtr<T_Object> &object)
{
	Glib::RefPtr<Glib::Object> builder_object = builder->get_object(name);
	if (!builder_object)
	{
		std::string msg = "No \"" + name + "\" object in window.ui";
		throw std::runtime_error(msg);
	}
	object = Glib::RefPtr<T_Object>::cast_static(builder_object);
}

void KssWindow::set_action_enabled(Glib::RefPtr<Gio::SimpleActionGroup> action_group, const Glib::ustring &action_name, bool enabled)
{
	Glib::RefPtr<Gio::SimpleAction> ref_action = Glib::RefPtr<Gio::SimpleAction>::cast_static(action_group->lookup_action(action_name));
	if (ref_action)
		ref_action->set_enabled(enabled);
}

void KssWindow::set_actions_enabled(Glib::RefPtr<Gio::SimpleActionGroup> action_group, bool enabled)
{
	for (auto &action : action_group->list_actions())
	{
		set_action_enabled(action_group, action, enabled);
	}
}

int KssWindow::get_frequency() 
{
	return m_frequency_switch->property_active() ? 50 : 60;
}
void KssWindow::set_last_open_folder_path(std::string path)
{
	m_last_open_folder_path = path;
}
const std::string& KssWindow::get_last_open_folder_path() const
{
	return m_last_open_folder_path;
}
void KssWindow::open_file(const std::string &filename)
{
	duration_detection.abort();
	m_majimix->stop_playback(0);
	m_majimix->start_stop_mixer(false); // prevent play new track
	m_majimix->drop_source(0);			// drops all majimix sources
	m_kss_track_handle = 0;				// reset song handle
	set_paused(true);

	m_file_infos = get_file_infos(filename);
	// std::cout << "File loader : " << m_file_infos.file_type << "\n";
	// std::cout << "---------------------\n";

	if (m_file_infos.file_type != KssFileInfos::NONE)
	{

		// add sources
		std::vector<int> sources_handles;
		for (const auto &kss_file : m_file_infos.kss_files)
		{
			sources_handles.emplace_back(m_majimix->add_source_kss(kss_file, 1, 5000));
		}
		m_majimix->update_kss_frequency(0, get_frequency());
		m_header_bar->set_subtitle(std::filesystem::path{filename}.filename().string());

		// populate list
		init_track_data(m_file_infos, sources_handles);
		// position to first track number: it's not always a good option, when list is sorted
		setTrack(1);
		// enable player functions
		set_actions_enabled(m_player_action_group, true);
	}
}

// void KssWindow::on_action_open() 
// {
// 	Gtk::FileChooserDialog dialog("Please choose a file", Gtk::FILE_CHOOSER_ACTION_OPEN);
// 	dialog.set_transient_for(*this);
	
// 	if(m_last_open_folder_path.size())
// 		dialog.set_current_folder(m_last_open_folder_path);

// 	//Add response buttons the the dialog:
// 	dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
// 	dialog.add_button("_Open", Gtk::RESPONSE_OK);

// 	//Add filters, so that only certain file types can be selected
// 	auto filter_kssm3u = Gtk::FileFilter::create();
// 	filter_kssm3u->set_name("kss/m3u files");
// 	filter_kssm3u->add_pattern("*.kss");
// 	filter_kssm3u->add_pattern("*.m3u");
// 	filter_kssm3u->add_pattern("*.KSS");
// 	filter_kssm3u->add_pattern("*.M3U");
// 	dialog.add_filter(filter_kssm3u);

// 	auto filter_kss = Gtk::FileFilter::create();
// 	filter_kss->set_name("kss files");
// 	filter_kss->add_pattern("*.kss");
// 	filter_kss->add_pattern("*.KSS");
// 	dialog.add_filter(filter_kss);

// 	auto filter_m3u = Gtk::FileFilter::create();
// 	filter_m3u->set_name("m3u files");
// 	filter_m3u->add_pattern("*.m3u");
// 	filter_m3u->add_pattern("*.M3U");
// 	dialog.add_filter(filter_m3u);

// 	auto filter_any = Gtk::FileFilter::create();
// 	filter_any->set_name("Any files");
// 	filter_any->add_pattern("*");
// 	dialog.add_filter(filter_any);

// 	//Show the dialog and wait for a user response:
// 	int result = dialog.run();

// 	//Handle the response:
//     switch(result) {
// 	    case(Gtk::RESPONSE_OK): {
// 			m_last_open_folder_path = dialog.get_current_folder();
//             auto filename = dialog.get_filename();
// 			open_file(filename);
// 	      break;
// 	    }
// 	    case Gtk::RESPONSE_CANCEL : {
// 	    //   std::cout << "Cancel clicked." << std::endl;
// 	      break;
// 	    }
// 	    default:
// 	    //   std::cout << "Unexpected button clicked." << std::endl;
// 	      break;
// 	  }
// }

// void KssWindow::on_action_quit()
// {
// 	// std::cout << "on_action_quit()" <<  get_application()->get_windows().size() << "\n";
// 	// get_application()->release();
// 	hide(); 
// 	// FIXME: a gerer de l'exterieur
// 	//        pb lorsque je fais un set_application
// 	// if(get_application())
// 	// {
// 	// 	// test tout quitter 
// 	// 	for (auto *w : get_application()->get_windows())
// 	// 		if(w)
// 	// 			w->hide();

// 	// 	// bool visible = false;
// 	// 	// for (auto *w : get_application()->get_windows())
// 	// 	// 	if (visible = w->is_visible())
// 	// 	// 		break;

// 	// 	// if (!visible)
// 	// 	// {
// 	// 	// 	std::cout << "quit !\n";
// 	// 	// 	get_application()->quit();
// 	// 	// }
// 	// }
// }

// void KssWindow::on_action_about()
// {
// 	Gtk::AboutDialog about_dialog;
// 	about_dialog.set_transient_for(*this);

// 	// about_dialog.set_logo(Gdk::Pixbuf::create_from_file("resources/gui/about2_.png"));
// 	// about_dialog.set_logo(Gdk::Pixbuf::create_from_file((kssroll::data_path / "about2_.png").string()));
// 	about_dialog.set_logo(Gdk::Pixbuf::create_from_resource("/kssroll/about2_.png"));
// 	about_dialog.set_program_name("KSS'Roll");
// 	about_dialog.set_version(KSSROLL_VERSION);
// 	about_dialog.set_comments("KSS songs player");
// 	about_dialog.set_license_type(Gtk::LICENSE_MIT_X11);
// 	about_dialog.set_default_icon_name("help-about");

// 	std::vector<Glib::ustring> list_authors;
// 	list_authors.push_back("François Jacobs");
// 	about_dialog.set_authors(list_authors);

// 	about_dialog.add_credit_section(_("KSS format"), {"libkss library - Mitsutaka Okazaki", "http://github.com/digital-sound-antiques/libkss"});
// 	about_dialog.add_credit_section(_("Audio library"), {
// 															"PortAudio Portable Real-Time Audio Library",
// 															"Copyright (c) 1999-2011 Ross Bencina and Phil Burk",
// 															"http://www.portaudio.com",
// 														});
// 	about_dialog.run();
// }

bool KssWindow::notify_duration_progress()
{
	// std::cout << "duration\n";
	if (progress_window == nullptr)
	{
		duration_detection.get_results();
		return false;
	}

	if (duration_detection.is_running())
	{
		double f = duration_detection.get_progress();
		m_progress_bar->set_fraction(f);
	}
	else
	{
		duration_detection.get_results();
		progress_window->hide();
		delete (progress_window);
		progress_window = nullptr;
		if (duration_detection.is_success())
		{
			m_file_infos = duration_detection.get_results();

			m_tracklist_selection_signal.block();
			std::vector<Gtk::TreeRowReference> removes;

			for (auto iter = m_refTreeModelTracks->children().begin(); iter; ++iter)
			{
				// Gtk::TreeModel::Row row = *iter;
				int id = (*iter)[m_columsTrackList.m_id];
				auto &track = m_file_infos.tracks[id];
				(*iter)[m_columsTrackList.m_duration_ms] = track.duration_detected ? track.duration_ms : -1;
				(*iter)[m_columsTrackList.m_samples] = track.samples_size;

				if (!track.duration_ms)
					removes.emplace_back(m_refTreeModelTracks, m_refTreeModelTracks->get_path(*iter));
			}

			for (auto it = removes.rbegin(); it != removes.rend(); ++it)
			{
				Gtk::TreeIter dead_row_iter = m_refTreeModelTracks->get_iter(it->get_path());
				m_refTreeModelTracks->erase(dead_row_iter);
			}

			set_column_visible(m_columsTrackList.m_samples, true);

			// start tracklist selection events
			m_tracklist_selection_signal.unblock();
		}
		else
		{
			// std::cout << "ECHEC !\n";
		}
		// stop signal
		return false;
	}
	return true;
}

void KssWindow::on_action_detect_duration()
{
	if (progress_window == nullptr)
	{
		if (duration_detection(m_file_infos))
		{
			try
			{
				progress_window = nullptr;
				// refBuilder->add_from_file("resources/gui/kssprogress.glade");
				auto refBuilder = Gtk::Builder::create_from_resource("/kssroll/kssprogress.glade");
				// auto refBuilder = Gtk::Builder::create();
				// refBuilder->add_from_file((kssroll::data_path / "kssprogress.glade").string());
				refBuilder->get_widget("WProgress", progress_window);
				refBuilder->get_widget("PbProgress", m_progress_bar);
				refBuilder->get_widget("LbTitle", m_progress_window_title);
				m_progress_window_title->set_text("Compute duration");
				m_progress_bar->set_fraction(0);

				progress_window->set_parent(*this);
				progress_window->set_transient_for(*this);
				// progress_window->set_modal(false);

				notify_duration_progress_time_signal = Glib::signal_timeout().connect(sigc::mem_fun(*this, &KssWindow::notify_duration_progress), 300);
				progress_window->show();
			}
			catch (const Glib::FileError &ex)
			{
				std::cerr << "FileError: " << ex.what() << std::endl;
				return;
			}
			catch (const Glib::MarkupError &ex)
			{
				std::cerr << "MarkupError: " << ex.what() << std::endl;
				return;
			}
			catch (const Gtk::BuilderError &ex)
			{
				std::cerr << "BuilderError: " << ex.what() << std::endl;
				return;
			}
		}
	}
}

void KssWindow::on_frequency_changed()
{
	int freq = get_frequency();
	m_frequency_label->set_text(freq == 60 ? "60 Hz" : "50 Hz");
	m_majimix->update_kss_frequency(0, freq);
}

void KssWindow::on_volume_change(double volume)
{
	// std::cout << volume << "\n";
	m_majimix->set_master_volume(static_cast<int>(volume));
}

bool KssWindow::on_playing_time_timeout()
{
	// std::cout<<m_majimix->get_kss_playtime_millis(m_kss_track_handle)<<"\n";
	int millis = m_majimix->get_kss_playtime_millis(m_kss_track_handle);
	int sec = millis / 1000;
	std::stringstream str;
#define fmt << std::setfill('0') << std::setw(2) <<
	str fmt(sec / 3600) << ":" fmt(sec / 60 % 60) << ":" fmt(sec % 60) << "." << std::setfill('0') << std::setw(3) << (millis - sec * 1000); // << "."  << std::setfill('0') << std::setw(1) << (millis/100 % 10);
#undef fmt
	// m_status_bar->push(str.str());
	m_label_play_time->set_text(str.str());

	return true;
}

void KssWindow::set_paused(bool p)
{
	m_playback_paused = p;
	m_playpause_button->set_icon_name(p ? "media-playback-start" : "media-playback-pause");
	
	m_player_action_play_pause->get_state(p);
	if(p == m_playback_paused)
		m_player_action_play_pause->change_state(!m_playback_paused);

	if(m_playback_paused == m_timeout_playing_time_signal.connected())
	{
	    if(m_playback_paused)
			m_timeout_playing_time_signal.disconnect();
		else
			m_timeout_playing_time_signal = Glib::signal_timeout().connect(sigc::mem_fun(*this, &KssWindow::on_playing_time_timeout), 100);
	}
}

void KssWindow::on_action_playpause()
{
	int status = m_majimix->get_mixer_status();
	if (status == majimix::MixerStopped)
	{
		// start majimix
		m_majimix->start_stop_mixer(true);
		set_paused(false);
		// restart selected track
		auto iter = m_treeViewTracks->get_selection()->get_selected();
		if (iter)
			setTrack(iter);
	}
	else
	{
		set_paused(!m_playback_paused);
		m_majimix->pause_resume_playback(m_kss_track_handle, m_playback_paused);
	}
}

template <class T>
Gtk::TreeView::Column *KssWindow::get_column_by_model_sort_column(const Gtk::TreeModelColumn<T> &model_sort_column)
{
	for (auto column : m_treeViewTracks->get_columns())
	{
		if (column->get_sort_column_id() == model_sort_column.index())
		{
			return column;
		}
	}
	return nullptr;
}

template <class T>
void KssWindow::set_column_visible(const Gtk::TreeModelColumn<T> &model_sort_column, bool visible)
{
	auto column = get_column_by_model_sort_column(model_sort_column);
	if (column)
		column->set_visible(visible);
}

void KssWindow::on_action_stop()
{
	int status = m_majimix->get_mixer_status();
	if (majimix::MixerStopped != status)
	{
		m_majimix->stop_playback(0);
		m_majimix->start_stop_mixer(false);
		m_kss_track_handle = 0;
		set_paused(true);
	}
}

void KssWindow::on_action_previoustrack()
{
	// std::cout << "on_action_previoustrack() " <<std::endl;
	auto selection = m_treeViewTracks->get_selection();
	if (selection)
	{
		auto view_it = selection->get_selected();
		if (view_it && --view_it)
		{
			m_treeViewTracks->get_selection()->select(view_it);
			m_treeViewTracks->scroll_to_row(m_refModelSort->get_path(view_it));
		}
	}
}

void KssWindow::on_action_nexttrack()
{
	// std::cout << "on_action_nexttrack() " <<std::endl;
	auto selection = m_treeViewTracks->get_selection();
	if (selection)
	{
		auto view_it = selection->get_selected();
		if (view_it && ++view_it)
		{
			m_treeViewTracks->get_selection()->select(view_it);
			m_treeViewTracks->scroll_to_row(m_refModelSort->get_path(view_it));
		}
	}
}

void KssWindow::setup_view_tracks_model()
{
	m_treeViewTracks->remove_all_columns();

	// data model
	m_refTreeModelTracks = Gtk::ListStore::create(m_columsTrackList);
	// view model
	m_refModelSort = Gtk::TreeModelSort::create(m_refTreeModelTracks);
	m_treeViewTracks->set_model(m_refModelSort);

	int line_number_column_id = m_treeViewTracks->append_column("n°", m_columsTrackList.m_col_line_number) - 1;
	int track_number_column_id = m_treeViewTracks->append_column("id", m_columsTrackList.m_col_track_number) - 1;
	int track_name_column_id = m_treeViewTracks->append_column("name", m_columsTrackList.m_col_track_name) - 1;
	auto cellRenderer = Gtk::manage(new Gtk::CellRendererText);
	int m3u_duration_column_id = m_treeViewTracks->append_column("approx. duration", *cellRenderer) - 1;
	int precise_duration_column_id = m_treeViewTracks->append_column("duration ms", m_columsTrackList.m_duration_ms) - 1;

	Gtk::TreeView::Column *pColumn = m_treeViewTracks->get_column(line_number_column_id);
	pColumn->set_reorderable(true);
	pColumn->set_sort_column(m_columsTrackList.m_col_line_number);
	m_treeViewTracks->get_column_cell_renderer(line_number_column_id)->set_alignment(1.0f, 0.0f);

	pColumn = m_treeViewTracks->get_column(track_number_column_id);
	pColumn->set_reorderable(true);
	pColumn->set_sort_column(m_columsTrackList.m_col_track_number);
	m_treeViewTracks->get_column_cell_renderer(track_number_column_id)->set_alignment(1.0f, 0.0f);

	pColumn = m_treeViewTracks->get_column(track_name_column_id);
	pColumn->set_resizable();
	// pColumn->set_sizing(Gtk::TreeViewColumnSizing::TREE_VIEW_COLUMN_AUTOSIZE);
	pColumn->set_sizing(Gtk::TreeViewColumnSizing::TREE_VIEW_COLUMN_GROW_ONLY);
	pColumn->set_reorderable(true);
	pColumn->set_sort_column(m_columsTrackList.m_col_track_name);

	pColumn = m_treeViewTracks->get_column(m3u_duration_column_id);
	pColumn->set_reorderable(true);
	pColumn->set_sort_column(m_columsTrackList.m_col_track_approx_duration);
	pColumn->set_cell_data_func(*m_treeViewTracks->get_column_cell_renderer(m3u_duration_column_id), sigc::mem_fun(*this, &KssWindow::on_tracklist_set_duration_value_sec));

	pColumn = m_treeViewTracks->get_column(precise_duration_column_id);
	pColumn->set_reorderable(true);
	pColumn->set_sort_column(m_columsTrackList.m_samples);
	pColumn->set_cell_data_func(*m_treeViewTracks->get_column_cell_renderer(precise_duration_column_id), sigc::mem_fun(*this, &KssWindow::on_tracklist_set_duration_value_ms));
}

void KssWindow::on_tracklist_set_duration_value_sec(Gtk::CellRenderer *renderer ,  const Gtk::TreeModel::iterator& iter) 
{
	if(iter)
		set_duration_value_millis(renderer, (*iter)[m_columsTrackList.m_col_track_approx_duration] * 1000, false);
}

void KssWindow::on_tracklist_set_duration_value_ms(Gtk::CellRenderer *renderer ,  const Gtk::TreeModel::iterator& iter) 
{
	if(iter)
		set_duration_value_millis(renderer, (*iter)[m_columsTrackList.m_duration_ms], true);
}

void KssWindow::set_duration_value_millis(Gtk::CellRenderer *renderer, int duration_millis, bool millis)
{
	std::stringstream str;
	if (duration_millis == -1)
		str << "[no end found (loop)]";
	else
	{
#define fmt << std::setfill('0') << std::setw(2) <<
		// str fmt (dur_sec/3600) << ":" fmt (dur_sec/60 % 60) << ":"  fmt (dur_sec % 60);
		str fmt(duration_millis / 3600000) << ":" fmt(duration_millis / 60000 % 60) << ":" fmt(duration_millis / 1000 % 60);
		if (millis)
			str << "." << std::setfill('0') << std::setw(3) << (duration_millis % 1000);
#undef fmt
	}
	((Gtk::CellRendererText *)renderer)->property_text() = str.str();
}

void KssWindow::init_track_data(const KssFileInfos &infos, const std::vector<int> &source_handles)
{
	// stop tracklist selection events
	m_tracklist_selection_signal.block();
	m_refTreeModelTracks->clear();

	set_column_visible(m_columsTrackList.m_samples, false);

	bool m3u = infos.file_type == KssFileInfos::M3U;
	// set_column_visible(m_columsTrackList.m_col_track_name, m3u);
	set_column_visible(m_columsTrackList.m_col_track_approx_duration, m3u);

	guint line = 0;
	for (auto &track : infos.tracks)
	{
		Gtk::TreeModel::Row row = *(m_refTreeModelTracks->append());
		row[m_columsTrackList.m_source_handle] = source_handles[track.file_index];
		row[m_columsTrackList.m_id] = line;
		row[m_columsTrackList.m_col_line_number] = ++line;
		row[m_columsTrackList.m_col_track_number] = track.track_number;
		row[m_columsTrackList.m_col_track_name] = track.track_name;
		row[m_columsTrackList.m_col_track_approx_duration] = track.duration_sec; // "00:00:00.000";
		row[m_columsTrackList.m_duration_ms] = 0;								 // "00:00:00.000";
		row[m_columsTrackList.m_samples] = 0;									 // "00:00:00.000";
	}
	// start tracklist selection events
	m_tracklist_selection_signal.unblock();
}

void KssWindow::on_tracklist_selection_changed()
{
	Gtk::TreeModel::iterator iter = m_treeViewTracks->get_selection()->get_selected();
	// std::cout << "selection change " << m_kss_track_handle << "\n";

	if (iter)
	{
		// std::cerr << "selection changed " << "\n";
		setTrack(iter);
	}
	else
	{
		// std::cerr << "deselecion" << "\n";
		if (m_kss_track_handle)
		{
			m_majimix->stop_playback(m_kss_track_handle);
			// std::cerr << "stop track_handle " << m_kss_track_handle << "\n";
		}
		m_kss_track_handle = 0;
	}
}

void KssWindow::setTrack(guint line_number)
{
	// std::cerr << "set track line "<< line_number << "\n";
	if (line_number)
	{
		Gtk::TreeIter iter{nullptr}; // view it
		{
			for (iter = m_refModelSort->children().begin(); iter != m_refModelSort->children().end(); ++iter)
				if ((*iter)[m_columsTrackList.m_col_line_number] == line_number)
					break;
		}
		if (iter)
		{
			m_treeViewTracks->get_selection()->select(iter);
			m_treeViewTracks->scroll_to_row(m_refModelSort->get_path(iter));
		}
	}
}

void KssWindow::setTrack(const Gtk::TreeIter &iter)
{
	// std::cerr << "set track iter \n";
	auto model_it = m_refModelSort->convert_iter_to_child_iter(iter);
	Gtk::TreeModel::Row row = *model_it;
	{

		row[m_columsTrackList.m_col_line_number];
		const uint8_t track_number = row[m_columsTrackList.m_col_track_number];
		const int source_handle = row[m_columsTrackList.m_source_handle];
		// std::cerr << "before start track  handle " << m_kss_track_handle << " playback_paused :" << m_playback_paused << "\n";

		int kss_track_handle = m_majimix->play_kss_track(source_handle, track_number, true, true, true);
		// std::cerr << "start track "<< " on handle " << kss_track_handle << "\n";

		if (kss_track_handle)
		{
			if (m_kss_track_handle && m_kss_track_handle != kss_track_handle)
			{
				// changement de track on ne souhaite pas mixer plusieurs lignes ici
				m_majimix->stop_playback(m_kss_track_handle);
			}
			m_kss_track_handle = kss_track_handle;
		}
		// else std::cerr << " !!!!!!!!! NOHANDLE !!!!!!!!!!\n";

		if (m_playback_paused)
			m_majimix->pause_resume_playback(m_kss_track_handle, m_playback_paused);
		// std::cerr << "start track "<< static_cast<int>(track_number) << " on handle " << m_kss_track_handle << "\n";
	}
}

void KssWindow::on_zoom_change(const Glib::RefPtr<Gtk::Adjustment> &adjustment)
{
	// std::cout << "zoom "<< adjustment->get_value() << std::endl;
	for (guint i = 0; i < m_treeViewTracks->get_n_columns(); ++i)
	{
		m_treeViewTracks->get_column(i)->set_sizing(Gtk::TreeViewColumnSizing::TREE_VIEW_COLUMN_AUTOSIZE);
		((Gtk::CellRendererText *)m_treeViewTracks->get_column_cell_renderer(i))->property_scale() = adjustment->get_value(); // Pango::SCALE_XX_SMALL
	}
	// on doit prevenir des changements pour chaque ligne
	for (Gtk::TreeModel::iterator iter = m_refTreeModelTracks->children().begin(); iter != m_refTreeModelTracks->children().end(); ++iter)
	{
		m_refTreeModelTracks->row_changed(m_refTreeModelTracks->get_path(iter), iter);
	}
}