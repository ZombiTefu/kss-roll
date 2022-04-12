/**
 * @file main.cpp
 * @author  François Jacobs
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
#include <gtkmm.h>
#include <iostream>
#include <filesystem>
#include "ksswindow.hpp"
#include <majimix/majimix.hpp>
#include "config.hpp"

class KssRoll : public Gtk::Application
{
	KssRoll(int argc, char *argv[]);
	KssWindow *create_appwindow();
	void on_hide_window(Gtk::Window *window);
	void on_action_open();
	void on_action_about();
	void on_action_quit();

//protected:
	// // Override default signal handlers:
	// void on_activate() override;
	// void on_open(const Gio::Application::type_vec_files &files, const Glib::ustring &hint) override;

protected:
	// Override default signal handlers:
	void on_startup() override;
	void on_activate() override;
	void on_open(const Gio::Application::type_vec_files &files,
				 const Glib::ustring &hint) override;

public:
	static Glib::RefPtr<KssRoll> create(int argc, char *argv[]);
};

KssRoll::KssRoll(int argc, char *argv[])
: Gtk::Application(argc, argv, "kssroll.majimix.kss", Gio::ApplicationFlags::APPLICATION_HANDLES_OPEN)
{
}

Glib::RefPtr<KssRoll> KssRoll::create(int argc, char *argv[])
{
  return Glib::RefPtr<KssRoll>(new KssRoll(argc, argv));
}

KssWindow* KssRoll::create_appwindow()
{
	KssWindow *kssWindow = nullptr;
	try
	{
		// auto refBuilder = Gtk::Builder::create();
		// refBuilder->add_from_file((kssroll::data_path / "ksswin.glade").string());
		// resources.c :  glib-compile-resources --target=resources.cpp --generate-source test_resources.xml
		auto refBuilder = Gtk::Builder::create_from_resource("/kssroll/ksswin.glade");
		refBuilder->get_widget_derived("window1", kssWindow);
		if(kssWindow) 
		{
			add_window(*kssWindow);
			// Delete the window when it is hidden.
  			kssWindow->signal_hide().connect(sigc::bind<Gtk::Window*>(sigc::mem_fun(*this, &KssRoll::on_hide_window), kssWindow));

		}
	}
	catch(const Glib::FileError& ex)
	{
		std::cerr << "FileError: " << ex.what() << std::endl;
	}
	catch(const Glib::MarkupError& ex)
	{
		std::cerr << "MarkupError: " << ex.what() << std::endl;
	}
	catch(const Gtk::BuilderError& ex)
	{
		std::cerr << "BuilderError: " << ex.what() << std::endl;
	}
  	return kssWindow;
}

void KssRoll::on_startup()
{
  std::cout << "startup\n";
  // Call the base class's implementation.
  Gtk::Application::on_startup();

  add_action("open", sigc::mem_fun(*this, &KssRoll::on_action_open));
  add_action("quit", sigc::mem_fun(*this, &KssRoll::on_action_quit));
  add_action("about", sigc::mem_fun(*this, &KssRoll::on_action_about));
  add_action("menu");
  add_action("options");

  //   // Add actions and keyboard accelerators for the application menu.
  //   // add_action("preferences", sigc::mem_fun(*this, &KssRoll::on_action_preferences));
  //   // add_action("quit", sigc::mem_fun(*this, &KssRoll::on_action_quit));
  // //   set_accel_for_action("app.quit", "<Ctrl>Q");

  //   auto refBuilder = Gtk::Builder::create();
  //   try
  //   {
  //     refBuilder->add_from_resource("/kssroll/app_menu.ui");
  //   }
  //   catch (const Glib::Error& ex)
  //   {
  //     std::cerr << "KssRoll::on_startup(): " << ex.what() << std::endl;
  //     return;
  //   }

  //   auto object = refBuilder->get_object("appmenu");
  //   auto app_menu = Glib::RefPtr<Gio::MenuModel>::cast_dynamic(object);

  //   if (app_menu)
  //   {
  // 	  std::cout << "set MENU\n";
  //     set_app_menu(app_menu);
  //   }
  //   else
  //     std::cerr << "ExampleApplication::on_startup(): No \"appmenu\" object in app_menu.ui"
  //               << std::endl;
}

void KssRoll::on_action_open()
{
	auto appwindow = dynamic_cast<KssWindow *>(get_windows()[0]);
	Gtk::FileChooserDialog dialog("Please choose a file", Gtk::FILE_CHOOSER_ACTION_OPEN);
	dialog.set_transient_for(*appwindow);
	
	if(appwindow->get_last_open_folder_path().size())
		dialog.set_current_folder(appwindow->get_last_open_folder_path());

	//Add response buttons the the dialog:
	dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
	dialog.add_button("_Open", Gtk::RESPONSE_OK);

	//Add filters, so that only certain file types can be selected
	auto filter_kssm3u = Gtk::FileFilter::create();
	filter_kssm3u->set_name("kss/m3u files");
	filter_kssm3u->add_pattern("*.kss");
	filter_kssm3u->add_pattern("*.m3u");
	filter_kssm3u->add_pattern("*.KSS");
	filter_kssm3u->add_pattern("*.M3U");
	dialog.add_filter(filter_kssm3u);

	auto filter_kss = Gtk::FileFilter::create();
	filter_kss->set_name("kss files");
	filter_kss->add_pattern("*.kss");
	filter_kss->add_pattern("*.KSS");
	dialog.add_filter(filter_kss);

	auto filter_m3u = Gtk::FileFilter::create();
	filter_m3u->set_name("m3u files");
	filter_m3u->add_pattern("*.m3u");
	filter_m3u->add_pattern("*.M3U");
	dialog.add_filter(filter_m3u);

	auto filter_any = Gtk::FileFilter::create();
	filter_any->set_name("Any files");
	filter_any->add_pattern("*");
	dialog.add_filter(filter_any);

	//Show the dialog and wait for a user response:
	int result = dialog.run();

	//Handle the response:
    switch(result) {
	    case(Gtk::RESPONSE_OK): {
			appwindow->set_last_open_folder_path(dialog.get_current_folder());
            auto filename = dialog.get_filename();
			appwindow->open_file(filename);
	      break;
	    }
	    case Gtk::RESPONSE_CANCEL : {
	    //   std::cout << "Cancel clicked." << std::endl;
	      break;
	    }
	    default:
	    //   std::cout << "Unexpected button clicked." << std::endl;
	      break;
	  }
}

void KssRoll::on_action_quit()
{
	auto windows = get_windows();
	windows[0]->hide();
	//   for (auto window : windows)
	//     window->hide();
}

void KssRoll::on_action_about()
{
	Gtk::AboutDialog about_dialog;
	about_dialog.set_transient_for(*get_windows()[0]);

	// about_dialog.set_logo(Gdk::Pixbuf::create_from_file("resources/gui/about2_.png"));
	// about_dialog.set_logo(Gdk::Pixbuf::create_from_file((kssroll::data_path / "about2_.png").string()));
	about_dialog.set_logo(Gdk::Pixbuf::create_from_resource("/kssroll/about2_.png"));
	about_dialog.set_program_name("KSS'Roll");
	about_dialog.set_version(KSSROLL_VERSION);
	about_dialog.set_comments("KSS songs player");
	about_dialog.set_license_type(Gtk::LICENSE_MIT_X11);
	about_dialog.set_default_icon_name("help-about");

	std::vector<Glib::ustring> list_authors;
	list_authors.push_back("François Jacobs");
	about_dialog.set_authors(list_authors);

	about_dialog.add_credit_section(_("KSS format"), {"libkss library - Mitsutaka Okazaki", "http://github.com/digital-sound-antiques/libkss"});
	about_dialog.add_credit_section(_("Audio library"), {
															"PortAudio Portable Real-Time Audio Library",
															"Copyright (c) 1999-2011 Ross Bencina and Phil Burk",
															"http://www.portaudio.com",
														});
	about_dialog.run();
}

void KssRoll::on_activate()
{
	std::cout << "on_activate\n";
	KssWindow *appwindow = nullptr;
/*
	auto windows = get_windows();
	if (windows.size() > 0)
		appwindow = dynamic_cast<KssWindow *>(windows[0]);
	else */
		appwindow = create_appwindow();
		
		/* FIXME: 
		(kssroll:32155): GLib-GIO-WARNING **: 23:33:34.887: Your application did not unregister from D-Bus before destruction. Consider using g_application_run().
		(kssroll:32155): GLib-GIO-CRITICAL **: 23:33:34.887: g_dbus_connection_flush_sync: assertion 'G_IS_DBUS_CONNECTION (connection)' failed
		(kssroll:32155): GLib-GObject-CRITICAL **: 23:33:34.887: g_object_unref: assertion 'G_IS_OBJECT (object)' failed
		appwindow->set_application(Glib::RefPtr<Gtk::Application>{this});
		*/
	


	if (appwindow)
		appwindow->present();
}

void KssRoll::on_open(const Gio::Application::type_vec_files &files,
					  const Glib::ustring & hint)
{
	std::cout << "on_open\n";
	// The application has been asked to open some files,
	// so let's open a new view for each one.
	KssWindow *appwindow = nullptr;
	auto windows = get_windows();
	if (windows.size() > 0)
		appwindow = dynamic_cast<KssWindow *>(windows[0]);

	if (!appwindow)
	{
		std::cout << "on_open : no win\n";
		appwindow = create_appwindow();
	}

	if (appwindow)
	{
		std::cout << "on_open : win\n";
		//   for (const auto& file : files)
		//     appwindow->open_file_view(file);
		if (files.size() > 0)
		{
			std::cout << "on_open : file\n";
			//appwindow->on_app_open(files, hint);
			auto file = files[0];
			if (file)
			{
				appwindow->open_file(file->get_path());
			}
		}
		appwindow->present();
	}
}

void KssRoll::on_hide_window(Gtk::Window* window)
{
  // window->unset_application();
  delete window;
}



// std::string getexepath() {
// #ifdef _WIN32
// 	 char result[ MAX_PATH ];
// 	 return std::string( result, GetModuleFileName( NULL, result, MAX_PATH ) );
// //#elif linux
// #else
//   char result[ PATH_MAX ];
//   ssize_t count = readlink( "/proc/self/exe", result, PATH_MAX );
//   return std::string( result, (count > 0) ? count : 0 );
// #endif
// }

void createWindow(KssWindow*& kssWindow) {
	//Load the Glade file and instantiate its widgets:
	// auto refBuilder = Gtk::Builder::create();
	try
	{
		//	     m_refBuilder->add_from_string(ui_info);
		// refBuilder->add_from_file("resources/gui/ksswin.glade");
		// refBuilder->add_from_file((kssroll::data_path / "ksswin.glade").string());
		auto refBuilder = Gtk::Builder::create_from_resource("/kssroll/ksswin.glade");


		//		Gtk::Adjustment *zoom_adjustment;
		//		refBuilder->get_widget("adjustment-zoom", zoom_adjustment);
		//		if (!zoom_adjustment)
		//			throw std::runtime_error("No \"zoom_adjustment\" object in window.ui");


		// XXX refBuilder->add_from_resource("/gui/kss-ix_w.glade");
		// This call to get_widget_derived() requires gtkmm 3.19.7 or higher.
		// refBuilder->get_widget_derived("window1", kssWindow, getexepath());
		refBuilder->get_widget_derived("window1", kssWindow);


	}
	catch(const Glib::FileError& ex)
	{
		std::cerr << "FileError: " << ex.what() << std::endl;
		return;
	}
	catch(const Glib::MarkupError& ex)
	{
		std::cerr << "MarkupError: " << ex.what() << std::endl;
		return;
	}
	catch(const Gtk::BuilderError& ex)
	{
		std::cerr << "BuilderError: " << ex.what() << std::endl;
		return;
	}
	//	// inutile !
	//	catch(const Glib::Error& ex)
	//	{
	//		std::cerr << "Building window failed: " <<  ex.what();
	//		return;
	//	}
}

// void on_app_open(const Gio::Application::type_vec_files &files, const Glib::ustring &s)
// {
// 	std::cout << "hello " << s << "\n";
// }

// void on_startup(Gtk::Window *w, Glib::RefPtr<Gtk::Application> app) 
// {
// 	w->set_application(app);
// }

int main (int argc, char *argv[])
{
     /* Setup locale/gettext */
    setlocale (LC_ALL, "");
    bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);

	// // test resources files
	// // if not installed DATADIR may not contains resources files
	// // we try another path 
	// {
	// 	std::filesystem::path path{kssroll::data_path / "ksswin.glade"};
	// 	std::filesystem::directory_entry de{path};
	// 	if (!de.exists())
	// 	{
	// 		kssroll::data_path = "resources/gui";
	// 		path = kssroll::data_path / "ksswin.glade";
	// 		de.assign(path);
	// 		if (de.exists())
	// 			std::cout << "found resources in " << path.parent_path() << "\n";
	// 		else
	// 		{
	// 			std::cout << "can't find gui resources\n";
	// 			return -1; // TODO: msg
	// 		}
	// 	}
	// }

	// Since this example is running uninstalled, we have to help it find its
	// schema. This is *not* necessary in a properly installed application.
	// Glib::setenv ("GSETTINGS_SCHEMA_DIR", ".", false);

    // std::cout<<"\tinitialize\n";
    majimix::pa::initialize();
//    auto majimix_ptr = majimix::pa::create_instance();
//
//	std::cout<<"\tset_format 44100Hz 16bits stereo with 3 mixer channels\n";
//    if(majimix_ptr->set_format( 44100, true, 16, 3))
//	{
//        std::cout<<"\t\t OK\n";
//    }

/**

    auto app = Gtk::Application::create(argc, argv, "kssroll.majimix.kss", Gio::ApplicationFlags::APPLICATION_HANDLES_OPEN);
	
	//Get the GtkBuilder-instantiated dialog:
	KssWindow* kssWindow = nullptr;
	createWindow(kssWindow);
	if(kssWindow) {
		
		// app->signal_startup().connect(sigc::bind<Gtk::Window*, Glib::RefPtr<Gtk::Application>>(sigc::ptr_fun(on_startup), kssWindow, app), true);
		app->signal_startup().connect(sigc::bind<Glib::RefPtr<Gtk::Application>>(sigc::mem_fun(*kssWindow, &KssWindow::initialize), app), true);
		// app->signal_open().connect(sigc::ptr_fun(&on_app_open));
		// Start:
		// app->hold();
		app->run(*kssWindow);
		// kssWindow->set_application(app);
	}
	delete kssWindow;
	// app->release();

 **/
	auto app = KssRoll::create(argc, argv);
	app->run();


    std::cout<<"terminate\n";
	majimix::pa::terminate();

    return 0;
}
