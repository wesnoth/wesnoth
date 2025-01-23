/*
	Copyright (C) 2008 - 2024
	by Iris Morelle <shadowm2006@gmail.com>
	Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "addon/info.hpp"
#include "addon/manager.hpp"
#include "addon/state.hpp"
#include "addon/validation.hpp"
#include "cursor.hpp"
#include "font/pango/escape.hpp"
#include "formula/string_utils.hpp"
#include "game_config.hpp"
#include "gettext.hpp"
#include "gui/dialogs/addon/addon_auth.hpp"
#include "gui/dialogs/addon/install_dependencies.hpp"
#include "gui/dialogs/file_progress.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/widgets/retval.hpp"
#include "log.hpp"
#include "preferences/preferences.hpp"
#include "serialization/string_utils.hpp"
#include "serialization/utf8_exception.hpp"
#include "utils/parse_network_address.hpp"

#include <stdexcept>

#include "addon/client.hpp"

static lg::log_domain log_addons_client("addons-client");
#define ERR_ADDONS LOG_STREAM(err ,  log_addons_client)
#define WRN_ADDONS LOG_STREAM(warn,  log_addons_client)
#define LOG_ADDONS LOG_STREAM(info,  log_addons_client)
#define DBG_ADDONS LOG_STREAM(debug, log_addons_client)

using gui2::dialogs::network_transmission;

addons_client::addons_client(const std::string& address)
	: addr_(address)
	, host_()
	, port_()
	, conn_(nullptr)
	, last_error_()
	, last_error_data_()
	, server_id_()
	, server_version_()
	, server_capabilities_()
	, server_url_()
	, license_notice_()
{
	try {
		std::tie(host_, port_) = parse_network_address(addr_, std::to_string(default_campaignd_port));
	} catch(const std::runtime_error&) {
		throw invalid_server_address();
	}
}

void addons_client::connect()
{
	LOG_ADDONS << "connecting to server " << host_ << " on port " << port_;

	utils::string_map i18n_symbols;
	i18n_symbols["server_address"] = addr_;

	conn_.reset(new network_asio::connection(host_, port_));

	const auto& msg = VGETTEXT("Connecting to $server_address|...", i18n_symbols);

	wait_for_transfer_done(msg, transfer_mode::connect);

	config response_buf;

	send_simple_request("server_id", response_buf);
	wait_for_transfer_done(msg);

	if(!is_error_response(response_buf)) {
		if(auto info = response_buf.optional_child("server_id")) {
			server_id_ = info["id"].str();
			server_version_ = info["version"].str();

			for(const auto& cap : utils::split(info["cap"].str())) {
				server_capabilities_.insert(cap);
			}

			server_url_ = info["url"].str();
			license_notice_ = info["license_notice"].str();
		}
	} else {
		clear_last_error();
	}

	if(server_version_.empty()) {
		// An educated guess
		server_capabilities_ = { "auth:legacy" };
	}

	const std::string version_desc = server_version_.empty() ? "<1.15.7 or earlier>" : server_version_;
	const std::string id_desc = server_id_.empty() ? "<id not provided>" : server_id_;

	LOG_ADDONS << "Server " << id_desc << " version " << version_desc
			   << " supports: " << utils::join(server_capabilities_, " ");
}

std::map<std::string, int> addons_client::get_addon_count_by_type()
{
	std::map<std::string, int> counts;

	config response;
	config request;
	request.add_child("addon_count_by_type");

	send_request(request, response);
	wait_for_transfer_done(_("Requesting add-on counts by type..."));

	for(const auto& attr : response.attribute_range()) {
		counts[attr.first] = attr.second.to_int();
	}

	if(is_error_response(response)) {
		gui2::show_error_message(_("The server responded with an error:") + "\n" + get_last_server_error());
		return counts;
	}

	return counts;
}

config addons_client::get_addon_downloads_by_version(const std::string& addon)
{
	config response;
	config request;
	config& child = request.add_child("addon_downloads_by_version");
	child["addon"] = addon;

	send_request(request, response);
	wait_for_transfer_done(_("Requesting add-on downloads by version..."));

	if(is_error_response(response)) {
		gui2::show_error_message(_("The server responded with an error:") + "\n" + get_last_server_error());
		config dummy;
		return dummy;
	}

	return response;
}

config addons_client::get_forum_auth_usage()
{
	config response;
	config request;
	request.add_child("forum_auth_usage");

	send_request(request, response);
	wait_for_transfer_done(_("Requesting forum_auth usage..."));

	if(is_error_response(response)) {
		gui2::show_error_message(_("The server responded with an error:") + "\n" + get_last_server_error());
		config dummy;
		return dummy;
	}

	return response;
}

config addons_client::get_addon_admins()
{
	config response;
	config request;
	request.add_child("admins_list");

	send_request(request, response);
	wait_for_transfer_done(_("Requesting list of admins..."));

	if(is_error_response(response)) {
		gui2::show_error_message(_("The server responded with an error:") + "\n" + get_last_server_error());
		config dummy;
		return dummy;
	}

	return response;
}

config addons_client::get_hidden_addons(const std::string& username, const std::string& passphrase)
{
	config response;
	config request;
	config& child = request.add_child("list_hidden");
	child["username"] = username;
	child["passphrase"] = passphrase;

	send_request(request, response);
	wait_for_transfer_done(_("Getting list of hidden add-ons..."));

	if(is_error_response(response)) {
		gui2::show_error_message(_("The server responded with an error:") + "\n" + get_last_server_error());
		config dummy;
		return dummy;
	}

	return response;
}

bool addons_client::hide_addon(const std::string& addon, const std::string& username, const std::string& passphrase)
{
	config response;
	config request;
	config& child = request.add_child("hide_addon");
	child["addon"] = addon;
	child["username"] = username;
	child["passphrase"] = passphrase;

	send_request(request, response);
	wait_for_transfer_done(_("Hiding add-on..."));

	if(is_error_response(response)) {
		gui2::show_error_message(_("The server responded with an error:") + "\n" + get_last_server_error());
		return false;
	}

	return true;
}

bool addons_client::unhide_addon(const std::string& addon, const std::string& username, const std::string& passphrase)
{
	config response;
	config request;
	config& child = request.add_child("unhide_addon");
	child["addon"] = addon;
	child["username"] = username;
	child["passphrase"] = passphrase;

	send_request(request, response);
	wait_for_transfer_done(_("Unhiding add-on..."));

	if(is_error_response(response)) {
		gui2::show_error_message(_("The server responded with an error:") + "\n" + get_last_server_error());
		return false;
	}

	return true;
}

bool addons_client::request_addons_list(config& cfg, bool icons)
{
	cfg.clear();

	config request;
	config& req_child = request.add_child("request_campaign_list");
	req_child["send_icons"] = icons;

	config response_buf;

	/** @todo FIXME: get rid of this legacy "campaign"/"campaigns" silliness
	 */

	send_request(request, response_buf);
	wait_for_transfer_done(_("Downloading list of add-ons..."));

	std::swap(cfg, response_buf.mandatory_child("campaigns"));

	return !is_error_response(response_buf);
}

bool addons_client::request_distribution_terms(std::string& terms)
{
	if(!license_notice_.empty()) {
		// Server identification supported, we already know the terms so this
		// operation always succeeds without going through the server.
		terms = license_notice_;
		return true;
	}

	terms.clear();

	config response_buf;

	send_simple_request("request_terms", response_buf);
	wait_for_transfer_done(_("Requesting distribution terms..."));

	if(auto msg_cfg = response_buf.optional_child("message")) {
		terms = msg_cfg["message"].str();
	}

	return !is_error_response(response_buf);
}

bool addons_client::upload_addon(const std::string& id, std::string& response_message, config& cfg, bool local_only)
{
	LOG_ADDONS << "preparing to upload " << id;

	response_message.clear();

	utils::string_map i18n_symbols;
	i18n_symbols["addon_title"] = font::escape_text(cfg["title"].str());
	if(i18n_symbols["addon_title"].empty()) {
		i18n_symbols["addon_title"] = font::escape_text(make_addon_title(id));
	}

	if(!addon_name_legal(id)){
		i18n_symbols["addon_id"] = font::escape_text(id);
		last_error_ =
			VGETTEXT("The add-on <i>$addon_title</i> has an invalid id '$addon_id' "
				"and cannot be published.", i18n_symbols);
		return false;
	}

	cfg["name"] = id;

	config addon_data;
	try {
		archive_addon(id, addon_data);
	} catch(const utf8::invalid_utf8_exception&){
		last_error_ =
			VGETTEXT("The add-on <i>$addon_title</i> has a file or directory "
				"containing invalid characters and cannot be published.", i18n_symbols);
		return false;
	}

	std::vector<std::string> badnames;
	if(!check_names_legal(addon_data, &badnames)){
		last_error_ =
			VGETTEXT("The add-on <i>$addon_title</i> has an invalid file or directory "
				"name and cannot be published. "

				"File or directory names may not contain '..' or end with '.' or be longer than 255 characters. "
				"It also may not contain whitespace, control characters, or any of the following characters:\n\n&quot; * / : &lt; &gt; ? \\ | ~"
				, i18n_symbols);
		last_error_data_ = font::escape_text(utils::join(badnames, "\n"));
		return false;
	}
	if(!check_case_insensitive_duplicates(addon_data, &badnames)){
		last_error_ =
			VGETTEXT("The add-on <i>$addon_title</i> contains files or directories with case conflicts. "
				"File or directory names may not be differently-cased versions of the same string.", i18n_symbols);
		last_error_data_ = font::escape_text(utils::join(badnames, "\n"));
		return false;
	}

	if(cfg["forum_auth"].to_bool() && !conn_->using_tls() && !game_config::allow_insecure) {
		last_error_ = VGETTEXT("The connection to the remote server is not secure. The add-on <i>$addon_title</i> cannot be uploaded.", i18n_symbols);
		return false;
	}

	if(addon_icon_too_large(cfg["icon"].str())) {
		last_error_ = VGETTEXT("The file size for the icon for the add-on <i>$addon_title</i> is too large.", i18n_symbols);
		return false;
	}

	if(!local_only) {
		// Try to make an upload pack if it's avaible on the server
		config hashlist, hash_request;
		config& request_body = hash_request.add_child("request_campaign_hash");
		// We're requesting the latest version of an addon, so we may not specify it
		// #TODO: Make a selection of the base version for the update ?
		request_body["name"] = cfg["name"];
		// request_body["from"] = ???
		send_request(hash_request, hashlist);
		wait_for_transfer_done(_("Requesting file index..."));

		// A silent error check
		if(!hashlist.has_child("error")) {
			if(!contains_hashlist(addon_data, hashlist) || !contains_hashlist(hashlist, addon_data)) {
				LOG_ADDONS << "making an update pack for the add-on " << id;
				config updatepack;
				// The client shouldn't send the pack if the server is old due to the previous check,
				// so the server should handle the new format in the `upload` request
				make_updatepack(updatepack, hashlist, addon_data);

				config request_buf, response_buf;
				request_buf.add_child("upload", cfg).append(std::move(updatepack));
				// #TODO: Make a selection of the base version for the update ? ,
				// For now, if it's unspecified we'll use the latest avaible before the upload version
				send_request(request_buf, response_buf);
				wait_for_transfer_done(VGETTEXT("Sending an update pack for the add-on <i>$addon_title</i>...", i18n_symbols
				), transfer_mode::upload);

				if(auto message_cfg = response_buf.optional_child("message")) {
					response_message = message_cfg["message"].str();
					LOG_ADDONS << "server response: " << response_message;
				}

				if(!is_error_response(response_buf))
					return true;
			}
		}
	}
	// If there is an error including an unrecognised request for old servers or no hash data for new uploads we'll just send a full pack

	config request_buf, response_buf;
	request_buf.add_child("upload", cfg).add_child("data", std::move(addon_data));

	LOG_ADDONS << "sending " << id;

	send_request(request_buf, response_buf);
	wait_for_transfer_done(VGETTEXT("Sending add-on <i>$addon_title</i>...", i18n_symbols
	), transfer_mode::upload);

	if(auto message_cfg = response_buf.optional_child("message")) {
		response_message = message_cfg["message"].str();
		LOG_ADDONS << "server response: " << response_message;
	}

	return !is_error_response(response_buf);

}

bool addons_client::delete_remote_addon(const std::string& id, std::string& response_message, const std::set<std::string>& admin_set)
{
	response_message.clear();

	config cfg;
	if(admin_set.empty()) {
		// No point in validating when we're deleting it.
		cfg = get_addon_pbl_info(id, false);
	} else {
		cfg["primary_authors"] = utils::join(admin_set);
	}

	utils::string_map i18n_symbols;
	i18n_symbols["addon_title"] = font::escape_text(cfg["title"].str());
	if(i18n_symbols["addon_title"].empty()) {
		i18n_symbols["addon_title"] = font::escape_text(make_addon_title(id));
	}

	config request_buf, response_buf;
	config& request_body = request_buf.add_child("delete");

	// if the passphrase isn't provided from the _server.pbl, try to pre-populate it from the preferences before prompting for it
	if(cfg["passphrase"].empty()) {
		cfg["passphrase"] = prefs::get().password(prefs::get().campaign_server(), cfg["author"]);
		if(!gui2::dialogs::addon_auth::execute(cfg)) {
			config dummy;
			config& error = dummy.add_child("error");
			error["message"] = "Password not provided.";
			return !is_error_response(dummy);
		} else {
			prefs::get().set_password(prefs::get().campaign_server(), cfg["author"], cfg["passphrase"]);
		}
	}

	request_body["admin"] = admin_set.size() > 0;
	request_body["name"] = id;
	request_body["passphrase"] = cfg["passphrase"];
	// needed in case of forum_auth authentication since the author stored on disk on the server is not necessarily the current primary author
	request_body["uploader"] = cfg["uploader"];

	LOG_ADDONS << "requesting server to delete " << id;

	send_request(request_buf, response_buf);
	wait_for_transfer_done(VGETTEXT("Removing add-on <i>$addon_title</i> from the server...", i18n_symbols));

	if(auto message_cfg = response_buf.optional_child("message")) {
		response_message = message_cfg["message"].str();
		LOG_ADDONS << "server response: " << response_message;
	}

	return !is_error_response(response_buf);
}

bool addons_client::download_addon(config& archive_cfg, const std::string& id, const std::string& title, const version_info& version, bool increase_downloads)
{
	archive_cfg.clear();

	config request_buf;
	config& request_body = request_buf.add_child("request_campaign");

	request_body["name"] = id;
	request_body["increase_downloads"] = increase_downloads;
	request_body["version"] = version.str();
	request_body["from_version"] = get_addon_version_info(id);

	utils::string_map i18n_symbols;
	i18n_symbols["addon_title"] = font::escape_text(title);

	LOG_ADDONS << "downloading " << id;

	send_request(request_buf, archive_cfg);
	wait_for_transfer_done(VGETTEXT("Downloading add-on <i>$addon_title</i>...", i18n_symbols));

	return !is_error_response(archive_cfg);
}

bool addons_client::install_addon(config& archive_cfg, const addon_info& info)
{
	const cursor::setter cursor_setter(cursor::WAIT);

	utils::string_map i18n_symbols;
	i18n_symbols["addon_title"] = font::escape_text(info.title);

	auto progress_dlg = gui2::dialogs::file_progress::display(_("Add-ons Manager"), VGETTEXT("Installing add-on <i>$addon_title</i>...", i18n_symbols));
	auto progress_cb = [&progress_dlg](unsigned value) {
		progress_dlg->update_progress(value);
	};

	if(archive_cfg.has_child("removelist") || archive_cfg.has_child("addlist")) {
		LOG_ADDONS << "Received an updatepack for the addon '" << info.id << "'";

		// A consistency check
		for(const auto [key, cfg] : archive_cfg.all_children_view()) {
			if(key == "removelist" || key == "addlist") {
				if(!check_names_legal(cfg)) {
					gui2::show_error_message(VGETTEXT("The add-on <i>$addon_title</i> has an invalid file or directory "
									"name and cannot be installed.", i18n_symbols));
					return false;
				}
				if(!check_case_insensitive_duplicates(cfg)) {
					gui2::show_error_message(VGETTEXT("The add-on <i>$addon_title</i> has file or directory names "
									"with case conflicts. This may cause problems.", i18n_symbols));
				}
			}
		}

		for(const auto [key, cfg] : archive_cfg.all_children_view()) {
			if(key == "removelist") {
				purge_addon(cfg);
			} else if(key == "addlist") {
				unarchive_addon(cfg, progress_cb);
			}
		}

		LOG_ADDONS << "Update completed.";

		//#TODO: hash verification ???
	} else {
		LOG_ADDONS << "Received a full pack for the addon '" << info.id << "'";

		if(!check_names_legal(archive_cfg)) {
			gui2::show_error_message(VGETTEXT("The add-on <i>$addon_title</i> has an invalid file or directory "
							"name and cannot be installed.", i18n_symbols));
			return false;
		}
		if(!check_case_insensitive_duplicates(archive_cfg)) {
			gui2::show_error_message(VGETTEXT("The add-on <i>$addon_title</i> has file or directory names "
							"with case conflicts. This may cause problems.", i18n_symbols));
		}

		LOG_ADDONS << "unpacking " << info.id;

		// Remove any previously installed versions
		if(!remove_local_addon(info.id)) {
			WRN_ADDONS << "failed to uninstall previous version of " << info.id << "; the add-on may not work properly!";
		}

		unarchive_addon(archive_cfg, progress_cb);
		LOG_ADDONS << "unpacking finished";
	}

	config info_cfg;
	info.write_minimal(info_cfg);
	write_addon_install_info(info.id, info_cfg);

	return true;
}

bool addons_client::try_fetch_addon(const addon_info & addon)
{
	config archive;

	if(!(
		download_addon(archive, addon.id, addon.display_title_full(), addon.current_version, !is_addon_installed(addon.id)) &&
		install_addon(archive, addon)
		)) {
		const std::string& server_error = get_last_server_error();
		if(!server_error.empty()) {
			gui2::show_error_message(
				_("The server responded with an error:") + "\n" + server_error);
		}
		return false;
	} else {
		return true;
	}
}

addons_client::install_result addons_client::do_resolve_addon_dependencies(const addons_list& addons, const addon_info& addon)
{
	install_result result;
	result.outcome = install_outcome::success;
	result.wml_changed = false;

	auto cursor_setter = std::make_unique<cursor::setter>(cursor::WAIT);

	// TODO: We don't currently check for the need to upgrade. I'll probably
	// work on that when implementing dependency tiers later.

	const std::set<std::string>& deps = addon.resolve_dependencies(addons);

	std::vector<std::string> missing_deps;
	std::vector<std::string> broken_deps;
	// if two add-ons both have the same dependency and are being downloaded in a batch (such as via the adhoc connection)
	// then the version cache will not be updated after the first is downloaded
	// which will result in it being treated as version 0.0.0, which is then interpreted as being "upgradeable"
	// which then causes the user to be prompted to download the same dependency multiple times
	version_info unknown_version(0, 0, 0);

	for(const std::string& dep : deps) {
		try {
			addon_tracking_info info = get_addon_tracking_info(addons.at(dep));

			// ADDON_NONE means not installed.
			if(info.state == ADDON_NONE) {
				missing_deps.push_back(dep);
			} else if(info.state == ADDON_INSTALLED_UPGRADABLE && info.installed_version != unknown_version) {
				// Tight now, we don't need to distinguish the lists of missing
				// and outdated addons, so just add them to missing.
				missing_deps.push_back(dep);
			}
		} catch(const std::out_of_range&) {
			// Dependency wasn't found on server, check locally directly.
			if(!is_addon_installed(dep)) {
				broken_deps.push_back(dep);
			}
		}
	}

	cursor_setter.reset();

	if(!broken_deps.empty()) {
		std::string broken_deps_report;

		broken_deps_report = _n(
			"The selected add-on has the following dependency, which is not currently installed or available from the server. Do you wish to continue?",
			"The selected add-on has the following dependencies, which are not currently installed or available from the server. Do you wish to continue?",
			broken_deps.size());
		broken_deps_report += "\n";

		for(const std::string& broken_dep_id : broken_deps) {
			broken_deps_report += "\n    " + font::unicode_bullet + " " + make_addon_title(broken_dep_id);
		}

		if(gui2::show_message(_("Broken Dependencies"), broken_deps_report, gui2::dialogs::message::yes_no_buttons) != gui2::retval::OK) {
			result.outcome = install_outcome::abort;
			return result; // canceled by user
		}
	}

	if(missing_deps.empty()) {
		// No dependencies to install, carry on.
		return result;
	}

	{
		addons_list options;
		for(const std::string& dep : missing_deps) {
			options[dep] = addons.at(dep);
		}

		if(!gui2::dialogs::install_dependencies::execute(options)) {
			return result; // the user has chosen to continue without installing anything.
		}
	}

	//
	// Install dependencies now.
	//

	std::vector<std::string> failed_titles;

	for(const std::string& dep : missing_deps) {
		const addon_info& missing_addon = addons.at(dep);

		if(!try_fetch_addon(missing_addon)) {
			failed_titles.push_back(missing_addon.title);
		} else {
			result.wml_changed = true;
		}
	}

	if(!failed_titles.empty()) {
		const std::string& failed_deps_report = _n(
			"The following dependency could not be installed. Do you still wish to continue?",
			"The following dependencies could not be installed. Do you still wish to continue?",
			failed_titles.size()) + std::string("\n\n") + utils::bullet_list(failed_titles);

		result.outcome = gui2::show_message(_("Dependencies Installation Failed"), failed_deps_report, gui2::dialogs::message::yes_no_buttons) == gui2::retval::OK ? install_outcome::success : install_outcome::abort; // If the user cancels, return abort. Otherwise, return success, since the user chose to ignore the failure.
		return result;
	}

	return result;
}

bool addons_client::do_check_before_overwriting_addon(const addon_info& addon)
{
	const std::string& addon_id = addon.id;

	const bool pbl = have_addon_pbl_info(addon_id);
	const bool vcs = have_addon_in_vcs_tree(addon_id);

	if(!pbl && !vcs) {
		return true;
	}

	utils::string_map symbols;
	symbols["addon"] = font::escape_text(addon.title);
	std::string text;
	std::vector<std::string> extra_items;

	text = VGETTEXT("The add-on '$addon|' is already installed and contains additional information that will be permanently lost if you continue:", symbols);
	text += "\n\n";

	if(pbl) {
		extra_items.push_back(_("Publishing information file (.pbl)"));
	}

	if(vcs) {
		extra_items.push_back(_("Version control system (VCS) information"));
	}

	text += utils::bullet_list(extra_items) + "\n\n";
	text += _("Do you really wish to continue?");

	return gui2::show_message(_("Confirm"), text, gui2::dialogs::message::yes_no_buttons) == gui2::retval::OK;
}

addons_client::install_result addons_client::install_addon_with_checks(const addons_list& addons, const addon_info& addon)
{
	if(!(do_check_before_overwriting_addon(addon))) {
		// Just do nothing and leave.
		install_result result;
		result.outcome = install_outcome::abort;
		result.wml_changed = false;

		return result;
	}

	// Resolve any dependencies
	install_result res = do_resolve_addon_dependencies(addons, addon);
	if(res.outcome != install_outcome::success) { // this function only returns SUCCESS and ABORT as outcomes
		return res; // user aborted
	}

	if(!try_fetch_addon(addon)) {
		res.outcome = install_outcome::failure;
		return res; //wml_changed should have whatever value was obtained in resolving dependencies
	} else {
		res.wml_changed = true;
		return res; //we successfully installed something, so now the wml was definitely changed
	}
}

bool addons_client::is_error_response(const config& response_cfg)
{
	if(auto error = response_cfg.optional_child("error")) {
		if(error->has_attribute("status_code")) {
			const auto& status_msg = translated_addon_check_status(error["status_code"].to_unsigned());
			last_error_ = font::escape_text(status_msg);
		} else {
			last_error_ = font::escape_text(error["message"].str());
		}
		last_error_data_ = font::escape_text(error["extra_data"].str());
		ERR_ADDONS << "server error: " << *error;
		return true;
	} else {
		last_error_.clear();
		last_error_data_.clear();
		return false;
	}
}

void addons_client::clear_last_error()
{
	last_error_.clear();
	last_error_data_.clear();
}

void addons_client::clear_server_info()
{
	server_id_.clear();
	server_version_.clear();
	server_capabilities_.clear();
	server_url_.clear();
	license_notice_.clear();
}

void addons_client::check_connected() const
{
	assert(conn_ != nullptr);
	if(conn_ == nullptr) {
		ERR_ADDONS << "not connected to server";
		throw not_connected_to_server();
	}
}

void addons_client::send_request(const config& request, config& response)
{
	check_connected();

	response.clear();
	conn_->transfer(request, response);
}

void addons_client::send_simple_request(const std::string& request_string, config& response)
{
	config request;
	request.add_child(request_string);
	send_request(request, response);
}
struct read_addon_connection_data : public network_transmission::connection_data
{
	read_addon_connection_data(network_asio::connection& conn, addons_client& client)
		: conn_(conn), client_(client) {}
	std::size_t total() override { return conn_.bytes_to_read(); }
	virtual std::size_t current()  override { return conn_.bytes_read(); }
	virtual bool finished() override { return conn_.done(); }
	virtual void cancel() override { client_.connect(); }
	virtual void poll() override { conn_.poll(); }
	network_asio::connection& conn_;
	addons_client& client_;
};
struct connect_connection_data : public network_transmission::connection_data
{
	connect_connection_data(network_asio::connection& conn, addons_client& client)
		: conn_(conn), client_(client) {}
	std::size_t total() override { return conn_.bytes_to_read(); }
	std::size_t current() override { return conn_.bytes_read(); }
	bool finished() override { return conn_.done(); }
	void cancel() override { client_.disconnect(); }
	void poll() override { conn_.poll(); }
	network_asio::connection& conn_;
	addons_client& client_;
};
struct write_addon_connection_data : public network_transmission::connection_data
{
	write_addon_connection_data(network_asio::connection& conn, addons_client& client)
		: conn_(conn), client_(client) {}
	std::size_t total() override { return conn_.bytes_to_write(); }
	virtual std::size_t current()  override { return conn_.bytes_written(); }
	virtual bool finished() override { return conn_.done(); }
	virtual void cancel() override { client_.connect(); }
	virtual void poll() override { conn_.poll(); }
	network_asio::connection& conn_;
	addons_client& client_;
};
void addons_client::wait_for_transfer_done(const std::string& status_message, transfer_mode mode)
{
	check_connected();
	std::unique_ptr<network_transmission::connection_data> cd;
	switch(mode) {
	case transfer_mode::download:
		cd.reset(new read_addon_connection_data{*conn_, *this});
		break;
	case transfer_mode::connect:
		cd.reset(new connect_connection_data{*conn_, *this});
		break;
	case transfer_mode::upload:
		cd.reset(new write_addon_connection_data{*conn_, *this});
		break;
	default:
		throw std::invalid_argument("Addon client: invalid transfer mode");
	}

	gui2::dialogs::network_transmission stat(*cd, _("Add-ons Manager"), status_message);

	if(!stat.show()) {
		// Notify the caller chain that the user aborted the operation.
		if(mode == transfer_mode::connect) {
			throw user_disconnect();
		} else {
			throw user_exit();
		}
	}
}
