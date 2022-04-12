/**
 * @file ksswindow.hpp
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

#ifndef KSSWINDOW_H_
#define KSSWINDOW_H_

#include <gtkmm.h>
#include <memory>
#include <cstdint>
#include <majimix/majimix.hpp>
#include "loader.hpp" 
#include "kss_tools.hpp"

// DONE: duration empty
// DONE: handle column not visible
// DONE: playing time
// DONE: numbers alignment
// DONE: zoom
// DONE: option autostop + autostop time
// DONE: durations detection
// DONE: show / hide duration ms
// ABORT: m3u 0 based id
// DONE: resources.c
// DONE: compute duration + open desktop => abort 
// FIXME: column resize (BUG when zoom !)
// TODO: duration detection option abort - facile avec un atomic + gestion thread nombre < max proc voire choisir / param + ne doit pas être completement modale !
// TODO: I18n en cours - a ameliorer avec install - colonne - compute duration
// TODO: prefs. / settings : switch / zoom /rate / shortcuts 
// TODO: historique
// TODO: export waves / ogg / FLAC
// TODO: shortcuts
// TODO: zip + être plus souple avec le nom des kss des m3u (case unsensitive)
// TODO: ouverture de plusieurs fichiers => concat
// TODO: fermer / quitter - 2 options différentes
// TODO: permettre de voir les commentaires M3U
// TODO: save to m3u
// PARTIAL: menu gerer par application KssRoll  



class KssWindow : public Gtk::Window {

	// void on_app_open(const Gio::Application::type_vec_files &, const Glib::ustring &)
	// {
	// 	std::cout << "hello\n";
	// }

	static constexpr const char* WINDOW_TITLE			= "KSS'Roll";

	static constexpr const char* ACTION_PLAYPAUSE 		= "playpause";				// alternatively play and pause
	static constexpr const char* ACTION_STOP 			= "stop";					// stop playback
	static constexpr const char* ACTION_PREVIOUSTRACK 	= "previoustrack";			// go to previous track
	static constexpr const char* ACTION_NEXTTRACK 		= "nexttrack";				// go to next track


	// // app action group
	// Glib::RefPtr<Gio::SimpleActionGroup> m_app_action_group;
	// player action group
	Glib::RefPtr<Gio::SimpleActionGroup> m_player_action_group;
	// player play/pause action
    Glib::RefPtr<Gio::SimpleAction> m_player_action_play_pause {nullptr};


	// widgets
	Gtk::HeaderBar *m_header_bar;
	Gtk::Switch *m_frequency_switch {nullptr};
	Gtk::Label *m_frequency_label {nullptr};
	Gtk::Label *m_label_play_time {nullptr};
	Gtk::VolumeButton *m_volume_buton {nullptr};
	Gtk::ToggleToolButton *m_playpause_button {nullptr};       
	Gtk::TreeView *m_treeViewTracks;
	Glib::RefPtr<Gtk::Adjustment> m_adjustment_zoom;

	Gtk::Window *progress_window {nullptr};
	Gtk::ProgressBar  *m_progress_bar {nullptr};
	Gtk::Label *m_progress_window_title {nullptr};


	// majimix instance
    std::unique_ptr<majimix::Majimix> m_majimix;
	// majimix song handle
    int m_kss_track_handle = 0;
	// player status    
	bool m_playback_paused {true};
	// last opened folder
	std::string m_last_open_folder_path;
	// current file
	KssFileInfos m_file_infos;
	// Durations detection tool
	DurationDetection duration_detection;




	// list view model
	class ModelColumnsTrackList : public Gtk::TreeModel::ColumnRecord {
	public:
		Gtk::TreeModelColumn<guint> m_col_line_number;				// line numer 1-n					// TODO: devrait être insensible au tri
		Gtk::TreeModelColumn<uint8_t> m_col_track_number;			// track number in kss file [0-255] // TODO: ne devrait pas être affichée
		Gtk::TreeModelColumn<std::string> m_col_track_name;			// track name                       // TODO: afficher si m3u ?
		Gtk::TreeModelColumn<guint> m_col_track_approx_duration;    // duration (second)                // TODO: duration du m3u - afficher uniquement si m3u
		Gtk::TreeModelColumn<gint> m_duration_ms;                   // duration (second)
	    Gtk::TreeModelColumn<int> m_source_handle;                  //                                  
		Gtk::TreeModelColumn<int> m_id;																	// TODO: ajourd'hui c'est un doublon de line number -1
		Gtk::TreeModelColumn<uint32_t> m_samples;
		ModelColumnsTrackList()
		{
			add(m_col_line_number);
			add(m_col_track_number);
			add(m_col_track_name);
			add(m_col_track_approx_duration);
			add(m_duration_ms);
			add(m_source_handle);
			add(m_id);
			add(m_samples);
		}
	};

	// kss row model : m_kss_track_view_model
	ModelColumnsTrackList m_columsTrackList;

	// data model
	Glib::RefPtr<Gtk::ListStore> m_refTreeModelTracks;

	// sortable model ( model_it <- convert_iter_to_child_iter)
	Glib::RefPtr<Gtk::TreeModelSort> m_refModelSort;

	template<class T>
	Gtk::TreeView::Column* get_column_by_model_sort_column(const Gtk::TreeModelColumn<T>& model_sort_column);

	template<class T>
	void set_column_visible(const Gtk::TreeModelColumn<T> &model_sort_column, bool visible);

	void setup_view_tracks_model();
	void init_track_data(const KssFileInfos &infos, const std::vector<int>& source_handles);
	void set_duration_value_millis(Gtk::CellRenderer *renderer, int duration_millis, bool millis);
	int get_frequency();
	void setTrack(guint line_number);
	void setTrack(const Gtk::TreeIter& iter);
	void set_paused(bool);
	
	
	
	
	bool notify_duration_progress();

	void on_tracklist_set_duration_value_sec(Gtk::CellRenderer*  renderer, const Gtk::TreeModel::iterator& iter);
	void on_tracklist_set_duration_value_ms(Gtk::CellRenderer*  renderer, const Gtk::TreeModel::iterator& iter);
	void on_tracklist_selection_changed();





	// utils
	// -----

	// builder widget instantiation
	template <class T_Widget>
	static void get_widget(const Glib::RefPtr<Gtk::Builder>& builder, const Glib::ustring& name, T_Widget*& widget);
	// builder object instantiation
	template <class T_Object>
	static void get_object(const Glib::RefPtr<Gtk::Builder>& builder, const Glib::ustring& name, Glib::RefPtr<T_Object>& object);
	// enable / disable an action by name
	void set_action_enabled(Glib::RefPtr<Gio::SimpleActionGroup> action_group, const Glib::ustring& action_name, bool enabled);
	// enable / disable all actions of an action group
	void set_actions_enabled(Glib::RefPtr<Gio::SimpleActionGroup> action_group, bool enabled);


	// signals
	// -------

	// treeview: selection change
	sigc::connection m_tracklist_selection_signal;
	// playing duration: timeout
	sigc::connection m_timeout_playing_time_signal;
	// compute songs durations: timout
	sigc::connection notify_duration_progress_time_signal;


	// actions
	// -------

	// // open a file
    // void on_action_open();
	// // quit the app
    // void on_action_quit();
	// // display about box
    // void on_action_about();
	// detect durations
	void on_action_detect_duration();
	// player: play/pause
	void on_action_playpause();
	// player: stop
	void on_action_stop();
	// player: position to previous song
	void on_action_previoustrack();
	// player: position to next song
    void on_action_nexttrack();
	// player: frequency switched (50/60 Hz)
	void on_frequency_changed();
	// player: volume modified
	void on_volume_change(double volume);
	// player: update playing time
	bool on_playing_time_timeout();
	// treeview: content zoomed in/out
	void on_zoom_change(const Glib::RefPtr<Gtk::Adjustment>& adjustment);

	// void on_app_shutdown();
	// bool on_delete_event(GdkEventAny* any_event);


public:
	KssWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade);
	virtual ~KssWindow();

	// void initialize(Glib::RefPtr<Gtk::Application> app);
	
	// open file from desktop
	// void on_app_open(const Gio::Application::type_vec_files &files, const Glib::ustring &s);
	void open_file(const std::string &name);
	void set_last_open_folder_path(std::string path);
	const std::string& get_last_open_folder_path() const;
};


#endif /* KSSWINDOW_H_ */
