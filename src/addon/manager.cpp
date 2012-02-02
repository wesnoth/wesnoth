/* $Id$ */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
                 2008 - 2012 by Ignacio R. Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "addon/manager.hpp"
#include "dialogs.hpp"
#include "filesystem.hpp"
#include "foreach.hpp"
#include "formatter.hpp"
#include "game_display.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "gui/dialogs/addon_connect.hpp"
#include "gui/dialogs/addon_list.hpp"
#include "gui/dialogs/addon/description.hpp"
#include "gui/dialogs/addon/uninstall_list.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/network_transmission.hpp"
#include "gui/dialogs/simple_item_selector.hpp"
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

bool have_addon_in_vcs_tree(const std::string& addon_name)
{
	static const std::string parentd = get_addon_campaigns_dir();
	return
		file_exists(parentd+"/"+addon_name+"/.svn") ||
		file_exists(parentd+"/"+addon_name+"/.git") ||
		file_exists(parentd+"/"+addon_name+"/.hg");
}

bool have_addon_pbl_info(const std::string& addon_name)
{
	static const std::string parentd = get_addon_campaigns_dir();
	return
		file_exists(parentd+"/"+addon_name+".pbl") ||
		file_exists(parentd+"/"+addon_name+"/_server.pbl");
}

bool get_addon_info(const std::string& addon_name, config& cfg)
{
	const std::string parentd = get_addon_campaigns_dir();

	// Cope with old-style or new-style file organization
	const std::string exterior = parentd + "/" + addon_name + ".pbl";
	const std::string interior = parentd + "/" + addon_name + "/_server.pbl";
	const bool is_old_style = file_exists(exterior);
	const std::string pbl_file = (is_old_style ? exterior : interior);

	scoped_istream stream = istream_file(pbl_file);
	read(cfg, *stream);
	return is_old_style;
}

void set_addon_info(const std::string& addon_name, const config& cfg, const bool is_old_style)
{
	const std::string parentd = get_addon_campaigns_dir();
	scoped_ostream stream = ostream_file(parentd + "/" + addon_name + "/_server.pbl");
	if(is_old_style) stream = ostream_file(parentd + "/" + addon_name + ".pbl");
	write(*stream, cfg);
}

bool remove_local_addon(const std::string& addon)
{
	bool ret = true;
	const std::string addon_dir = get_addon_campaigns_dir() + "/" + addon;

	LOG_CFG << "removing local add-on: " << addon << '\n';

	if(file_exists(addon_dir) && !delete_directory(addon_dir, true)) {
		ERR_CFG << "Failed to delete directory/file: " << addon_dir << '\n';
		ret = false;
	}

	if(file_exists(addon_dir + ".cfg") && !delete_directory(addon_dir + ".cfg", true)) {
		ERR_CFG << "Failed to delete directory/file: " << addon_dir << ".cfg\n";
		ret = false;
	}

	if(!ret) {
		ERR_CFG << "removal of add-on " << addon << " failed!\n";
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
        /* Eclipse plugin */
        files.push_back("*.wesnoth");
        files.push_back("*.project");
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

	foreach (const config &d, cfg.child_range("dir")) {
		unarchive_dir(dir, d);
	}

	foreach (const config &f, cfg.child_range("file")) {
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
		std::vector<addon_info> infov_;
		gui::filter_textbox* filter_;

	public:
		display_description(display& disp, std::vector<addon_info> const& infov, gui::filter_textbox* filter)
			: disp_(disp)
			, infov_(infov)
			, filter_(filter)
		{}

		virtual gui::dialog_button_action::RESULT button_pressed(int filter_choice)
		{
			assert(filter_ != NULL);
			const int menu_selection = filter_->get_index(filter_choice);
			if(menu_selection < 0) { return gui::CONTINUE_DIALOG; }
			size_t const uchoice = static_cast<size_t>(menu_selection);

			if(uchoice < infov_.size()) {
				gui2::taddon_description::display(
						  infov_[uchoice]
						, disp_.video());
			}

			return gui::CONTINUE_DIALOG;
		}
	};


	/**
	 * Strip the ".cfg" extension..
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
				++i;
			}
		}
		// process items of type Addon/_main.cfg too
		i = dirs.begin();
		while(i != dirs.end())
		{
			if (file_exists(parent_dir + *i + "/_main.cfg")) {
				files.push_back(*i);
				++i;
			} else {
				i = dirs.erase(i);
			}
		}
	}

	/**
	 * Creates a more human-readable representation of a file size.
	 *
	 * @returns        Representation of file size in the biggest byte multiply
	 *                 possible.
	 */
	static std::string format_file_size(double size)
	{
		if(size > 0.0) {
			return utils::si_string(size, true, _("unit_byte^B"));
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

	/** Replaces underscores to dress up file or dirnames as add-on names.
	 *
	 * @todo In the future we should store more local information about add-ons and use
	 *       this only as a fallback; it could be desirable to fetch translated names as well
	 *       somehow.
	 */
	std::string get_addon_name(const std::string& id)
	{
		std::string retv(id);
		std::replace(retv.begin(), retv.end(), '_', ' ');
		return retv;
	}

	void upload_addon_to_server(game_display& disp, const std::string& addon, network_asio::connection& connection)
	{
		config request;
		request.add_child("request_terms");
		config response;
		connection.transfer(request, response);
		gui2::tnetwork_transmission request_terms_dialog(connection, _("Requesting terms"), "");
		bool result = request_terms_dialog.show(disp.video());
		if(!result) return;

		if (const config &c = response.child("error")) {
			std::string error_message = _("The server responded with an error: \"$error|\"");
			utils::string_map symbols;
			symbols["error"] = c["message"].str();
			error_message = utils::interpolate_variables_into_string(error_message, &symbols);
			gui2::show_error_message(disp.video(), error_message);
			return;
		} else if (const config &c = response.child("message")) {

			if(gui2::show_message(disp.video()
					,_("Terms")
					, c["message"]
					, gui2::tmessage::ok_cancel_buttons) != gui2::twindow::OK) {

				return;
			}
		}

		config cfg;
		const bool is_old_style = get_addon_info(addon,cfg);

		std::string passphrase = cfg["passphrase"];
		// generate a random passphrase and write it to disk
		// if the .pbl file doesn't provide one already
		if(passphrase.empty()) {
			passphrase.resize(8);
			for(size_t n = 0; n != 8; ++n) {
				passphrase[n] = 'a' + (rand()%26);
			}
			cfg["passphrase"] = passphrase;
			set_addon_info(addon,cfg,is_old_style);
		}

		cfg["name"] = addon;

		config addon_data;
		archive_addon(addon,addon_data);

		request.clear();
		response.clear();
		request.add_child("upload",cfg).add_child("data",addon_data);

		LOG_NET << "uploading add-on...\n";
		connection.transfer(request, response);
		gui2::tnetwork_transmission upload_dialog(connection, _("Sending add-on"), "", true);
		result = upload_dialog.show(disp.video());
		if(!result) return;

		if (const config &c = response.child("error")) {
			gui2::show_error_message(disp.video(), _("The server responded with an error: \"") +
			                        c["message"].str() + '"');
		} else if (const config &c = response.child("message")) {
			gui2::show_transient_message(disp.video(), _("Response"), c["message"]);
		}
	}

	void delete_remote_addon(game_display& disp, const std::string& addon, network_asio::connection& connection)
	{
		config cfg;
		get_addon_info(addon,cfg);

		config msg;
		msg["name"] = addon;
		msg["passphrase"] = cfg["passphrase"];

		config request, response;
		request.add_child("delete",msg);

		connection.transfer(request, response);
		gui2::tnetwork_transmission network_connect(connection, _("Requesting the addon to be deleted"), "");
		bool result = network_connect.show(disp.video());
		if(!result) return;

		if (const config &c = response.child("error")) {
			gui2::show_error_message(disp.video(), _("The server responded with an error: \"") +
			                        c["message"].str() + '"');
		} else if (const config &c = response.child("message")) {
			gui2::show_transient_message(disp.video(), _("Response"), c["message"]);
		}
	}

	bool install_addon(game_display& disp,
	                   const std::string& addon_id, const std::string& addon_title,
	                   const std::string& addon_type_str, const std::string& addon_uploads_str,
	                   const std::string& addon_version_str,
	                   network_asio::connection& connection, bool* do_refresh,
	                   bool show_result = true)
	{
		// Proceed to download and install
		config request;
		request.add_child("request_campaign")["name"] = addon_id;

		utils::string_map syms;
		syms["addon_title"] = addon_title;
		const std::string& download_dlg_title =
			utils::interpolate_variables_into_string(_("Downloading add-on: $addon_title|..."), &syms);

		// WML structure where the add-on archive, or any error messages, are stored.
		config cfg;

		connection.transfer(request, cfg);
		gui2::tnetwork_transmission network_connect(connection, download_dlg_title, _("Downloading..."));
		bool result = network_connect.show(disp.video());
		if(!result) {
			return false;
		}

		if (config const &dlerror = cfg.child("error")) {
			gui2::show_error_message(disp.video(), dlerror["message"]);
			return false;
		}

		if(!check_names_legal(cfg)) {
			gui2::show_error_message(disp.video(), _("The add-on has an invalid file or directory name and cannot be installed."));
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
		static const std::string debug_default_icon = game_config::images::missing;

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
			LOG_CFG << "remote add-on '" << addon_name << "' has an icon without TC/RC specification\n";
			icon.append("~RC(magenta>red)");
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
	bool addon_dependencies_met(game_display &disp, config const& addons_tree,
		const std::string &addon_id,
		network_asio::connection& connection,
	        bool* do_refresh)
	{
		const config &selected_campaign = addons_tree.find_child("campaign", "name", addon_id);
		assert(selected_campaign);
		// Get all dependencies which are not already installed.
		// TODO: Somehow determine if the version is outdated.
		std::vector<std::string> dependencies = utils::split(selected_campaign["dependencies"]);
		const std::vector<std::string>& installed = installed_addons();
		std::vector<std::string>::const_iterator i;
		std::string missing = "";
		size_t count_missing = 0;

		foreach(const std::string& i, dependencies) {
			if (std::find(installed.begin(), installed.end(), i) == installed.end()) {
				missing += "\n" + i;
				++count_missing;
			}
		}
		// If there are any, display a message.
		// TODO: Somehow offer to automatically download
		// the missing dependencies.
		if (!missing.empty()) {
			const config::const_child_itors &remote_addons_list = addons_tree.child_range("campaign");
			std::vector<const config *> remote_matches_cfgs;
			std::vector< std::string > safe_matches;
			std::vector< std::string > unsafe_matches;
			std::ostringstream unsafe_list;
			std::map<std::string, version_info> remote_version_map;
			foreach (const config &remote_addon, remote_addons_list)
			{
				const std::string& name = remote_addon["name"];
				if (std::find(dependencies.begin(), dependencies.end(), name) != dependencies.end()) {
					const std::string& version = remote_addon["version"];
					try {
						remote_version_map.insert(std::make_pair(name, version_info(version)));
					} catch(version_info::not_sane_exception const&) {
						ERR_CFG << "remote add-on '" << name << "' has invalid version string '" << version << "', skipping from updates check...\n";
						continue;
					}
					std::vector<std::string>::const_iterator local_match =
							std::find(installed.begin(), installed.end(), name);
					if(local_match == installed.end()) {
						safe_matches.push_back(name);
						remote_matches_cfgs.push_back(&remote_addon);
					}
				}
			}
			if(!safe_matches.empty()) {
				// column contents
				std::vector<std::string> addons, titles, versions, options, filtered_opts;
				std::vector<int> sizes;

				std::vector<std::string> types, uploads;

				std::string sep(1, COLUMN_SEPARATOR);
				const std::string& heading =
					(formatter() << HEADING_PREFIX << sep << _("Name") << sep << _("Version") << sep
					             << _("Author") << sep << _("Type") << sep << _("Size")).str();
				options.push_back(heading);
				filtered_opts.push_back(heading);

				assert(safe_matches.size() == remote_matches_cfgs.size());
				for(size_t i = 0; i < safe_matches.size(); ++i) {
					const config& c = *(remote_matches_cfgs[i]);

					uploads.push_back(c["uploads"]);

					const std::string& type = c["type"];
					const std::string& name = c["name"];
					int size = c["size"];
					std::string sizef = format_file_size(size);
					const std::string& version = remote_version_map[name];

					std::string author = c["author"];
					std::string title = c["title"];
					if(title.empty()) {
						title = name;
						std::replace(title.begin(), title.end(), '_', ' ');
					}

					utils::truncate_as_wstring(title, 20);
					utils::truncate_as_wstring(author, 16);

					//add negative sizes to reverse the sort order
					sizes.push_back(-size);

					types.push_back(type);
					titles.push_back(title);
					addons.push_back(name);
					versions.push_back(version);

					std::string icon = c["icon"];
					do_addon_icon_fixups(icon, name);

					const std::string text_columns =
						title + COLUMN_SEPARATOR +
						version + COLUMN_SEPARATOR +
						author + COLUMN_SEPARATOR +
						type + COLUMN_SEPARATOR +
						sizef + COLUMN_SEPARATOR;
					options.push_back(IMAGE_PREFIX + icon + COLUMN_SEPARATOR + text_columns);
					filtered_opts.push_back(text_columns);
				}

				// Create dialog
				gui::dialog upd_dialog(disp,
					_("Install dependencies"),
					_n("The selected add-on has the following dependency. Do you want to install it?",
					   "The selected add-on has the following dependencies. Do you want to install them?",
					   safe_matches.size()
					  ), gui::YES_NO);

				gui::menu* addon_menu =
					new gui::menu(disp.video(), options, false, -1,
				              gui::dialog::max_menu_width, NULL, NULL, false);

				// Add widgets
				upd_dialog.set_menu(addon_menu);

				// Activate
				int index = upd_dialog.show();
				if(index < 0)
					return true;

				bool result = true;
				std::vector<std::string> failed_titles;

				for(size_t i = 0; i < addons.size() && i < remote_matches_cfgs.size(); ++i)
				{
					if (!install_addon(disp, addons[i], titles[i],
			       		            types[i], uploads[i], versions[i], connection,
			               		    do_refresh, false)) {
						result=false;
						failed_titles.push_back(titles[i]);
					} else {
						if (!addon_dependencies_met(disp, addons_tree, addons[i], connection, do_refresh)) {
							const std::string err_title = _("Installation of a dependency failed");
							const std::string err_message =
								_("While the add-on has been installed, a dependency is missing. Try to update the installed add-ons.");

							gui2::show_message(disp.video(), err_title, err_message);
						}
					}
				}

				if(!result) {
					assert(failed_titles.empty() == false);
					std::string failed_titles_list_fmt;
					foreach(const std::string& entry, failed_titles) {
						failed_titles_list_fmt += '\n';
						failed_titles_list_fmt += entry;
					}
					const std::string err_title = _("Installation failed");
					const std::string err_message =
								_n("The following add-on could not be downloaded or updated successfully:",
								   "The following add-ons could not be downloaded or updated successfully:",
								   failed_titles.size()) + failed_titles_list_fmt;

					gui2::show_message(disp.video(), err_title, err_message);
					return true;
				}
			}
		}
		return true;
	}

	void addons_update_dlg(game_display &disp, config const& addons_tree, const config::const_child_itors &remote_addons_list,
	                       network_asio::connection& connection,
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
		foreach (const config &remote_addon, remote_addons_list)
		{
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
			int size = c["size"];
			std::string sizef = format_file_size(size);
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
			sizes.push_back(-size);

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

		if(upd_all) {
			for(size_t i = 0; i < addons.size() && i < remote_matches_cfgs.size(); ++i)
			{
				if (!install_addon(disp, addons[i], titles[i],
				                   types[i], uploads[i], newversions[i], connection,
				                   do_refresh, false)) {
					result=false;
					failed_titles.push_back(titles[i]);
				} else {
					if (!addon_dependencies_met(disp, addons_tree, addons[i], connection, do_refresh)) {
						const std::string err_title = _("Installation of some dependency failed");
						const std::string err_message =
							_("While the add-on has been installed, some dependency is missing. Try to update the installed add-ons.");

						gui2::show_message(disp.video(), err_title, err_message);
					}
				}
			}
		} else {
			const size_t i = static_cast<size_t>(index);
			if (!install_addon(disp, addons[i], titles[i],
				               types[i], uploads[i], newversions[i], connection,
				               do_refresh, false)) {
				result=false;
				failed_titles.push_back(titles[i]);
			} else {
				if (!addon_dependencies_met(disp, addons_tree, addons[i], connection, do_refresh)) {
					const std::string err_title = _("Installation of some dependency failed");
					const std::string err_message =
						_("While the add-on has been installed, some dependency is missing. Try to update the installed add-ons.");

					gui2::show_message(disp.video(), err_title, err_message);
				}
			}
		}

		if(!result) {
			assert(failed_titles.empty() == false);
			std::string failed_titles_list_fmt;
			foreach(const std::string& entry, failed_titles) {
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

	bool check_whether_overwrite(game_display& disp,
		const std::string& addon,
		const std::vector<std::string>& own_addons)
	{
		foreach(const std::string& current_own_addon, own_addons) {
			if(current_own_addon == addon) {
				utils::string_map symbols;
				symbols["addon"] = addon;
				const std::string& confirm_message = utils::interpolate_variables_into_string(
					_("You seem to be the author of '$addon|'. Downloading '$addon|' may overwrite any changes you have made since the last upload and may delete your pbl file. Do you really wish to continue?"),
					&symbols);
				const int res = gui2::show_message(disp.video(),
					_("Confirm"),
					confirm_message,
					gui2::tmessage::yes_no_buttons);
				if(res == gui2::twindow::OK) return true;
				else return false;
			}
		}
		return true;
	}

	void download_addons(game_display& disp, const std::string& remote_address,
			bool update_mode, bool* do_refresh, int old_index = 0)
	{
		const std::vector<std::string> address_components =
			utils::split(remote_address, ':');
		if(address_components.empty()) {
			return;
		}

		const std::string remote_port = address_components.size() == 2
				? address_components[1]
				: lexical_cast<std::string>(default_campaignd_port);

		std::string remote_host = address_components[0];
		preferences::set_campaign_server(remote_address);

		try {
			network_asio::connection connection(remote_host, remote_port);
			gui2::tnetwork_transmission network_connect(connection, _("Requesting list of add-ons"), _("Connecting..."));
			bool result = network_connect.show(disp.video());
			if(!result)
				return;
			config request, response;
			request.add_child("request_campaign_list");
			connection.transfer(request, response);
			network_connect.set_subtitle(_("Downloading..."));
			result = network_connect.show(disp.video());
			if(!result)
				return;

			if (config const &error = response.child("error")) {
				gui2::show_error_message(disp.video(), error["message"]);
				return;
			}

			config const &addons_tree = response.child("campaigns");
			if (!addons_tree) {
				gui2::show_error_message(disp.video(), _("An error occurred while communicating with the server."));
				return;
			}

			const config::const_child_itors &addon_cfgs = addons_tree.child_range("campaign");
			if(update_mode) {
				addons_update_dlg(disp, addons_tree, addon_cfgs, connection, do_refresh);
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

			std::vector< addon_info > infos;

			foreach(const config &c, addon_cfgs)
			{
				const std::string& name = c["name"];
				const std::string& downloads = c["downloads"].str();
				int size = c["size"];
				std::string sizef = format_file_size(size);
				const std::string& type_str = c["type"];
				const ADDON_TYPE type = get_addon_type(type_str);
				const std::string& type_label_str = get_translatable_addon_type(type);

				addon_info inf;

				inf.sizestr = sizef;

				addons.push_back(name);
				versions.push_back(c["version"]);
				uploads.push_back(c["uploads"]);
				descriptions.push_back(c["description"]);

				inf.description = c["description"].str();

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

				inf.name = title;

				std::string version = c["version"], author = c["author"];

				inf.version = version;
				inf.author = author;

				//add negative sizes to reverse the sort order
				sizes.push_back(-size);

				std::string icon = c["icon"];
				do_addon_icon_fixups(icon, name);

				inf.icon = icon;

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

				config::const_child_itors const& linguas = c.child_range("translation");
				for(config::const_child_iterator i = linguas.first; i != linguas.second; ++i) {
					inf.translations.push_back((*i)["language"]);
				}

				infos.push_back(inf);
			}

			foreach(const std::string& pub, publish_options) {
				static const std::string publish_icon = "icons/icon-addon-publish.png";
				const std::string text = _("Publish add-on: ") + get_addon_name(pub);
				options.push_back(IMAGE_PREFIX + publish_icon + COLUMN_SEPARATOR + font::GOOD_TEXT + text);
				options_to_filter.push_back(text);
			}
			foreach(const std::string& del, delete_options) {
				static const std::string delete_icon = "icons/icon-addon-delete.png";
				const std::string text = _("Delete add-on: ") + get_addon_name(del);
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

				display_description description_helper(disp, infos, filter);

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

				utils::string_map symbols;
				symbols["addon"] = get_addon_name(addon);

				const std::string& confirm_message = utils::interpolate_variables_into_string(
					_("Deleting '$addon|' will permanently erase its download and upload counts on the add-ons server. Do you really wish to continue?"),
					&symbols);

				const int res = gui2::show_message(disp.video(),
					_("Confirm"), confirm_message, gui2::tmessage::yes_no_buttons);

				if(res == gui2::twindow::OK) {
					delete_remote_addon(disp, addon, connection);
				}
			}
			// Handle publish option
			else if(index >= int(addons.size())) {
				const std::string& addon = publish_options[index - int(addons.size())];
				upload_addon_to_server(disp, addon, connection);
			}
			// Handle download option
			else if(check_whether_overwrite(disp, addons[index], publish_options))
			{
				// Handle download
				install_addon(disp, addons[index], titles[index], types[index],
							  uploads[index], versions[index], connection, do_refresh);
				if (!addon_dependencies_met(disp, addons_tree, addons[index], connection, do_refresh)) {
					const std::string err_title = _("Installation of some dependency failed");
					const std::string err_message =
						_("While the add-on has been installed, some dependency is missing. Try to update the installed add-ons.");

					gui2::show_message(disp.video(), err_title, err_message);
				}
			}

			// Show the dialog again, and position it on the last selected item
			download_addons(disp, remote_address, update_mode, do_refresh, index);

		} catch(config::error& e) {
			ERR_CFG << "config::error thrown during transaction with add-on server; \""<< e.message << "\"\n";
			gui2::show_error_message(disp.video(), _("Network communication error."));
		} catch(network::error& e) {
			ERR_NET << "network::error thrown during transaction with add-on server; \""<< e.message << "\"\n";
			gui2::show_error_message(disp.video(), _("Remote host disconnected."));
		} catch(const network_asio::error& e) {
			ERR_NET << "network_asio::error thrown during transaction with add-on server; \""<< e.what() << "\"\n";
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
		static const std::string list_lead = "\n\n";
		static const std::string list_sep = "\n";

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

		int res;

		std::vector<std::string> remove_ids, remove_names;

		do {
			gui2::taddon_uninstall_list dlg(addons);
			dlg.show(disp.video());

			remove_ids = dlg.selected_addons();
			if(remove_ids.empty()) {
				return;
			}

			remove_names.clear();

			foreach(const std::string& id, remove_ids) {
				remove_names.push_back(get_addon_name(id));
			}

			const std::string confirm_message = _n(
				"Are you sure you want to remove the following installed add-on?",
				"Are you sure you want to remove the following installed add-ons?",
				remove_ids.size()) + list_lead + utils::join(remove_names, list_sep);

			res = gui2::show_message(disp.video()
					, _("Confirm")
					, confirm_message
					, gui2::tmessage::yes_no_buttons);
		} while (res != gui2::twindow::OK);

		std::vector<std::string> failed_names, skipped_names, succeeded_names;

		foreach(const std::string& id, remove_ids) {
			const std::string& name = get_addon_name(id);

			if(have_addon_pbl_info(id) || have_addon_in_vcs_tree(id)) {
				skipped_names.push_back(name);
			} else if(remove_local_addon(id)) {
				succeeded_names.push_back(name);
			} else {
				failed_names.push_back(name);
			}
		}

		if(!skipped_names.empty()) {
			const std::string dlg_msg = _n(
				"The following add-on appears to have publishing or version control information stored locally, and will not be removed:",
				"The following add-ons appear to have publishing or version control information stored locally, and will not be removed:",
				skipped_names.size());

			gui2::show_error_message(
				disp.video(), dlg_msg + list_lead + utils::join(skipped_names, list_sep));
		}

		if(!failed_names.empty()) {
			gui2::show_error_message(disp.video(), _n(
				"The following add-on could not be deleted properly:",
				"The following add-ons could not be deleted properly:",
				failed_names.size()) + list_lead + utils::join(failed_names, list_sep));
		}

		if(!succeeded_names.empty()) {
			const std::string dlg_title =
				_n("Add-on Deleted", "Add-ons Deleted", succeeded_names.size());
			const std::string dlg_msg = _n(
				"The following add-on was successfully deleted:",
				"The following add-ons were successfully deleted:",
				succeeded_names.size());

			gui2::show_transient_message(
				disp.video(), dlg_title,
				dlg_msg + list_lead + utils::join(succeeded_names, list_sep));

			if(should_reload_cfg != NULL) {
				*should_reload_cfg = true;
			}
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
	std::string host_name = preferences::campaign_server();
	const bool have_addons = !installed_addons().empty();

	gui2::taddon_connect addon_dlg(host_name, have_addons, have_addons);

	addon_dlg.show(disp.video());

	res = addon_dlg.get_retval();

	if(res == gui2::twindow::OK) {
		res = addon_download;
	}

	switch(res) {
		case addon_update:
		case addon_download:
			download_addons(disp, host_name, res==addon_update, &do_refresh);
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
	foreach(std::string const& dir, addons)
		addon_info_files.push_back(parentd+"/"+dir+"/_info.cfg");

	size_t i = 0;

	foreach(std::string const& info_file, addon_info_files) {
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
			std::string const version = info_cfg["version"];
			LOG_CFG << "caching add-on version info: " << addon << " [" << version << "]\n";
			version_info_cache.insert(std::make_pair(addon, version_info(version)));
		}
		// Don't print the warning if the user is clearly the author
		else if (!have_addon_pbl_info(addon) && !have_addon_in_vcs_tree(addon)) {
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
