/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
                 2008 - 2015 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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
#include "addon/validation.hpp"
#include "cursor.hpp"
#include "formula/string_utils.hpp"
#include "gettext.hpp"
#include "gui/dialogs/addon/install_dependencies.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/widgets/window.hpp"
#include "log.hpp"
#include "serialization/parser.hpp"
#include "serialization/string_utils.hpp"

#include "addon/client.hpp"

static lg::log_domain log_addons_client("addons-client");
#define ERR_ADDONS LOG_STREAM(err ,  log_addons_client)
#define WRN_ADDONS LOG_STREAM(warn,  log_addons_client)
#define LOG_ADDONS LOG_STREAM(info,  log_addons_client)
#define DBG_ADDONS LOG_STREAM(debug, log_addons_client)

using gui2::dialogs::network_transmission;

addons_client::addons_client(CVideo& v, const std::string& address)
	: v_(v)
	, addr_(address)
	, host_()
	, port_()
	, conn_(nullptr)
	, stat_(nullptr)
	, last_error_()
	, last_error_data_()
{
	const std::vector<std::string>& address_components =
		utils::split(addr_, ':');

	if(address_components.empty()) {
		throw invalid_server_address();
	}

	// FIXME: this parsing will break IPv6 numeric addresses! */
	host_ = address_components[0];
	port_ = address_components.size() == 2 ?
		address_components[1] : std::to_string(default_campaignd_port);
}

void addons_client::connect()
{
	LOG_ADDONS << "connecting to server " << host_ << " on port " << port_ << '\n';

	utils::string_map i18n_symbols;
	i18n_symbols["server_address"] = addr_;

	conn_.reset(new network_asio::connection(host_, port_));

	this->wait_for_transfer_done(
		vgettext("Connecting to $server_address|...", i18n_symbols));
}

bool addons_client::request_addons_list(config& cfg)
{
	cfg.clear();

	config response_buf;

	/** @todo FIXME: get rid of this legacy "campaign"/"campaigns" silliness
	 */

	this->send_simple_request("request_campaign_list", response_buf);
	this->wait_for_transfer_done(_("Downloading list of add-ons..."));

	cfg = response_buf.child("campaigns");

	return !this->update_last_error(response_buf);
}

bool addons_client::request_distribution_terms(std::string& terms)
{
	terms.clear();

	config response_buf;

	this->send_simple_request("request_terms", response_buf);
	this->wait_for_transfer_done(_("Requesting distribution terms..."));

	if(const config& msg_cfg = response_buf.child("message")) {
		terms = msg_cfg["message"].str();
	}

	return !this->update_last_error(response_buf);
}

bool addons_client::upload_addon(const std::string& id, std::string& response_message, config& cfg)
{
	LOG_ADDONS << "preparing to upload " << id << '\n';

	response_message.clear();

	utils::string_map i18n_symbols;
	i18n_symbols["addon_title"] = cfg["title"];
	if(i18n_symbols["addon_title"].empty()) {
		i18n_symbols["addon_title"] = make_addon_title(id);
	}

	if(!addon_name_legal(id)){
		i18n_symbols["addon_id"] = id;
		this->last_error_ =
			vgettext("The add-on <i>$addon_title</i> has an invalid id '$addon_id' "
				"and cannot be published.", i18n_symbols);
		return false;
	}

	std::string passphrase = cfg["passphrase"];
	// generate a random passphrase and write it to disk
	// if the .pbl file doesn't provide one already
	if(passphrase.empty()) {
		passphrase.resize(8);
		for(size_t n = 0; n != 8; ++n) {
			passphrase[n] = 'a' + (rand()%26);
		}
		cfg["passphrase"] = passphrase;
		set_addon_pbl_info(id, cfg);

		LOG_ADDONS << "automatically generated an initial passphrase for " << id << '\n';
	}

	cfg["name"] = id;

	config addon_data;
	archive_addon(id, addon_data);

	std::vector<std::string> badnames;
	if(!check_names_legal(addon_data, &badnames)){
		this->last_error_ =
			vgettext("The add-on <i>$addon_title</i> has an invalid file or directory "
				"name and cannot be published.", i18n_symbols);
		this->last_error_data_ = utils::join(badnames, "\n");
		return false;
	}

	config request_buf, response_buf;
	request_buf.add_child("upload", cfg).add_child("data", addon_data);

	LOG_ADDONS << "sending " << id << '\n';

	this->send_request(request_buf, response_buf);
	this->wait_for_transfer_done(vgettext("Sending add-on <i>$addon_title</i>...", i18n_symbols
	), true);

	if(const config& message_cfg = response_buf.child("message")) {
		response_message = message_cfg["message"].str();
		LOG_ADDONS << "server response: " << response_message << '\n';
	}

	return !this->update_last_error(response_buf);

}

bool addons_client::delete_remote_addon(const std::string& id, std::string& response_message)
{
	response_message.clear();

	config cfg = get_addon_pbl_info(id);

	utils::string_map i18n_symbols;
	i18n_symbols["addon_title"] = cfg["title"];
	if(i18n_symbols["addon_title"].empty()) {
		i18n_symbols["addon_title"] = make_addon_title(id);
	}

	config request_buf, response_buf;
	config& request_body = request_buf.add_child("delete");

	request_body["name"] = id;
	request_body["passphrase"] = cfg["passphrase"];

	LOG_ADDONS << "requesting server to delete " << id << '\n';

	this->send_request(request_buf, response_buf);
	this->wait_for_transfer_done(vgettext("Removing add-on <i>$addon_title</i> from the server...", i18n_symbols
	));

	if(const config& message_cfg = response_buf.child("message")) {
		response_message = message_cfg["message"].str();
		LOG_ADDONS << "server response: " << response_message << '\n';
	}

	return !this->update_last_error(response_buf);
}

bool addons_client::download_addon(config& archive_cfg, const std::string& id, const std::string& title, bool increase_downloads)
{
	archive_cfg.clear();

	config request_buf;
	config& request_body = request_buf.add_child("request_campaign");

	request_body["name"] = id;
	request_body["increase_downloads"] = increase_downloads;

	utils::string_map i18n_symbols;
	i18n_symbols["addon_title"] = title;

	LOG_ADDONS << "downloading " << id << '\n';

	this->send_request(request_buf, archive_cfg);
	this->wait_for_transfer_done(vgettext("Downloading add-on <i>$addon_title</i>...", i18n_symbols));

	return !this->update_last_error(archive_cfg);
}

bool addons_client::install_addon(config& archive_cfg, const addon_info& info)
{
	const cursor::setter cursor_setter(cursor::WAIT);

	utils::string_map i18n_symbols;
	i18n_symbols["addon_title"] = info.title;

	if(!check_names_legal(archive_cfg)) {
		gui2::show_error_message(v_,
			vgettext("The add-on <i>$addon_title</i> has an invalid file or directory "
				"name and cannot be installed.", i18n_symbols));
		return false;
	}

	// Add local version information before unpacking

	config* maindir = &archive_cfg.find_child("dir", "name", info.id);
	if(!*maindir) {
		LOG_ADDONS << "downloaded add-on '" << info.id << "' is missing its directory in the archive; creating it\n";
		maindir = &archive_cfg.add_child("dir");
		(*maindir)["name"] = info.id;
	}

	LOG_ADDONS << "generating version info for add-on '" << info.id << "'\n";

	std::ostringstream info_contents;
	config wml;

	info_contents <<
		"#\n"
		"# File automatically generated by Wesnoth to keep track\n"
		"# of version information on installed add-ons. DO NOT EDIT!\n"
		"#\n";

	info.write_minimal(wml.add_child("info"));
	write(info_contents, wml);

	config file;
	file["name"] = "_info.cfg";
	file["contents"] = info_contents.str();

	maindir->add_child("file", file);

	LOG_ADDONS << "unpacking " << info.id << '\n';

	// Remove any previously installed versions
	if(!remove_local_addon(info.id)) {
		WRN_ADDONS << "failed to uninstall previous version of " << info.id << "; the add-on may not work properly!" << std::endl;
	}

	unarchive_addon(archive_cfg);
	LOG_ADDONS << "unpacking finished\n";

	return true;
}

bool addons_client::try_fetch_addon(const addon_info & addon)
{
	config archive;

	if(!(
		download_addon(archive, addon.id, addon.title, !is_addon_installed(addon.id)) &&
		install_addon(archive, addon)
		)) {
		const std::string& server_error = get_last_server_error();
		if(!server_error.empty()) {
			gui2::show_error_message(v_,
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

	std::unique_ptr<cursor::setter> cursor_setter(new cursor::setter(cursor::WAIT));

	// TODO: We don't currently check for the need to upgrade. I'll probably
	// work on that when implementing dependency tiers later.

	const std::set<std::string>& deps = addon.resolve_dependencies(addons);

	std::vector<std::string> missing_deps;
	std::vector<std::string> broken_deps;

	for(const std::string& dep : deps) {
		if(!is_addon_installed(dep)) {
			if(addons.find(dep) != addons.end()) {
				missing_deps.push_back(dep);
			} else {
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

		if(gui2::show_message(v_, _("Broken Dependencies"), broken_deps_report, gui2::dialogs::message::yes_no_buttons) != gui2::window::OK) {
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
			const addon_info& missing_addon = addons.at(dep);
			options[dep] = missing_addon;
		}

		gui2::dialogs::install_dependencies dlg(options);
		bool cont = dlg.show(v_);
		if(!cont) {
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

		result.outcome = gui2::show_message(v_, _("Dependencies Installation Failed"), failed_deps_report, gui2::dialogs::message::yes_no_buttons) == gui2::window::OK ? install_outcome::success : install_outcome::abort; // If the user cancels, return abort. Otherwise, return success, since the user chose to ignore the failure.
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
	symbols["addon"] = addon.title;
	std::string text;
	std::vector<std::string> extra_items;

	text = vgettext("The add-on '$addon|' is already installed and contains additional information that will be permanently lost if you continue:", symbols);
	text += "\n\n";

	if(pbl) {
		extra_items.push_back(_("Publishing information file (.pbl)"));
	}

	if(vcs) {
		extra_items.push_back(_("Version control system (VCS) information"));
	}

	text += utils::bullet_list(extra_items) + "\n\n";
	text += _("Do you really wish to continue?");

	return gui2::show_message(v_, _("Confirm"), text, gui2::dialogs::message::yes_no_buttons) == gui2::window::OK;
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

bool addons_client::update_last_error(config& response_cfg)
{
	if(config const &error = response_cfg.child("error")) {
		this->last_error_ = error["message"].str();
		this->last_error_data_ = error["extra_data"].str();
		ERR_ADDONS << "server error: " << error << '\n';
		return true;
	} else {
		this->last_error_.clear();
		this->last_error_data_.clear();
		return false;
	}
}

void addons_client::check_connected() const
{
	assert(conn_ != nullptr);
	if(conn_ == nullptr) {
		ERR_ADDONS << "not connected to server" << std::endl;
		throw not_connected_to_server();
	}
}

void addons_client::send_request(const config& request, config& response)
{
	check_connected();

	response.clear();
	this->conn_->transfer(request, response);
}

void addons_client::send_simple_request(const std::string& request_string, config& response)
{
	config request;
	request.add_child(request_string);
	this->send_request(request, response);
}
struct read_addon_connection_data : public network_transmission::connection_data
{
	read_addon_connection_data(network_asio::connection& conn) : conn_(conn) {}
	size_t total() override { return conn_.bytes_to_read(); }
	virtual size_t current()  override { return conn_.bytes_read(); }
	virtual bool finished() override { return conn_.done(); }
	virtual void cancel() override { return conn_.cancel(); }
	virtual void poll() override { conn_.poll(); }
	network_asio::connection& conn_;
};
struct write_addon_connection_data : public network_transmission::connection_data
{
	write_addon_connection_data(network_asio::connection& conn) : conn_(conn) {}
	size_t total() override { return conn_.bytes_to_write(); }
	virtual size_t current()  override { return conn_.bytes_written(); }
	virtual bool finished() override { return conn_.done(); }
	virtual void cancel() override { return conn_.cancel(); }
	virtual void poll() override { conn_.poll(); }
	network_asio::connection& conn_;
};
void addons_client::wait_for_transfer_done(const std::string& status_message, bool track_upload)
{
	check_connected();
	std::unique_ptr<network_transmission::connection_data> cd;
	if(track_upload)
		cd.reset(new write_addon_connection_data{ *conn_ });
	else
		cd.reset(new read_addon_connection_data{ *conn_ });
	if(!stat_) {
		stat_.reset(new network_transmission(*cd, _("Add-ons Manager"), status_message));
	} else {
		stat_->set_subtitle(status_message);
		stat_->set_connection_data(*cd);
	}

	if(!stat_->show(v_)) {
		// Notify the caller chain that the user aborted the operation.
		throw user_exit();
	}
}
