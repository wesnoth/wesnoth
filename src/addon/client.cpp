/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
                 2008 - 2014 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
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
#include "display.hpp"
#include "formula_string_utils.hpp"
#include "gettext.hpp"
#include "gui/dialogs/message.hpp"
#include "log.hpp"
#include "serialization/parser.hpp"
#include "serialization/string_utils.hpp"

#include "addon/client.hpp"

static lg::log_domain log_addons_client("addons-client");
#define ERR_ADDONS LOG_STREAM(err ,  log_addons_client)
#define WRN_ADDONS LOG_STREAM(warn,  log_addons_client)
#define LOG_ADDONS LOG_STREAM(info,  log_addons_client)
#define DBG_ADDONS LOG_STREAM(debug, log_addons_client)

addons_client::addons_client(display& disp, const std::string& address)
	: disp_(disp)
	, addr_(address)
	, host_()
	, port_()
	, conn_(NULL)
	, stat_(NULL)
	, last_error_()
{
	const std::vector<std::string>& address_components =
		utils::split(addr_, ':');

	if(address_components.empty()) {
		throw invalid_server_address();
	}

	// FIXME: this parsing will break IPv6 numeric addresses! */
	host_ = address_components[0];
	port_ = address_components.size() == 2 ?
		address_components[1] : str_cast(default_campaignd_port);
}

void addons_client::connect()
{
	LOG_ADDONS << "connecting to server " << host_ << " on port " << port_ << '\n';

	utils::string_map i18n_symbols;
	i18n_symbols["server_address"] = addr_;

	conn_ = new network_asio::connection(host_, port_);

	this->wait_for_transfer_done(
		vgettext("Connecting to $server_address|...", i18n_symbols));
}

bool addons_client::request_addons_list(config& cfg)
{
	cfg.clear();

	config response_buf;

	/** @todo FIXME: get rid of this legacy "campaign"/"campaigns" silliness */

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

bool addons_client::upload_addon(const std::string& id, std::string& response_message)
{
	LOG_ADDONS << "preparing to upload " << id << '\n';

	response_message.clear();

	config cfg;
	get_addon_pbl_info(id, cfg);

	utils::string_map i18n_symbols;
	i18n_symbols["addon_title"] = cfg["title"];
	if(i18n_symbols["addon_title"].empty()) {
		i18n_symbols["addon_title"] = make_addon_title(id);
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

	config cfg;
	get_addon_pbl_info(id, cfg);

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
		gui2::show_error_message(disp_.video(),
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

bool addons_client::update_last_error(config& response_cfg)
{
	if(config const &error = response_cfg.child("error")) {
		this->last_error_ = error["message"].str();
		ERR_ADDONS << "server error: " << error << '\n';
		return true;
	} else {
		this->last_error_.clear();
		return false;
	}
}

void addons_client::check_connected() const
{
	assert(conn_ != NULL);
	if(conn_ == NULL) {
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

void addons_client::wait_for_transfer_done(const std::string& status_message, bool track_upload)
{
	check_connected();

	if(!stat_) {
		stat_ = new gui2::tnetwork_transmission(*conn_, _("Add-ons Manager"), status_message);
	} else {
		stat_->set_subtitle(status_message);
		stat_->set_track_upload(track_upload);
	}

	if(!stat_->show(disp_.video())) {
		// Notify the caller chain that the user aborted the operation.
		throw user_exit();
	}
}

addons_client::~addons_client()
{
	delete stat_; // stat_ depends on conn_, so it must be destroyed first!
	delete conn_;
}
