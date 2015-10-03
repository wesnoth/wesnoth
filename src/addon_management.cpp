/* $Id$ */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
                 2008 - 2011 by Ignacio R. Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "addon_management.hpp"
#include "dialogs.hpp"
#include "filesystem.hpp"
#include "foreach.hpp"
#include "formatter.hpp"
#include "game_display.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "gui/dialogs/addon_connect.hpp"
#include "gui/dialogs/addon_list.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "log.hpp"
#include "marked-up_text.hpp"
#include "serialization/parser.hpp"
#include "version.hpp"
#include "wml_separators.hpp"
#include "formula_string_utils.hpp"

static lg::log_domain log_config("config");
#define ERR_CFG LOG_STREAM(err , log_config)
#define LOG_CFG LOG_STREAM(info, log_config)
#define WRN_CFG LOG_STREAM(warn, log_config)

static lg::log_domain log_filesystem("filesystem");
#define ERR_FS  LOG_STREAM(err , log_filesystem)

static lg::log_domain log_network("network");
#define ERR_NET LOG_STREAM(err , log_network)
#define LOG_NET LOG_STREAM(info, log_network)

void get_addon_info(const std::string& addon_name, config& cfg)
{
	const std::string parentd = get_addon_campaigns_dir();

	// Cope with old-style or new-style file organization
	const std::string exterior = parentd + "/" + addon_name + ".pbl";
	const std::string interior = parentd + "/" + addon_name + "/_server.pbl";
	const std::string pbl_file = (file_exists(exterior)? exterior : interior);

	scoped_istream stream = istream_file(pbl_file);
	read(cfg, *stream);
}

void set_addon_info(const std::string& addon_name, const config& cfg)
{
	const std::string parentd = get_addon_campaigns_dir();

	scoped_ostream stream = ostream_file(parentd + "/" + addon_name + ".pbl");
	write(*stream, cfg);
}

bool remove_local_addon(const std::string& addon, std::string* log)
{
	bool ret = true;
	std::ostringstream messages;
	const std::string addon_dir = get_addon_campaigns_dir() + "/" + addon;

	LOG_CFG << "removing local add-on: " << addon << '\n';

	if(file_exists(addon_dir) && !delete_directory(addon_dir)) {
		messages << "Failed to delete directory/file: " << addon_dir << '\n';
		ret = false;
	}

	if(file_exists(addon_dir + ".cfg") && !delete_directory(addon_dir + ".cfg")) {
		messages << "Failed to delete directory/file: " << addon_dir << ".cfg\n";
		ret = false;
	}

	if(log != NULL) {
		*log = messages.str();
	}

	if(!ret) {
		ERR_CFG << "removal of add-on " << addon << " failed:\n" << messages.str();
	}

	return ret;
}

std::vector<std::string> available_addons()
{
	std::vector<std::string> res;
	std::vector<std::string> files, dirs;
	const std::string parentd = get_addon_campaigns_dir();
	get_files_in_dir(parentd,&files,&dirs);

	for(std::vector<std::string>::const_iterator i = dirs.begin(); i != dirs.end(); ++i) {
		const std::string external_cfg_file = *i + ".cfg";
		const std::string internal_cfg_file = *i + "/_main.cfg";
		const std::string external_pbl_file = *i + ".pbl";
		const std::string internal_pbl_file = *i + "/_server.pbl";
		if((std::find(files.begin(),files.end(),external_cfg_file) != files.end() || file_exists(parentd + "/" + internal_cfg_file)) &&
		   (std::find(files.begin(),files.end(),external_pbl_file) != files.end() || (file_exists(parentd + "/" + internal_pbl_file)))) {
			res.push_back(*i);
		}
	}
	for(std::vector<std::string>::const_iterator i = files.begin(); i != files.end(); ++i) {
		const size_t length = i->size() - 4;
		if (i->rfind(".cfg", length) != length) continue;
		const std::string name = i->substr(0, length);
		// Continue if there is a dir (which we already processed).
		if (std::find(dirs.begin(), dirs.end(), name) != dirs.end()) continue;
		if (std::find(files.begin(), files.end(), name + ".pbl") != files.end()) {
			res.push_back(name);
		}
	}

	return res;
}

std::vector<std::string> installed_addons()
{
	std::vector<std::string> res;
	const std::string parentd = get_addon_campaigns_dir();
	std::vector<std::string> files, dirs;
	get_files_in_dir(parentd,&files,&dirs);

	for(std::vector<std::string>::const_iterator i = dirs.begin(); i != dirs.end(); ++i) {
		const std::string external_cfg_file = *i + ".cfg";
		const std::string internal_cfg_file = *i + "/_main.cfg";
		if(std::find(files.begin(),files.end(),external_cfg_file) != files.end() || file_exists(parentd + "/" + internal_cfg_file)) {
			res.push_back(*i);
		}
	}

	return res;
}

static inline bool IsCR(const char& c)
{
	return c == '\x0D';
}

static std::string strip_cr(std::string str, bool strip)
{
	if(!strip)
		return str;
	std::string::iterator new_end = std::remove_if(str.begin(), str.end(), IsCR);
	str.erase(new_end, str.end());
	return str;
}

namespace {
	void append_default_ignore_patterns(std::pair<std::vector<std::string>, std::vector<std::string> >& patterns)
	{
		std::vector<std::string>& files = patterns.first;
		std::vector<std::string>& dirs  = patterns.second;

		/* Don't upload dot-files/dirs, which are hidden files in
		   UNIX platforms */
		files.push_back(".*");
		dirs.push_back(".*");
		/* MacOS X metadata-like cruft (http://floatingsun.net/2007/02/07/whats-with-__macosx-in-zip-files/) */
		dirs.push_back("__MACOSX");

		files.push_back("#*#");
		files.push_back("*~");
		files.push_back("*-bak");
		files.push_back("*.swp");
		files.push_back("*.pbl");
		files.push_back("*.ign");
		files.push_back("_info.cfg");
		files.push_back("*.exe");
		files.push_back("*.bat");
		files.push_back("*.cmd");
		files.push_back("*.com");
		files.push_back("*.scr");
		files.push_back("*.sh");
		files.push_back("*.js");
		files.push_back("*.vbs");
		files.push_back("*.o");
		/* Remove junk created by certain file manager ;) */
		files.push_back("Thumbs.db");
    }
}

static std::pair<std::vector<std::string>, std::vector<std::string> > read_ignore_patterns(const std::string& addon_name)
{
	const std::string parentd = get_addon_campaigns_dir();
	const std::string exterior = parentd + "/" + addon_name + ".ign";
	const std::string interior = parentd + "/" + addon_name + "/_server.ign";

	std::pair<std::vector<std::string>, std::vector<std::string> > patterns;
	std::string ign_file;
	LOG_CFG << "searching for .ign file for '" << addon_name << "'...\n";
	if (file_exists(interior)) {
		ign_file = interior;
	} else if (file_exists(exterior)) {
		ign_file = exterior;
	} else {
		LOG_CFG << "no .ign file found for '" << addon_name << "'\n"
		        << "inserting default ignore patterns...\n";
		append_default_ignore_patterns(patterns);
		return patterns; // just default patterns
	}
	LOG_CFG << "found .ign file: " << ign_file << '\n';
	std::istream *stream = istream_file(ign_file);
	std::string line;
	while (std::getline(*stream, line)) {
		utils::strip(line);
		const size_t l = line.size();
		if (line[l - 1] == '/') { // directory; we strip the last /
			patterns.second.push_back(line.substr(0, l - 1));
		} else { // file
			patterns.first.push_back(line);
		}
	}
	return patterns;
}

static void archive_file(const std::string& path, const std::string& fname, config& cfg)
{
	cfg["name"] = fname;
	const bool is_cfg = (fname.size() > 4 ? (fname.substr(fname.size() - 4) == ".cfg") : false);
	cfg["contents"] = encode_binary(strip_cr(read_file(path + '/' + fname),is_cfg));
}

inline bool looks_like_pbl(const std::string& file)
{
	return utils::wildcard_string_match(utils::lowercase(file), "*.pbl");
}

static void archive_dir(const std::string& path, const std::string& dirname, config& cfg, std::pair<std::vector<std::string>, std::vector<std::string> >& ignore_patterns)
{
	cfg["name"] = dirname;
	const std::string dir = path + '/' + dirname;

	std::vector<std::string> files, dirs;
	get_files_in_dir(dir,&files,&dirs);
	for(std::vector<std::string>::const_iterator i = files.begin(); i != files.end(); ++i) {
		bool valid = !looks_like_pbl(*i);
		for(std::vector<std::string>::const_iterator p = ignore_patterns.first.begin(); p != ignore_patterns.first.end(); ++p) {
			if (utils::wildcard_string_match(*i, *p)) {
				valid = false;
				break;
			}
		}
		if (valid) {
			archive_file(dir,*i,cfg.add_child("file"));
		}
	}

	for(std::vector<std::string>::const_iterator j = dirs.begin(); j != dirs.end(); ++j) {
		bool valid = true;
		for(std::vector<std::string>::const_iterator p = ignore_patterns.second.begin(); p != ignore_patterns.second.end(); ++p) {
			if (utils::wildcard_string_match(*j, *p)) {
				valid = false;
				break;
			}
		}
		if (valid) {
			archive_dir(dir,*j,cfg.add_child("dir"),ignore_patterns);
		}
	}
}

void archive_addon(const std::string& addon_name, config& cfg)
{
	const std::string parentd = get_addon_campaigns_dir();

	std::pair<std::vector<std::string>, std::vector<std::string> > ignore_patterns;
	// External .cfg may not exist; newer campaigns have a _main.cfg
	const std::string external_cfg = addon_name + ".cfg";
	if (file_exists(parentd + "/" + external_cfg)) {
		archive_file(parentd, external_cfg, cfg.add_child("file"));
	}
	ignore_patterns = read_ignore_patterns(addon_name);
	archive_dir(parentd, addon_name, cfg.add_child("dir"), ignore_patterns);
}

static void unarchive_file(const std::string& path, const config& cfg)
{
	write_file(path + '/' + cfg["name"].str(), unencode_binary(cfg["contents"]));
}

static void unarchive_dir(const std::string& path, const config& cfg)
{
	std::string dir;
	if (cfg["name"].empty())
		dir = path;
	else
		dir = path + '/' + cfg["name"].str();

	make_directory(dir);

	BOOST_FOREACH (const config &d, cfg.child_range("dir")) {
		unarchive_dir(dir, d);
	}

	BOOST_FOREACH (const config &f, cfg.child_range("file")) {
		unarchive_file(dir, f);
	}
}

void unarchive_addon(const config& cfg)
{
	const std::string parentd = get_addon_campaigns_dir();
	unarchive_dir(parentd, cfg);
}

namespace {
	/** Class to handle showing add-on descriptions. */
	class display_description : public gui::dialog_button_action
	{
		display& disp_;
		std::vector<std::string> titles_, desc_;
		gui::filter_textbox* filter_;

	public:
		display_description(display& disp, std::vector<std::string> const& titles, std::vector<std::string> const& descriptions, gui::filter_textbox* filter)
			: disp_(disp)
			, titles_(titles)
			, desc_(descriptions)
			, filter_(filter)
		{}

		virtual gui::dialog_button_action::RESULT button_pressed(int filter_choice)
		{
			assert(filter_ != NULL);
			const int menu_selection = filter_->get_index(filter_choice);
			if(menu_selection < 0) { return gui::CONTINUE_DIALOG; }
			size_t const uchoice = static_cast<size_t>(menu_selection);

			std::string text;
			std::string title;

			if(uchoice >= desc_.size()) {
				text = _("No description available.");
			}
			else {
				title = titles_[uchoice];
				text = desc_[uchoice];
			}

			gui2::show_transient_message(disp_.video(), title, text);

			return gui::CONTINUE_DIALOG;
		}
	};


	/**
	 * Strip the ".cfg" extension and replace "_" with " " for display.
	 *
	 * @param files      List of files in the add-ons directory.
	 * @param dirs       List of subdirectories in the add-ons directory.
	 * @param parent_dir Path to the add-ons directory.
	 */
	void prepare_addons_list_for_display(std::vector<std::string>& files,
	                                     std::vector<std::string>& dirs,
	                                     const std::string& parent_dir)
	{
		std::vector<std::string>::iterator i = files.begin();
		while(i != files.end())
		{
			const std::string::size_type pos = i->rfind(".cfg", i->size());
			if(pos == std::string::npos) {
				i = files.erase(i);
			} else {
				i->erase(pos);
				// remove it from the directory list too
				for(std::vector<std::string>::iterator j = dirs.begin(); j != dirs.end() ; ++j) {
					if (*i == *j) {
						dirs.erase(j);
						break;
					}
				};
				std::replace(i->begin(), i->end(), '_', ' ');
				i++;
			}
		}
		// process items of type Addon/_main.cfg too
		i = dirs.begin();
		while(i != dirs.end())
		{
			if (file_exists(parent_dir + *i + "/_main.cfg")) {
				std::replace(i->begin(), i->end(), '_', ' ');
				files.push_back(*i);
				i++;
			} else {
				i = dirs.erase(i);
			}
		}
	}

	/**
	 * Creates a more human-readable representation of a file size.
	 *
	 * @param size_str File size string, as obtained from a config object.
	 *
	 * @returns        Representation of file size in the biggest byte multiply
	 *                 possible.
	 */
	static std::string format_file_size(const std::string& size_str)
	{
		double size = lexical_cast_default<double>(size_str,0.0);

		const double k = 1024;
		if(size > 0.0) {
			std::string size_postfix = _("B");	// bytes
			if(size > k) {
				size /= k;
				size_postfix = _("KB");			// kilobytes
				if(size > k) {
					size /= k;
					size_postfix = _("MB");		// megabytes
					if(size > k) {
						size /= k;
						size_postfix = _("GB");	// gigabytes
					}
				}
			}

			std::ostringstream stream;
#ifdef _MSC_VER
			// Visual C++ makes 'precision' set the number of decimal places.
			// Other platforms make it set the number of significant figures
			stream.precision(1);
			stream << std::fixed << size << size_postfix;
#else
			if (size < 100) stream.precision(3);
			else size = static_cast<int>(size);
			stream << size << size_postfix;
#endif
			return stream.str();
		} else {
			return "";
		}
	}

	/**
	 * Return a short string describing an add-on's type.
	 *
	 * @param type Numerical add-on type.
	 *
	 * @return     A string, translated to the current locale.
	 */
	std::string get_translatable_addon_type(ADDON_TYPE type)
	{
		switch (type) {
			case ADDON_SP_CAMPAIGN:
				return _("addon_type^Campaign");
			case ADDON_SP_SCENARIO:
				return _("addon_type^Scenario");
			case ADDON_MP_ERA:
				return _("addon_type^MP era");
			case ADDON_MP_FACTION:
				return _("addon_type^MP faction");
			case ADDON_MP_MAPS:
				return _("addon_type^MP map-pack");
			case ADDON_MP_SCENARIO:
				return _("addon_type^MP scenario");
			case ADDON_MP_CAMPAIGN:
				return _("addon_type^MP campaign");
			case ADDON_MEDIA:
				return _("addon_type^Resources");
			case ADDON_OTHER:
				return _("addon_type^Other");
			default:
				return _("addon_type^(unknown)");
		}
	}

	/**
	 * Checks if an add-on's dependencies are met.
	 *
	 * @param disp    Object to be used for displaying interactive messages.
	 * @param deplist List of dependencies (add-on identifiers).
	 *
	 * @returns       true if dependencies are met; false otherwise.
	 */
	bool addon_dependencies_met(game_display &disp,
		const std::vector<std::string> &deplist, const std::string &addon_title)
	{
		const std::vector<std::string>& installed = installed_addons();
		std::vector<std::string>::const_iterator i;
		std::string missing = "";
		size_t count_missing = 0;

		BOOST_FOREACH(const std::string& i, deplist) {
			if (std::find(installed.begin(), installed.end(), i) == installed.end()) {
				missing += "\n" + i;
				++count_missing;
			}
		}
		// If there are any, display a message.
		// TODO: Somehow offer to automatically download
		// the missing dependencies.
		if (!missing.empty()) {
			const std::string msg_title    = _("Dependencies");
			utils::string_map symbols;
			symbols["addon_title"] = addon_title;
			const std::string msg_entrytxt = utils::interpolate_variables_into_string(
				_n("$addon_title depends upon the following add-on which you have not installed yet:",
				"$addon_title depends upon the following add-ons which you have not installed yet:",
				count_missing), &symbols);
			std::string msg_reminder = utils::interpolate_variables_into_string(_("Do you still want to download $addon_title|? (You will have to install all the dependencies in order to play.)"), &symbols);

			if(!(gui2::show_message(disp.video()
					, msg_title
					, msg_entrytxt + "\n \n" + missing + "\n \n" + msg_reminder
					, gui2::tmessage::ok_cancel_buttons) == gui2::twindow::OK)) {

				return false;
			}
		}
		return true;
	}

	void upload_addon_to_server(game_display& disp, const std::string& addon, network::connection sock)
	{
		config request_terms;
		request_terms.add_child("request_terms");
		network::send_data(request_terms, sock, true);
		config data;
		sock = network::receive_data(data,sock,5000);
		if(!sock) {
			gui2::show_error_message(disp.video(), _("Connection timed out"));
			return;
		} else if (const config &c = data.child("error")) {
			std::string error_message = _("The server responded with an error: \"$error|\"");
			utils::string_map symbols;
			symbols["error"] = c["message"].str();
			error_message = utils::interpolate_variables_into_string(error_message, &symbols);
			gui2::show_error_message(disp.video(), error_message);
			return;
		} else if (const config &c = data.child("message")) {

			if(gui2::show_message(disp.video()
					,_("Terms")
					, c["message"]
					, gui2::tmessage::ok_cancel_buttons) != gui2::twindow::OK) {

				return;
			}
		}

		config cfg;
		get_addon_info(addon,cfg);

		std::string passphrase = cfg["passphrase"];
		// generate a random passphrase and write it to disk
		// if the .pbl file doesn't provide one already
		if(passphrase.empty()) {
			passphrase.resize(8);
			for(size_t n = 0; n != 8; ++n) {
				passphrase[n] = 'a' + (rand()%26);
			}
			cfg["passphrase"] = passphrase;
			set_addon_info(addon,cfg);
		}

		cfg["name"] = addon;

		config addon_data;
		archive_addon(addon,addon_data);

		data.clear();
		data.add_child("upload",cfg).add_child("data",addon_data);

		LOG_NET << "uploading add-on...\n";
		// @todo Should be enabled once the campaign server can be recompiled.
		network::send_data(data, sock, true);

		sock = dialogs::network_send_dialog(disp,_("Sending add-on"),data,sock);
		if(!sock) {
			return;
		} else if (const config &c = data.child("error")) {
			gui2::show_error_message(disp.video(), _("The server responded with an error: \"") +
			                        c["message"].str() + '"');
		} else if (const config &c = data.child("message")) {
			gui2::show_transient_message(disp.video(), _("Response"), c["message"]);
		}
	}

	void delete_remote_addon(game_display& disp, const std::string& addon, network::connection sock)
	{
		config cfg;
		get_addon_info(addon,cfg);

		config msg;
		msg["name"] = addon;
		msg["passphrase"] = cfg["passphrase"];

		config data;
		data.add_child("delete",msg);

		network::send_data(data, sock, true);

		sock = network::receive_data(data,sock,5000);
		if(!sock) {
			gui2::show_error_message(disp.video(), _("Connection timed out"));
		} else if (const config &c = data.child("error")) {
			gui2::show_error_message(disp.video(), _("The server responded with an error: \"") +
			                        c["message"].str() + '"');
		} else if (const config &c = data.child("message")) {
			gui2::show_transient_message(disp.video(), _("Response"), c["message"]);
		}
	}

	bool install_addon(game_display& disp, config const& addons_tree,
	                   const std::string& addon_id, const std::string& addon_title,
	                   const std::string& addon_type_str, const std::string& addon_uploads_str,
	                   const std::string& addon_version_str,
	                   const network::manager& /*net_manager*/,
	                   const network::connection& sock, bool* do_refresh,
	                   bool show_result = true)
	{
		// Get all dependencies of the addon/campaign selected for download.
		const config &selected_campaign = addons_tree.find_child("campaign", "name", addon_id);
		assert(selected_campaign);
		// Get all dependencies which are not already installed.
		// TODO: Somehow determine if the version is outdated.
		std::vector<std::string> dependencies = utils::split(selected_campaign["dependencies"]);
		if (!addon_dependencies_met(disp, dependencies, addon_title)) return false;
		// Proceed to download and install
		config request;
		request.add_child("request_campaign")["name"] = addon_id;
		network::send_data(request, sock, true);

		utils::string_map syms;
		syms["addon_title"] = addon_title;
		const std::string& download_dlg_title =
			utils::interpolate_variables_into_string(_("Downloading add-on: $addon_title|..."), &syms);

		// WML structure where the add-on archive, or any error messages, are stored.
		config cfg;
		network::connection res = dialogs::network_receive_dialog(disp, download_dlg_title, cfg, sock);
		if(!res) {
			return false;
		}

		if (config const &dlerror = cfg.child("error")) {
			gui2::show_error_message(disp.video(), dlerror["message"]);
			return false;
		}

		if(!check_names_legal(cfg)) {
			gui2::show_error_message(disp.video(), _("The add-on has an invalid file or directory name and can not be installed."));
			return false;
		}

		// add revision info to the addon archive
		config *maindir = &cfg.find_child("dir", "name", addon_id);
		if (!*maindir) {
			LOG_CFG << "downloaded addon '" << addon_id << "' is missing its own directory, creating...\n";
			maindir = &cfg.add_child("dir");
			(*maindir)["name"] = addon_id;
		}

		LOG_CFG << "generating version info for addon '" << addon_id << "'\n";
		config f;
		f["name"] = "_info.cfg";

		utils::string_map info_tab;
		info_tab["type"] = !addon_type_str.empty() ? addon_type_str : std::string("unknown");
		info_tab["uploads"] = addon_uploads_str;
		info_tab["version"] = addon_version_str;

		static std::string const info_template =
			"#\n"
			"# File automatically generated by Wesnoth to keep track\n"
			"# of version information on installed add-ons. DO NOT EDIT!\n"
			"#\n"
			"[info]\n"
			"    type=\"$type\"\n"
			"    uploads=\"$uploads\"\n"
			"    version=\"$version\"\n"
			"[/info]\n";
		std::string const info_file_contents =
			utils::interpolate_variables_into_string(info_template, &info_tab);
		f["contents"] = info_file_contents;

		maindir->add_child("file", f);
		LOG_CFG << "generated version info, unpacking...\n";

		// remove any existing versions of the just downloaded add-on,
		// assuming it consists of a dir and a cfg file
		if(!remove_local_addon(addon_id)) {
			WRN_CFG << "failed to uninstall existing add-on version before installing; add-on may not work properly\n";
		}

		unarchive_addon(cfg);
		LOG_CFG << "addon unpacked successfully\n";

		if(show_result) {
			const std::string& message =
				utils::interpolate_variables_into_string(_("The add-on '$addon_title|' has been successfully installed."), &syms);
			gui2::show_transient_message(disp.video(), _("Add-on Installed"), message);
		}

		if(do_refresh != NULL)
			*do_refresh = true;
		return true;
	}

	void do_addon_icon_fixups(std::string& icon, const std::string& addon_name)
	{
		static const std::string default_icon = "misc/blank-hex.png";
		static const std::string debug_default_icon = "misc/missing-image.png";

		if(icon.empty()) {
			ERR_CFG << "remote add-on '" << addon_name << "' has no icon\n";
			icon = default_icon;
		}
		else if(!image::exists(icon)) {
			ERR_CFG << "remote add-on '" << addon_name << "' has an icon which cannot be found: '" << icon << "'\n";
			if(game_config::debug) {
				icon = debug_default_icon;
			}
			else {
				icon = default_icon;
			}
		}
		else if(icon.find("units/") != std::string::npos && icon.find_first_of('~') == std::string::npos) {
			// a hack to prevent magenta icons, because they look awful
			WRN_CFG << "remote add-on '" << addon_name << "' has an icon without TC/RC specification\n";
			icon.append("~RC(magenta>red)");
		}
	}

	void addons_update_dlg(game_display &disp, config &cfg, const config::const_child_itors &remote_addons_list,
	                       const network::manager& net_manager, const network::connection& sock,
	                       bool* do_refresh)
	{
		std::vector<const config *> remote_matches_cfgs;
		std::vector< std::string > safe_matches;
		std::vector< std::string > unsafe_matches;
		std::ostringstream unsafe_list;
		const std::vector< std::string >& all_local = installed_addons();
		// Add-ons that can be published and are outdated will not be offered for update,
		// but a message will be displayed warning about them to the user.
		const std::vector< std::string >& all_publish = available_addons();
		std::vector<version_info> safe_local_versions;
		std::vector<version_info> unsafe_local_versions;
		std::map<std::string, version_info> remote_version_map;
		BOOST_FOREACH (const config &remote_addon, remote_addons_list)
		{
			if(remote_addon == NULL) continue; // shouldn't happen...
			const std::string& name = remote_addon["name"];
			const std::string& version = remote_addon["version"];
			try {
				remote_version_map.insert(std::make_pair(name, version_info(version)));
			} catch(version_info::not_sane_exception const&) {
				ERR_CFG << "remote add-on '" << name << "' has invalid version string '" << version << "', skipping from updates check...\n";
				continue;
			}
			std::vector<std::string>::const_iterator local_match =
					std::find(all_local.begin(), all_local.end(), name);
			try {
				if(local_match != all_local.end()) {
					const version_info& local_version = get_addon_version_info(name);
					if(remote_version_map[name] > local_version) {
						if(std::find(all_publish.begin(), all_publish.end(), name) != all_publish.end()) {
							unsafe_matches.push_back(name);
							unsafe_local_versions.push_back(local_version);
							unsafe_list << '\n';
							unsafe_list << name << " (local: " << local_version.str() << ", remote: " << version << ")";
						} else {
							safe_matches.push_back(name);
							safe_local_versions.push_back(local_version);
							remote_matches_cfgs.push_back(&remote_addon);
						}
					}
				}
			} catch(version_info::not_sane_exception const&) {
				ERR_CFG << "local add-on '" << name << "' has invalid or no version info, skipping from updates check...\n";
				continue;
			}
		}
		if(!unsafe_matches.empty()) {
			const std::string warn_title = _("Outdated add-ons");
			const std::string warn_entrytxt = _n(
				"An outdated local add-on has publishing information attached. It will not be offered for updating.",
				"Some outdated local add-ons have publishing information attached. They will not be offered for updating.",
				unsafe_matches.size());

			gui2::show_transient_message(disp.video()
					, warn_title
					, warn_entrytxt + unsafe_list.str());
		}

		if(safe_matches.empty()) {

			gui2::show_transient_message(disp.video()
					, _("No add-ons to update")
					, _("Could not find any updated add-ons on this server."));
			return;
		}

		// column contents
		std::vector<std::string> addons, titles, oldversions, newversions, options, filtered_opts;
		std::vector<int> sizes;

		std::vector<std::string> types, uploads;

		std::string sep(1, COLUMN_SEPARATOR);
		const std::string& heading =
			(formatter() << HEADING_PREFIX << sep << _("Name") << sep << _("Old version") << sep << _("New version") << sep
			             << _("Author") << sep << _("Size")).str();
		options.push_back(heading);
		filtered_opts.push_back(heading);

		assert(safe_matches.size() == safe_local_versions.size());
		assert(safe_matches.size() == remote_matches_cfgs.size());
		for(size_t i = 0; i < safe_matches.size(); ++i) {
			const config& c = *(remote_matches_cfgs[i]);

			types.push_back(c["type"]);
			uploads.push_back(c["uploads"]);

			const std::string& name = c["name"];
			const std::string& size = c["size"];
			const std::string& sizef = format_file_size(size);
			const std::string& oldver = safe_local_versions[i];
			const std::string& newver = remote_version_map[name];

			std::string author = c["author"];
			std::string title = c["title"];
			if(title.empty()) {
				title = name;
				std::replace(title.begin(), title.end(), '_', ' ');
			}

			utils::truncate_as_wstring(title, 20);
			utils::truncate_as_wstring(author, 16);

			//add negative sizes to reverse the sort order
			sizes.push_back(-atoi(size.c_str()));

			addons.push_back(name);
			titles.push_back(title);
			oldversions.push_back(oldver);
			newversions.push_back(newver);

			std::string icon = c["icon"];
			do_addon_icon_fixups(icon, name);

			const std::string text_columns =
				title + COLUMN_SEPARATOR +
				oldver + COLUMN_SEPARATOR +
				newver + COLUMN_SEPARATOR +
				author + COLUMN_SEPARATOR +
				sizef + COLUMN_SEPARATOR;
			options.push_back(IMAGE_PREFIX + icon + COLUMN_SEPARATOR + text_columns);
			filtered_opts.push_back(text_columns);
		}

		// Create dialog
		gui::dialog upd_dialog(disp,
			_("Update add-ons"),
			_("Select an add-on to update:"), gui::OK_CANCEL);

		// Create widgets
		gui::menu::basic_sorter sorter;
		sorter.set_alpha_sort(1).set_alpha_sort(2).set_alpha_sort(3).set_alpha_sort(4).set_position_sort(5,sizes);

		gui::menu::imgsel_style addon_style(gui::menu::bluebg_style);
		addon_style.scale_images(font::relative_size(72), font::relative_size(72));

		gui::menu* addon_menu =
			new gui::menu(disp.video(), options, false, -1,
			              gui::dialog::max_menu_width, &sorter,
			              &addon_style, false);
		gui::dialog_button* update_all_button =
			new gui::dialog_button(disp.video(), _("Update all"),
			                       gui::button::TYPE_PRESS, -255);
// FIXME: dunno if it was because I was testing with 0 elements (besides
// the header) or I'm doing it wrong, but this causes SIGSEGV. Better let
// someone with the patience and knowledge required to deal with filter_textbox
// do it instead
//
// 		gui::filter_textbox* filter =
// 			new gui::filter_textbox(disp.video(),
// 			                        _("Filter: "), options, filtered_opts, 1, upd_dialog, 300);

		// Add widgets
		upd_dialog.add_button(update_all_button, gui::dialog::BUTTON_EXTRA);
		upd_dialog.set_menu(addon_menu);
// 		upd_dialog.set_textbox(filter);

		// Activate
		int index = upd_dialog.show();
		const bool upd_all = index == -255;
// 		index = filter->get_index(index);
		// FIXME: until it is possible to integrate this functionality and download_addons()
		// in a single dialog, we'll resort to a magic result to detect the "all" choice.
		if(index < 0 && !upd_all)
			return;

		bool result = true;
		std::vector<std::string> failed_titles;

		assert(cfg.child("campaigns"));

		if(upd_all) {
			for(size_t i = 0; i < addons.size() && i < remote_matches_cfgs.size(); ++i)
			{
				if (!install_addon(disp, cfg.child("campaigns"), addons[i], titles[i],
				                   types[i], uploads[i], newversions[i], net_manager, sock,
				                   do_refresh, false)) {
					result=false;
					failed_titles.push_back(titles[i]);
				}
			}
		} else {
			const size_t i = static_cast<size_t>(index);
			if (!install_addon(disp, cfg.child("campaigns"), addons[i], titles[i],
				               types[i], uploads[i], newversions[i], net_manager, sock,
				               do_refresh, false)) {
				result=false;
				failed_titles.push_back(titles[i]);
			}
		}

		if(!result) {
			assert(failed_titles.empty() == false);
			std::string failed_titles_list_fmt;
			BOOST_FOREACH(const std::string& entry, failed_titles) {
				failed_titles_list_fmt += '\n';
				failed_titles_list_fmt += entry;
			}
			const std::string err_title = _("Update failed");
			const std::string err_message =
				_n("The following add-on could not be downloaded or updated successfully:",
				   "The following add-ons could not be downloaded or updated successfully:",
				   failed_titles.size()) + failed_titles_list_fmt;

			gui2::show_message(disp.video(), err_title, err_message);
			return;
		}

		const std::string msg_title = _("Update succeeded");
		const std::string msg_message = !upd_all ? _("Add-on updated successfully.") :
		                                           _("All add-ons updated successfully.");

		gui2::show_message(disp.video(), msg_title, msg_message);
	}

	void download_addons(game_display& disp, const std::string& remote_address,
			bool update_mode, bool* do_refresh, int old_index = 0)
	{
		const std::vector<std::string> address_components =
			utils::split(remote_address, ':');
		if(address_components.empty()) {
			return;
		}

		const std::string old_host = preferences::campaign_server();
		const int remote_port = lexical_cast_default<int>(address_components.back(),
		                                                  default_campaignd_port);
		std::string remote_host = address_components.front();
		preferences::set_campaign_server(remote_address);

		try {
			const network::manager net_manager;
			const network::connection sock =
				dialogs::network_connect_dialog(disp, _("Connecting to add-ons server..."),
				                                remote_host, remote_port);
			if(!sock) {
				gui2::show_error_message(disp.video(), _("Could not connect to host."));
				preferences::set_campaign_server(old_host);
				return;
			}

			config cfg;
			cfg.add_child("request_campaign_list");
			network::send_data(cfg, sock, true);
			network::connection res = dialogs::network_receive_dialog(disp, _("Requesting list of add-ons"), cfg, sock);
			if(!res) {
				return;
			}

			if (config const &error = cfg.child("error")) {
				gui2::show_error_message(disp.video(), error["message"]);
				return;
			}

			config const &addons_tree = cfg.child("campaigns");
			if (!addons_tree) {
				gui2::show_error_message(disp.video(), _("An error occurred while communicating with the server."));
				return;
			}

			const config::const_child_itors &addon_cfgs = addons_tree.child_range("campaign");
			if(update_mode) {
				addons_update_dlg(disp, cfg, addon_cfgs, net_manager, sock, do_refresh);
				return;
			}

			// column contents
			std::vector<std::string> addons, titles, versions, uploads, types, options, options_to_filter, descriptions;
			std::vector<int> sizes;

			std::string sep(1, COLUMN_SEPARATOR);
			const std::string& heading =
				(formatter() << HEADING_PREFIX << sep << _("Name") << sep << _("Version") << sep
				             << _("Author") << sep << _("Type") << sep << _("Downloads") << sep << _("Size")).str();
			options.push_back(heading);
			options_to_filter.push_back(heading);

			const std::vector< std::string >& publish_options = available_addons();

			std::vector< std::string > delete_options;

			BOOST_FOREACH(const config &c, addon_cfgs)
			{
				const std::string& name = c["name"];
				const std::string& downloads = c["downloads"].str();
				const std::string& size = c["size"];
				const std::string& sizef = format_file_size(size);
				const std::string& type_str = c["type"];
				const ADDON_TYPE type = get_addon_type(type_str);
				const std::string& type_label_str = get_translatable_addon_type(type);

				addons.push_back(name);
				versions.push_back(c["version"]);
				uploads.push_back(c["uploads"]);
				descriptions.push_back(c["description"]);
				types.push_back(type_str);

				if(std::count(publish_options.begin(), publish_options.end(), name) != 0) {
					delete_options.push_back(name);
				}

				std::string title = c["title"];
				if(title == "") {
					title = name;
					std::replace(title.begin(),title.end(),'_',' ');
				}
				titles.push_back(title);

				std::string version = c["version"], author = c["author"];

				//add negative sizes to reverse the sort order
				sizes.push_back(-atoi(size.c_str()));

				std::string icon = c["icon"];
				do_addon_icon_fixups(icon, name);

				std::string text_columns =
					title + COLUMN_SEPARATOR +
					version + COLUMN_SEPARATOR +
					author + COLUMN_SEPARATOR +
					type_label_str + COLUMN_SEPARATOR +
					downloads + COLUMN_SEPARATOR +
					sizef + COLUMN_SEPARATOR;

				// icon paths shouldn't be filtered!
				options_to_filter.push_back(text_columns + descriptions.back());

				utils::truncate_as_wstring(title, 20);
				utils::truncate_as_wstring(version, 12);
				utils::truncate_as_wstring(author, 16);

				text_columns =
					IMAGE_PREFIX + icon + COLUMN_SEPARATOR +
					title + COLUMN_SEPARATOR +
					version + COLUMN_SEPARATOR +
					author + COLUMN_SEPARATOR +
					type_label_str + COLUMN_SEPARATOR +
					downloads + COLUMN_SEPARATOR +
					sizef + COLUMN_SEPARATOR;

				options.push_back(text_columns);
			}

			std::string pub_option_text, del_option_text;

			BOOST_FOREACH(const std::string& pub, publish_options) {
				static const std::string publish_icon = "icons/icon-addon-publish.png";
				const std::string text = _("Publish add-on: ") + pub;
				options.push_back(IMAGE_PREFIX + publish_icon + COLUMN_SEPARATOR + font::GOOD_TEXT + text);
				options_to_filter.push_back(text);
			}
			BOOST_FOREACH(const std::string& del, delete_options) {
				static const std::string delete_icon = "icons/icon-addon-delete.png";
				const std::string text = _("Delete add-on: ") + del;
				options.push_back(IMAGE_PREFIX + delete_icon + COLUMN_SEPARATOR + font::BAD_TEXT + text);
				options_to_filter.push_back(text);
			}

			if(addons.empty() && publish_options.empty()) {
				gui2::show_error_message(disp.video(), _("There are no add-ons available for download from this server."));
				return;
			}

			int index = -1;
			if(gui2::new_widgets) {
				gui2::taddon_list dlg(addons_tree);
				dlg.show(disp.video());
			} else {

				gui::menu::basic_sorter sorter;
				sorter.set_alpha_sort(1).set_alpha_sort(2).set_alpha_sort(3).set_alpha_sort(4).set_numeric_sort(5).set_position_sort(6,sizes);

				gui::dialog addon_dialog(disp, _("Get add-ons"),
										 _("Choose the add-on to download."),
										 gui::OK_CANCEL);
				gui::menu::imgsel_style addon_style(gui::menu::bluebg_style);

				//make sure the icon isn't too big
				addon_style.scale_images(font::relative_size(72), font::relative_size(72));
				gui::menu *addon_menu = new gui::menu(disp.video(), options, false, -1,
													  gui::dialog::max_menu_width, &sorter,
													  &addon_style, false);
				addon_dialog.set_menu(addon_menu);

				gui::filter_textbox* filter = new gui::filter_textbox(disp.video(),
				_("Filter: "), options, options_to_filter, 1, addon_dialog, 300);
				addon_dialog.set_textbox(filter);

				display_description description_helper(disp, titles, descriptions, filter);

				gui::dialog_button* description = new gui::dialog_button(disp.video(), _("Description"), gui::button::TYPE_PRESS, gui::CONTINUE_DIALOG, &description_helper);
				addon_dialog.add_button(description, gui::dialog::BUTTON_EXTRA);

				// Scroll the menu to the previous selection
				addon_menu->move_selection(old_index);

				index = addon_dialog.show();
				index = filter->get_index(index);
			}

			if(index < 0) {
				return;
			}

			// Handle deletion option
			if(index >= int(addons.size() + publish_options.size())) {
				const std::string& addon = delete_options[index - int(addons.size() + publish_options.size())];
				delete_remote_addon(disp, addon, sock);
				return;
			}

			// Handle publish option
			if(index >= int(addons.size())) {
				const std::string& addon = publish_options[index - int(addons.size())];
				upload_addon_to_server(disp, addon, sock);
				return;
			}

			// Handle download
			install_addon(disp, addons_tree, addons[index], titles[index], types[index],
			              uploads[index], versions[index], net_manager, sock, do_refresh);

			// Show the dialog again, and position it on the same item installed
			download_addons(disp, remote_address, update_mode, do_refresh, index);

		} catch(config::error& e) {
			ERR_CFG << "config::error thrown during transaction with add-on server; \""<< e.message << "\"\n";
			gui2::show_error_message(disp.video(), _("Network communication error."));
		} catch(network::error& e) {
			ERR_NET << "network::error thrown during transaction with add-on server; \""<< e.message << "\"\n";
			gui2::show_error_message(disp.video(), _("Remote host disconnected."));
		} catch(io_exception& e) {
			ERR_FS << "io_exception thrown while installing an addon; \"" << e.what() << "\"\n";
			gui2::show_error_message(disp.video(), _("A problem occurred when trying to create the files necessary to install this add-on."));
		} catch(twml_exception& e) {
			e.show(disp);
		}
	}

	void uninstall_local_addons(game_display& disp, bool* should_reload_cfg)
	{
		std::vector<std::string> addons;
		std::vector<std::string> addon_dirs;

		const std::string parentdir = get_addon_campaigns_dir() + "/";

		get_files_in_dir(parentdir, &addons, &addon_dirs, FILE_NAME_ONLY);
		prepare_addons_list_for_display(addons, addon_dirs, parentdir);

		if (addons.empty()) {
			gui2::show_error_message(disp.video(),
				_("You have no add-ons installed."));
			return;
		}

		gui::menu::basic_sorter sorter;
		sorter.set_alpha_sort(1);

		int index = 0;
		int res;

		do {
			gui::dialog addon_dialog(disp,
							_("Uninstall add-ons"), _("Choose the add-on to remove."),
							gui::OK_CANCEL);
			gui::menu::imgsel_style &addon_style = gui::menu::bluebg_style;

			gui::menu *addon_menu = new gui::menu(disp.video(), addons, false, -1,
					gui::dialog::max_menu_width, &sorter, &addon_style, false);
			addon_dialog.set_menu(addon_menu);
			index = addon_dialog.show();

			if(index < 0)
				return;

			std::string confirm_message = _("Are you sure you want to remove the add-on '$addon|'?");
			utils::string_map symbols;
			symbols["addon"] = addons.at(index);
			confirm_message = utils::interpolate_variables_into_string(confirm_message, &symbols);

			res = gui2::show_message(disp.video()
					, _("Confirm")
					, confirm_message
					, gui2::tmessage::yes_no_buttons);
		} while (res != gui2::twindow::OK);

		// Put underscores back in the name
		std::string addon_id = addons.at(index);
		std::replace(addon_id.begin(), addon_id.end(), ' ', '_');

		// Try to remove add-on and report results
		std::string removal_log;
		if(remove_local_addon(addon_id, &removal_log))
		{
			std::string message = _("Add-on '$addon|' deleted.");
			utils::string_map symbols;
			symbols["addon"] = addons.at(index);
			message = utils::interpolate_variables_into_string(message, &symbols);
			gui2::show_transient_message(disp.video()
					, _("Add-on deleted")
					, message);

			if(should_reload_cfg != NULL)
				*should_reload_cfg = true;
		}
		else
		{
			std::string err_msg = _("Add-on could not be deleted properly:");
			err_msg += '\n';
			err_msg += removal_log;
			gui2::show_error_message(disp.video(), err_msg);
		}
	}


	const int addon_download  = 0;
	const int addon_uninstall = 2;  // NOTE this value is also known by WML so don't change it.
	const int addon_update    = 3;  // NOTE this value is also known by WML so don't change it.

} // end unnamed namespace 4

void manage_addons(game_display& disp)
{
	int res;
	bool do_refresh = false;
	std::string remote_host;
	const std::string default_host = preferences::campaign_server();
	const bool have_addons = !installed_addons().empty();

	gui2::taddon_connect addon_dlg;

	addon_dlg.set_host_name(default_host);
	addon_dlg.set_allow_remove(have_addons);
	addon_dlg.set_allow_updates(have_addons);
	addon_dlg.show(disp.video());

	res = addon_dlg.get_retval();
	remote_host = addon_dlg.host_name();

	if(res == gui2::twindow::OK) {
		res = addon_download;
	}

	switch(res) {
		case addon_update:
		case addon_download:
			download_addons(disp, remote_host, res==addon_update, &do_refresh);
			break;
		case addon_uninstall:
			uninstall_local_addons(disp, &do_refresh);
			break;
		default:
			return;
	}
	// Signal game_controller to reload WML
	if(do_refresh) {
		throw config_changed_exception();
	}
}

namespace {
	std::map< std::string, version_info > version_info_cache;
} // end unnamed namespace 5

void refresh_addon_version_info_cache()
{
	if(version_info_cache.empty() != true)
		version_info_cache.clear();

	LOG_CFG << "probing add-ons and refreshing version information cache...\n";

	const std::vector<std::string>& addons = installed_addons();
	if(addons.empty()) {
		LOG_CFG << "no add-ons found\n";
		return;
	}
	static const std::string parentd = get_addon_campaigns_dir();
	std::vector<std::string> addon_info_files;
	BOOST_FOREACH(std::string const& dir, addons)
		addon_info_files.push_back(parentd+"/"+dir+"/_info.cfg");

	size_t i = 0;

	BOOST_FOREACH(std::string const& info_file, addon_info_files) {
		assert(i < addons.size());

		std::string const& addon = addons[i];
		++i;

		if(file_exists(info_file)) {
			scoped_istream stream = istream_file(info_file);
			config cfg;
			read(cfg, *stream);

			config const &info_cfg = cfg.child("info");
			if (!info_cfg) {
				continue;
			}
			std::string const& version = info_cfg["version"];
			LOG_CFG << "caching add-on version info: " << addon << " [" << version << "]\n";
			version_info_cache.insert(std::make_pair(addon, version_info(version)));
		}
		// Don't print the warning if the user is clearly the author
		else if (!file_exists(parentd+"/"+addon+".pbl")
			  && !file_exists(parentd+"/"+addon+"/_server.pbl")
			  && !file_exists(parentd+"/"+addon+"/.svn")
			  && !file_exists(parentd+"/"+addon+"/.git")) {
				WRN_CFG << "add-on '" << addon << "' has no _info.cfg; cannot read version info\n";
		}
	}
}

const version_info& get_addon_version_info(const std::string& addon)
{
	static const version_info nil(0,0,0,false);
	std::map< std::string, version_info >::iterator entry = version_info_cache.find(addon);
	return entry != version_info_cache.end() ? entry->second : nil;
}
