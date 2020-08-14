/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
   */

/**
 * @file
 * Wesnoth addon server.
 * Expects a "server.cfg" config file in the current directory
 * and saves addons under data/.
 */

#include "server/campaignd/server.hpp"

#include "filesystem.hpp"
#include "lexical_cast.hpp"
#include "log.hpp"
#include "serialization/base64.hpp"
#include "serialization/binary_or_text.hpp"
#include "serialization/parser.hpp"
#include "serialization/string_utils.hpp"
#include "serialization/unicode.hpp"
#include "game_config.hpp"
#include "addon/validation.hpp"
#include "server/campaignd/addon_utils.hpp"
#include "server/campaignd/blacklist.hpp"
#include "server/campaignd/control.hpp"
#include "server/campaignd/fs_commit.hpp"
#include "game_version.hpp"
#include "hash.hpp"

#include <csignal>
#include <ctime>

#include <boost/random.hpp>
#include <boost/generator_iterator.hpp>

// the fork execute is unix specific only tested on Linux quite sure it won't
// work on Windows not sure which other platforms have a problem with it.
#if !(defined(_WIN32))
#include <errno.h>
#endif

static lg::log_domain log_campaignd("campaignd");
#define DBG_CS LOG_STREAM(debug, log_campaignd)
#define LOG_CS LOG_STREAM(info,  log_campaignd)
#define WRN_CS LOG_STREAM(warn,  log_campaignd)
#define ERR_CS LOG_STREAM(err,   log_campaignd)

static lg::log_domain log_config("config");
#define ERR_CONFIG LOG_STREAM(err, log_config)
#define WRN_CONFIG LOG_STREAM(warn, log_config)

static lg::log_domain log_server("server");
#define ERR_SERVER LOG_STREAM(err, log_server)

#include "server/common/send_receive_wml_helpers.ipp"

namespace {

/* Secure password storage functions */
bool authenticate(config& campaign, const config::attribute_value& passphrase)
{
	return utils::md5(passphrase, campaign["passsalt"]).base64_digest() == campaign["passhash"];
}

std::string generate_salt(std::size_t len)
{
	boost::mt19937 mt(std::time(0));
	std::string salt = std::string(len, '0');
	boost::uniform_int<> from_str(0, 63); // 64 possible values for base64
	boost::variate_generator< boost::mt19937, boost::uniform_int<>> get_char(mt, from_str);

	for(std::size_t i = 0; i < len; i++) {
		salt[i] = crypt64::encode(get_char());
	}

	return salt;
}

void set_passphrase(config& campaign, std::string passphrase)
{
	std::string salt = generate_salt(16);
	campaign["passsalt"] = salt;
	campaign["passhash"] = utils::md5(passphrase, salt).base64_digest();
}

} // end anonymous namespace

namespace campaignd {

server::server(const std::string& cfg_file)
	: server_base(default_campaignd_port, true)
	, addons_()
	, dirty_addons_()
	, cfg_()
	, cfg_file_(cfg_file)
	, read_only_(false)
	, compress_level_(0)
	, update_pack_lifespan_(0)
	, hooks_()
	, handlers_()
	, feedback_url_format_()
	, blacklist_()
	, blacklist_file_()
	, stats_exempt_ips_()
	, flush_timer_(io_service_)
{

#ifndef _WIN32
	struct sigaction sa;
	std::memset( &sa, 0, sizeof(sa) );
	#pragma GCC diagnostic ignored "-Wold-style-cast"
	sa.sa_handler = SIG_IGN;
	int res = sigaction( SIGPIPE, &sa, nullptr);
	assert( res == 0 );
#endif
	load_config();

	LOG_CS << "Port: " << port_ << "\n";

	// Ensure all campaigns to use secure hash passphrase storage
	if(!read_only_) {
		for(auto& addon : addons_) {
			config& campaign = addon.second;
			// Campaign already has a hashed password
			if(campaign["passphrase"].empty()) {
				continue;
			}

			LOG_CS << "Addon '" << campaign["title"] << "' uses unhashed passphrase. Fixing.\n";
			set_passphrase(campaign, campaign["passphrase"]);
			campaign["passphrase"] = "";
			dirty_addons_.emplace(addon.first);
		}
		write_config();
	}

	register_handlers();

	start_server();
	flush_cfg();
}

server::~server()
{
	write_config();
}

void server::load_config()
{
	LOG_CS << "Reading configuration from " << cfg_file_ << "...\n";

	filesystem::scoped_istream in = filesystem::istream_file(cfg_file_);
	read(cfg_, *in);

	read_only_ = cfg_["read_only"].to_bool(false);

	if(read_only_) {
		LOG_CS << "READ-ONLY MODE ACTIVE\n";
	}

	// Seems like compression level above 6 is a waste of CPU cycles.
	compress_level_ = cfg_["compress_level"].to_int(6);
	// One month probably will be fine (#TODO: testing needed)
	update_pack_lifespan_ = cfg_["update_pack_lifespan"].to_time_t(30 * 24 * 60 * 60);

	const config& svinfo_cfg = server_info();
	if(svinfo_cfg) {
		feedback_url_format_ = svinfo_cfg["feedback_url_format"].str();
	}

	blacklist_file_ = cfg_["blacklist_file"].str();
	load_blacklist();

	stats_exempt_ips_ = utils::split(cfg_["stats_exempt_ips"].str());

	// Load any configured hooks.
	hooks_.emplace(std::string("hook_post_upload"), cfg_["hook_post_upload"]);
	hooks_.emplace(std::string("hook_post_erase"), cfg_["hook_post_erase"]);

#ifndef _WIN32
	// Open the control socket if enabled.
	if(!cfg_["control_socket"].empty()) {
		const std::string& path = cfg_["control_socket"].str();

		if(path != fifo_path_) {
			const int res = mkfifo(path.c_str(),0660);
			if(res != 0 && errno != EEXIST) {
				ERR_CS << "could not make fifo at '" << path << "' (" << strerror(errno) << ")\n";
			} else {
				input_.close();
				int fifo = open(path.c_str(), O_RDWR|O_NONBLOCK);
				input_.assign(fifo);
				LOG_CS << "opened fifo at '" << path << "'. Server commands may be written to this file.\n";
				read_from_fifo();
				fifo_path_ = path;
			}
		}
	}
#endif

	// Certain config values are saved to WML again so that a given server
	// instance's parameters remain constant even if the code defaults change
	// at some later point.
	cfg_["compress_level"] = compress_level_;

	// But not the listening port number.
	port_ = cfg_["port"].to_int(default_campaignd_port);

	// Limit the max size of WML documents received from the net to prevent the
	// possible excessive use of resources due to malformed packets received.
	// Since an addon is sent in a single WML document this essentially limits
	// the maximum size of an addon that can be uploaded.
	simple_wml::document::document_size_limit = cfg_["document_size_limit"].to_int(default_document_size_limit);

	//Loading addons
	addons_.clear();
	std::vector<std::string> legacy_addons, dirs;
	filesystem::get_files_in_dir("data", &legacy_addons, &dirs);
	config meta;
	for(const std::string& addon_dir : dirs) {
		in = filesystem::istream_file(filesystem::normalize_path("data/" + addon_dir + "/addon.cfg"));
		read(meta, *in);
		if(meta) {
			addons_.emplace(meta["name"].str(), meta);
		} else {
			WRN_CS << "Failed to load addon from dir '" << addon_dir << "'\n";
		}
	}

	// Convert all legacy addons to the new format on load
	if(cfg_.has_child("campaigns")) {
		WRN_CS << "Old format addons have been detected in the config! They will be converted to the new file format! ";
		config& campaigns = cfg_.child("campaigns");
		WRN_CS << campaigns.child_count("campaign") << " entries to be processed.\n";
		for(config& campaign : campaigns.child_range("campaign")) {
			const std::string& addon_id = campaign["name"].str();
			const std::string& addon_file = campaign["filename"].str();
			if(get_addon(addon_id)) {
				ERR_CS << "The addon '" << addon_id
					   << "' already exists in the new form! The old version will be ignored and its file deleted!\n";
				filesystem::delete_file(filesystem::normalize_path(addon_file));
				continue;
			}
			if(std::find(legacy_addons.begin(), legacy_addons.end(), addon_id) == legacy_addons.end()) {
				ERR_CS << "No file has been found for the legacy addon '" << addon_id
					   << "'. Please, check the file structure!\n";
				continue;
			}

			config data;
			in = filesystem::istream_file(filesystem::normalize_path(addon_file));
			read_gz(data, *in);
			if(!data) {
				ERR_CS << "Couldn't read the content file for the legacy addon '" << addon_id << "'!\n";
				continue;
			}

			const std::string file_hash = utils::md5(campaign["version"].str()).hex_digest();
			config version_cfg = config("version", campaign["version"].str());
			version_cfg["filename"] = "/full_pack_" + file_hash + ".gz";
			campaign.add_child("version", version_cfg);

			data.remove_attributes("title", "campaign_name", "author", "description", "version", "timestamp", "original_timestamp", "icon", "type", "tags");
			filesystem::delete_file(filesystem::normalize_path(addon_file));
			{
				filesystem::atomic_commit campaign_file(addon_file + version_cfg["filename"].str());
				config_writer writer(*campaign_file.ostream(), true, compress_level_);
				writer.write(data);
				campaign_file.commit();
			}

			addons_.emplace(addon_id, campaign);
			dirty_addons_.emplace(addon_id);
		}
		cfg_.clear_children("campaigns");
		LOG_CS << "Legacy addons processing finished.\n";
		write_config();
	}

	LOG_CS << "Loaded addons metadata. " << addons_.size() << " addons found.\n";
}

void server::handle_new_client(socket_ptr socket)
{
	async_receive_doc(socket,
					  std::bind(&server::handle_request, this, _1, _2)
					  );
}

void server::handle_request(socket_ptr socket, std::shared_ptr<simple_wml::document> doc)
{
	config data;
	read(data, doc->output());

	config::all_children_iterator i = data.ordered_begin();

	if(i != data.ordered_end()) {
		// We only handle the first child.
		const config::any_child& c = *i;

		request_handlers_table::const_iterator j
				= handlers_.find(c.key);

		if(j != handlers_.end()) {
			// Call the handler.
			j->second(this, request(c.key, c.cfg, socket));
		} else {
			send_error("Unrecognized [" + c.key + "] request.",socket);
		}
	}
}

#ifndef _WIN32

void server::handle_read_from_fifo(const boost::system::error_code& error, std::size_t)
{
	if(error) {
		if(error == boost::asio::error::operation_aborted)
			// This means fifo was closed by load_config() to open another fifo
			return;
		ERR_CS << "Error reading from fifo: " << error.message() << '\n';
		return;
	}

	std::istream is(&admin_cmd_);
	std::string cmd;
	std::getline(is, cmd);

	const control_line ctl = cmd;

	if(ctl == "shut_down") {
		LOG_CS << "Shut down requested by admin, shutting down...\n";
		throw server_shutdown("Shut down via fifo command");
	} else if(ctl == "readonly") {
		if(ctl.args_count()) {
			cfg_["read_only"] = read_only_ = utils::string_bool(ctl[1], true);
		}

		LOG_CS << "Read only mode: " << (read_only_ ? "enabled" : "disabled") << '\n';
	} else if(ctl == "flush") {
		LOG_CS << "Flushing config to disk...\n";
		write_config();
	} else if(ctl == "reload") {
		if(ctl.args_count()) {
			if(ctl[1] == "blacklist") {
				LOG_CS << "Reloading blacklist...\n";
				load_blacklist();
			} else {
				ERR_CS << "Unrecognized admin reload argument: " << ctl[1] << '\n';
			}
		} else {
			LOG_CS << "Reloading all configuration...\n";
			load_config();
			LOG_CS << "Reloaded configuration\n";
		}
	} else if(ctl == "delete") {
		if(ctl.args_count() != 1) {
			ERR_CS << "Incorrect number of arguments for 'delete'\n";
		} else {
			const std::string& addon_id = ctl[1];

			LOG_CS << "deleting add-on '" << addon_id << "' requested from control FIFO\n";
			delete_addon(addon_id);
		}
	} else if(ctl == "hide" || ctl == "unhide") {
		if(ctl.args_count() != 1) {
			ERR_CS << "Incorrect number of arguments for '" << ctl.cmd() << "'\n";
		} else {
			const std::string& addon_id = ctl[1];
			config& campaign = get_addon(addon_id);

			if(!campaign) {
				ERR_CS << "Add-on '" << addon_id << "' not found, cannot " << ctl.cmd() << "\n";
			} else {
				campaign["hidden"] = ctl.cmd() == "hide";
				dirty_addons_.emplace(addon_id);
				write_config();
				LOG_CS << "Add-on '" << addon_id << "' is now " << (ctl.cmd() == "hide" ? "hidden" : "unhidden") << '\n';
			}
		}
	} else if(ctl == "setpass") {
		if(ctl.args_count() != 2) {
			ERR_CS << "Incorrect number of arguments for 'setpass'\n";
		} else {
			const std::string& addon_id = ctl[1];
			const std::string& newpass = ctl[2];
			config& campaign = get_addon(addon_id);

			if(!campaign) {
				ERR_CS << "Add-on '" << addon_id << "' not found, cannot set passphrase\n";
			} else if(newpass.empty()) {
				// Shouldn't happen!
				ERR_CS << "Add-on passphrases may not be empty!\n";
			} else {
				set_passphrase(campaign, newpass);
				dirty_addons_.emplace(addon_id);
				write_config();
				LOG_CS << "New passphrase set for '" << addon_id << "'\n";
			}
		}
	} else if(ctl == "setattr") {
		if(ctl.args_count() != 3) {
			ERR_CS << "Incorrect number of arguments for 'setattr'\n";
		} else {
			const std::string& addon_id = ctl[1];
			const std::string& key = ctl[2];
			const std::string& value = ctl[3];

			config& campaign = get_addon(addon_id);

			if(!campaign) {
				ERR_CS << "Add-on '" << addon_id << "' not found, cannot set attribute\n";
			} else if(key == "name") {
				ERR_CS << "setattr cannot be used to rename add-ons\n";
			} else if(key == "passphrase" || key == "passhash"|| key == "passsalt") {
				ERR_CS << "setattr cannot be used to set auth data -- use setpass instead\n";
			} else if(!campaign.has_attribute(key)) {
				// NOTE: This is a very naive approach for validating setattr's
				//       input, but it should generally work since add-on
				//       uploads explicitly set all recognized attributes to
				//       the values provided by the .pbl data or the empty
				//       string if absent, and this is normally preserved by
				//       the config serialization.
				ERR_CS << "Attribute '" << value << "' is not a recognized add-on attribute\n";
			} else {
				campaign[key] = value;
				dirty_addons_.emplace(addon_id);
				write_config();
				LOG_CS << "Set attribute on add-on '" << addon_id << "':\n"
				       << key << "=\"" << value << "\"\n";
			}
		}
	} else {
		ERR_CS << "Unrecognized admin command: " << ctl.full() << '\n';
	}

	read_from_fifo();
}

void server::handle_sighup(const boost::system::error_code&, int)
{
	LOG_CS << "SIGHUP caught, reloading config.\n";

	load_config(); // TODO: handle port number config changes

	LOG_CS << "Reloaded configuration\n";

	sighup_.async_wait(std::bind(&server::handle_sighup, this, _1, _2));
}

#endif

void server::flush_cfg()
{
	flush_timer_.expires_from_now(std::chrono::minutes(10));
	flush_timer_.async_wait(std::bind(&server::handle_flush, this, _1));
}

void server::handle_flush(const boost::system::error_code& error)
{
	if(error) {
		ERR_CS << "Error from reload timer: " << error.message() << "\n";
		throw boost::system::system_error(error);
	}
	write_config();
	flush_cfg();
}

void server::load_blacklist()
{
	// We *always* want to clear the blacklist first, especially if we are
	// reloading the configuration and the blacklist is no longer enabled.
	blacklist_.clear();

	if(blacklist_file_.empty()) {
		return;
	}

	try {
		filesystem::scoped_istream in = filesystem::istream_file(blacklist_file_);
		config blcfg;

		read(blcfg, *in);

		blacklist_.read(blcfg);
		LOG_CS << "using blacklist from " << blacklist_file_ << '\n';
	} catch(const config::error&) {
		ERR_CS << "failed to read blacklist from " << blacklist_file_ << ", blacklist disabled\n";
	}
}

void server::write_config()
{
	DBG_CS << "writing configuration and add-ons list to disk...\n";
	filesystem::atomic_commit out(cfg_file_);
	write(*out.ostream(), cfg_);
	out.commit();

	for(const std::string& name : dirty_addons_) {
		const config& addon = get_addon(name);
		if(addon && !addon["filename"].empty()) {
			filesystem::atomic_commit addon_out(filesystem::normalize_path(addon["filename"].str() + "/addon.cfg"));
			write(*addon_out.ostream(), addon);
			addon_out.commit();
		}
	}

	dirty_addons_.clear();
	DBG_CS << "... done\n";
}

void server::fire(const std::string& hook, const std::string& addon)
{
	const std::map<std::string, std::string>::const_iterator itor = hooks_.find(hook);
	if(itor == hooks_.end()) {
		return;
	}

	const std::string& script = itor->second;
	if(script.empty()) {
		return;
	}

#if defined(_WIN32)
	UNUSED(addon);
	ERR_CS << "Tried to execute a script on an unsupported platform\n";
	return;
#else
	pid_t childpid;

	if((childpid = fork()) == -1) {
		ERR_CS << "fork failed while updating campaign " << addon << '\n';
		return;
	}

	if(childpid == 0) {
		// We are the child process. Execute the script. We run as a
		// separate thread sharing stdout/stderr, which will make the
		// log look ugly.
		execlp(script.c_str(), script.c_str(), addon.c_str(), static_cast<char *>(nullptr));

		// exec() and family never return; if they do, we have a problem
		std::cerr << "ERROR: exec failed with errno " << errno << " for addon " << addon
				  << '\n';
		exit(errno);

	} else {
		return;
	}
#endif
}

bool server::ignore_address_stats(const std::string& addr) const
{
	for(const auto& mask : stats_exempt_ips_) {
		// TODO: we want CIDR subnet mask matching here, not glob matching!
		if(utils::wildcard_string_match(addr, mask)) {
			return true;
		}
	}

	return false;
}

void server::send_message(const std::string& msg, socket_ptr sock)
{
	simple_wml::document doc;
	doc.root().add_child("message").set_attr_dup("message", msg.c_str());
	async_send_doc(sock, doc, std::bind(&server::handle_new_client, this, _1), null_handler);
}

void server::send_error(const std::string& msg, socket_ptr sock)
{
	ERR_CS << "[" << client_address(sock) << "]: " << msg << '\n';
	simple_wml::document doc;
	doc.root().add_child("error").set_attr_dup("message", msg.c_str());
	async_send_doc(sock, doc, std::bind(&server::handle_new_client, this, _1), null_handler);
}

void server::send_error(const std::string& msg, const std::string& extra_data, socket_ptr sock)
{
	ERR_CS << "[" << client_address(sock) << "]: " << msg << '\n';
	simple_wml::document doc;
	simple_wml::node& err_cfg = doc.root().add_child("error");
	err_cfg.set_attr_dup("message", msg.c_str());
	err_cfg.set_attr_dup("extra_data", extra_data.c_str());
	async_send_doc(sock, doc, std::bind(&server::handle_new_client, this, _1), null_handler);
}

config& server::get_addon(const std::string& id)
{
	auto addon = addons_.find(id);
	if(addon != addons_.end()) {
		return addons_.at(id);
	} else {
		return config::get_invalid();
	}
}

void server::delete_addon(const std::string& id)
{
	config& cfg = get_addon(id);

	if(!cfg) {
		ERR_CS << "Cannot delete unrecognized add-on '" << id << "'\n";
		return;
	}

	std::string fn = cfg["filename"].str();

	if(fn.empty()) {
		ERR_CS << "Add-on '" << id << "' does not have an associated filename, cannot delete\n";
	}

	if(!filesystem::delete_directory(fn)) {
		ERR_CS << "Could not delete the directory for addon '" << id
		       << "' (" << fn << "): " << strerror(errno) << '\n';
	}

	addons_.erase(id);
	write_config();

	fire("hook_post_erase", id);

	LOG_CS << "Deleted add-on '" << id << "'\n";
}

#define REGISTER_CAMPAIGND_HANDLER(req_id) \
	handlers_[#req_id] = std::bind(&server::handle_##req_id, \
		std::placeholders::_1, std::placeholders::_2)

void server::register_handlers()
{
	REGISTER_CAMPAIGND_HANDLER(request_campaign_list);
	REGISTER_CAMPAIGND_HANDLER(request_campaign);
	REGISTER_CAMPAIGND_HANDLER(request_terms);
	REGISTER_CAMPAIGND_HANDLER(upload);
	REGISTER_CAMPAIGND_HANDLER(delete);
	REGISTER_CAMPAIGND_HANDLER(change_passphrase);
}

void server::handle_request_campaign_list(const server::request& req)
{
	LOG_CS << "sending addons list to " << req.addr << " using gzip\n";

	std::time_t epoch = std::time(nullptr);
	config addons_list;

	addons_list["timestamp"] = epoch;
	if(req.cfg["times_relative_to"] != "now") {
		epoch = 0;
	}

	bool before_flag = false;
	std::time_t before = epoch;
	if(!req.cfg["before"].empty()) {
		before += req.cfg["before"].to_time_t();
		before_flag = true;
	}

	bool after_flag = false;
	std::time_t after = epoch;
	if(!req.cfg["after"].empty()) {
		after += req.cfg["after"].to_time_t();
		after_flag = true;
	}

	const std::string& name = req.cfg["name"];
	const std::string& lang = req.cfg["language"];

	for(const auto& addon : addons_)
	{
		if(!name.empty() && name != addon.first) {
			continue;
		}

		config i = addon.second;

		if(i["hidden"].to_bool()) {
			continue;
		}

		const auto& tm = i["timestamp"];

		if(before_flag && (tm.empty() || tm.to_time_t(0) >= before)) {
			continue;
		}
		if(after_flag && (tm.empty() || tm.to_time_t(0) <= after)) {
			continue;
		}

		if(!lang.empty()) {
			bool found = false;

			for(const config& j : i.child_range("translation"))
			{
				if(j["language"] == lang && j["supported"].to_bool(true)) {//for old addons
					found = true;
					break;
				}
			}

			if(!found) {
				continue;
			}
		}

		addons_list.add_child("campaign", i);
	}

	for(config& j : addons_list.child_range("campaign"))
	{
		// Remove attributes containing information that's considered sensitive
		// or irrelevant to clients
		j.remove_attributes("passphrase", "passhash", "passsalt", "upload_ip", "email");

		// Build a feedback_url string attribute from the internal [feedback]
		// data or deliver an empty value, in case clients decide to assume its
		// presence.
		const config& url_params = j.child_or_empty("feedback");
		j["feedback_url"] = !url_params.empty() && !feedback_url_format_.empty()
							? format_addon_feedback_url(feedback_url_format_, url_params) : "";

		// Clients don't need to see the original data, so discard it.
		j.clear_children("feedback");

		// Update packs info is internal stuff
		j.clear_children("update_pack");
	}

	config response;
	response.add_child("campaigns", std::move(addons_list));

	std::ostringstream ostr;
	write(ostr, response);
	std::string wml = ostr.str();
	simple_wml::document doc(wml.c_str(), simple_wml::INIT_STATIC);
	doc.compress();

	async_send_doc(req.sock, doc, std::bind(&server::handle_new_client, this, _1));
}

void server::handle_request_campaign(const server::request& req)
{
	config& campaign = get_addon(req.cfg["name"]);

	if(!campaign || campaign["hidden"].to_bool()) {
		send_error("Add-on '" + req.cfg["name"].str() + "' not found.", req.sock);
		return;
	}

	// The desired version is selected on the client side considering
	// the min_wesnoth_version in [version] children of the addon info (#TODO: gfgtdf's part)
	std::string to = req.cfg["version"].str();
	const std::string& from = req.cfg["from_version"].str();
	std::string full_pack = campaign["filename"].str();

	auto version_map = get_version_map(campaign);

	if(version_map.empty()) {
		// Old format ?
		ERR_CS << "The (" + to + ") version of the addon '" << req.cfg["name"].str()
			   << "' is stored in the old format (bug?)! Trying to send a legacy full-pack.";
		to = campaign["version"].str();
	} else {
		auto version = version_map.find(version_info(to));
		if(version != version_map.end()) {
			full_pack += version->second["filename"].str();
		} else {
			WRN_CS << "The selected version (" << to << ") for the addon '" << req.cfg["name"].str()
				   << "' has not been found or it is unspecified! Sending the latest version instead!\n";
			to = version_map.rbegin()->first;
			full_pack += version_map.rbegin()->second["filename"].str();
		}

		// Negotiate an update pack if possible
		if(!from.empty() && version_map.count(version_info(from)) != 0) {
			config pack_data;
			// Make a line of consequential updates beginning from the old version to the new one.
			// Every pair of increasing versions on the server side should contain an update_pack
			// transition guaranteed during the upload.

			auto iter = version_map.begin();
			int size = 0;
			bool failed = false;

			while(!failed && std::distance(iter, version_map.end()) > 1) {
				const config& prev_version = iter->second;
				iter++;
				const config& next_version = iter->second;

				for(const config& pack : campaign.child_range("update_pack")) {
					if(pack["from"].str() == prev_version["version"].str()
							&& pack["to"].str() == next_version["version"].str()) {
						config update_pack;
						filesystem::scoped_istream in = filesystem::istream_file(campaign["filename"].str() + pack["filename"].str());
						read_gz(update_pack, *in);
						if(update_pack) {
							pack_data.append(update_pack);
							size += filesystem::file_size(campaign["filename"].str() + pack["filename"].str());
						} else {
							WRN_CS << "Unable to create an update pack sequence from version (" << from << ") to ("
								   << to << ") for the addon '" << req.cfg["name"].str() << "'. A full pack will be sent instead!\n";
							failed = true;
							break;
						}
					}
				}
			}

			if(!failed && !pack_data.empty()) {
				std::ostringstream ostr;
				write(ostr, pack_data);
				std::string wml = ostr.str();

				simple_wml::document doc(wml.c_str(), simple_wml::INIT_STATIC);
				doc.compress();

				LOG_CS << "sending an update pack (" << from << "->" << to << ") for addon '" << req.cfg["name"] << "' to " << req.addr << " size: " << size / 1024 << "KiB\n";

				async_send_doc(req.sock, doc, std::bind(&server::handle_new_client, this, _1), null_handler);

				// The pack was successfully formed, no full file needed
				full_pack = "";
			}
		}
	}

	if(!full_pack.empty()) {
		// Send a full pack download if the previous version is not specified or is not present on the server, or if
		// we're dealing with the old format (???)
		const int size = filesystem::file_size(full_pack);

		if(size < 0) {
			send_error("Add-on '" + req.cfg["name"].str() + "' could not be read by the server.", req.sock);
			return;
		}

		LOG_CS << "sending campaign '" << req.cfg["name"] << "' to " << req.addr << " size: " << size / 1024 << "KiB\n";
		async_send_file(req.sock, full_pack, std::bind(&server::handle_new_client, this, _1), null_handler);
	}

	// Clients doing upgrades or some other specific thing shouldn't bump
	// the downloads count. Default to true for compatibility with old
	// clients that won't tell us what they are trying to do.
	if(req.cfg["increase_downloads"].to_bool(true) && !ignore_address_stats(req.addr)) {
		const int downloads = campaign["downloads"].to_int() + 1;
		campaign["downloads"] = downloads;
		dirty_addons_.emplace(req.cfg["name"]);
	}
}

void server::handle_request_terms(const server::request& req)
{
	// This usually means the client wants to upload content, so tell it
	// to give up when we're in read-only mode.
	if(read_only_) {
		LOG_CS << "in read-only mode, request for upload terms denied\n";
		send_error("The server is currently in read-only mode, add-on uploads are disabled.", req.sock);
		return;
	}

	// TODO: possibly move to server.cfg
	static const std::string terms = R"""(All content within add-ons uploaded to this server must be licensed under the terms of the GNU General Public License (GPL), with the sole exception of graphics and audio explicitly denoted as released under a Creative Commons license either in:

    a) a combined toplevel file, e.g. “My_Addon/ART_LICENSE”; <b>or</b>
    b) a file with the same path as the asset with “.license” appended, e.g. “My_Addon/images/units/axeman.png.license”.

<b>By uploading content to this server, you certify that you have the right to:</b>

    a) release all included art and audio explicitly denoted with a Creative Commons license in the proscribed manner under that license; <b>and</b>
    b) release all other included content under the terms of the GPL; and that you choose to do so.)""";

	LOG_CS << "sending terms " << req.addr << "\n";
	send_message(terms, req.sock);
	LOG_CS << " Done\n";
}

void server::handle_upload(const server::request& req)
{
	const config& upload = req.cfg;

	LOG_CS << "uploading campaign '" << upload["name"] << "' from " << req.addr << ".\n";
	config data = upload.child("data");

	const std::string& name = upload["name"];
	config *campaign = nullptr;

	bool passed_name_utf8_check = false;

	try {
		const std::string& lc_name = utf8::lowercase(name);
		passed_name_utf8_check = true;

		for(auto& c : addons_) {
			if(utf8::lowercase(c.first) == lc_name) {
				campaign = &c.second;
				break;
			}
		}
	} catch(const utf8::invalid_utf8_exception&) {
		if(!passed_name_utf8_check) {
			LOG_CS << "Upload aborted - invalid_utf8_exception caught on handle_upload() check 1, "
				   << "the add-on pbl info contains invalid UTF-8\n";
			send_error("Add-on rejected: The add-on name contains an invalid UTF-8 sequence.", req.sock);
		} else {
			LOG_CS << "Upload aborted - invalid_utf8_exception caught on handle_upload() check 2, "
				   << "the internal add-ons list contains invalid UTF-8\n";
			send_error("Server error: The server add-ons list is damaged.", req.sock);
		}

		return;
	}

	std::vector<std::string> badnames;

	if(read_only_) {
		LOG_CS << "Upload aborted - uploads not permitted in read-only mode.\n";
		send_error("Add-on rejected: The server is currently in read-only mode.", req.sock);
	} else if(!data) {
		LOG_CS << "Upload aborted - no add-on data.\n";
		send_error("Add-on rejected: No add-on data was supplied.", req.sock);
	} else if(!addon_name_legal(upload["name"])) {
		LOG_CS << "Upload aborted - invalid add-on name.\n";
		send_error("Add-on rejected: The name of the add-on is invalid.", req.sock);
	} else if(is_text_markup_char(upload["name"].str()[0])) {
		LOG_CS << "Upload aborted - add-on name starts with an illegal formatting character.\n";
		send_error("Add-on rejected: The name of the add-on starts with an illegal formatting character.", req.sock);
	} else if(upload["title"].empty()) {
		LOG_CS << "Upload aborted - no add-on title specified.\n";
		send_error("Add-on rejected: You did not specify the title of the add-on in the pbl file!", req.sock);
	} else if(is_text_markup_char(upload["title"].str()[0])) {
		LOG_CS << "Upload aborted - add-on title starts with an illegal formatting character.\n";
		send_error("Add-on rejected: The title of the add-on starts with an illegal formatting character.", req.sock);
	} else if(get_addon_type(upload["type"]) == ADDON_UNKNOWN) {
		LOG_CS << "Upload aborted - unknown add-on type specified.\n";
		send_error("Add-on rejected: You did not specify a known type for the add-on in the pbl file! (See PblWML: wiki.wesnoth.org/PblWML)", req.sock);
	} else if(upload["author"].empty()) {
		LOG_CS << "Upload aborted - no add-on author specified.\n";
		send_error("Add-on rejected: You did not specify the author(s) of the add-on in the pbl file!", req.sock);
	} else if(upload["version"].empty()) {
		LOG_CS << "Upload aborted - no add-on version specified.\n";
		send_error("Add-on rejected: You did not specify the version of the add-on in the pbl file!", req.sock);
	} else if(upload["description"].empty()) {
		LOG_CS << "Upload aborted - no add-on description specified.\n";
		send_error("Add-on rejected: You did not specify a description of the add-on in the pbl file!", req.sock);
	} else if(upload["email"].empty()) {
		LOG_CS << "Upload aborted - no add-on email specified.\n";
		send_error("Add-on rejected: You did not specify your email address in the pbl file!", req.sock);
	} else if(!check_names_legal(data, &badnames)) {
		const std::string& filelist = utils::join(badnames, "\n");
		LOG_CS << "Upload aborted - invalid file names in add-on data (" << badnames.size() << " entries).\n";
		send_error(
			"Add-on rejected: The add-on contains files or directories with illegal names. "
			// Note: the double double quote will be flattened to a single double quote.
			"File or directory names may not contain whitespace, control characters or any of the following characters: '\"\" * / : < > ? \\ | ~'. "
			"It also may not contain '..' end with '.' or be longer than 255 characters.",
			filelist, req.sock);
	} else if(!check_case_insensitive_duplicates(data, &badnames)) {
		const std::string& filelist = utils::join(badnames, "\n");
		LOG_CS << "Upload aborted - case conflict in add-on data (" << badnames.size() << " entries).\n";
		send_error(
			"Add-on rejected: The add-on contains files or directories with case conflicts. "
			"File or directory names may not be differently-cased versions of the same string.",
			filelist, req.sock);
	} else if(upload["passphrase"].empty()) {
		LOG_CS << "Upload aborted - missing passphrase.\n";
		send_error("No passphrase was specified.", req.sock);
	} else if(campaign && !authenticate(*campaign, upload["passphrase"])) {
		LOG_CS << "Upload aborted - incorrect passphrase.\n";
		send_error("Add-on rejected: The add-on already exists, and your passphrase was incorrect.", req.sock);
	} else if(campaign && (*campaign)["hidden"].to_bool()) {
		LOG_CS << "Upload denied - hidden add-on.\n";
		send_error("Add-on upload denied. Please contact the server administration for assistance.", req.sock);
	} else {
		const std::time_t upload_ts = std::time(nullptr);

		LOG_CS << "Upload is owner upload.\n";

		try {
			if(blacklist_.is_blacklisted(name,
										 upload["title"].str(),
										 upload["description"].str(),
										 upload["author"].str(),
										 req.addr,
										 upload["email"].str()))
			{
				LOG_CS << "Upload denied - blacklisted add-on information.\n";
				send_error("Add-on upload denied. Please contact the server administration for assistance.", req.sock);
				return;
			}
		} catch(const utf8::invalid_utf8_exception&) {
			LOG_CS << "Upload aborted - the add-on pbl info contains invalid UTF-8 and cannot be "
				   << "checked against the blacklist\n";
			send_error("Add-on rejected: The add-on publish information contains an invalid UTF-8 sequence.", req.sock);
			return;
		}

		const bool existing_upload = campaign != nullptr;

		std::string message = "Add-on accepted.";

		if(campaign == nullptr) {
			addons_.emplace(upload["name"].str(), config("original_timestamp", upload_ts));
			campaign = &get_addon(upload["name"].str());
		}

		(*campaign)["title"] = upload["title"];
		(*campaign)["name"] = upload["name"];
		(*campaign)["filename"] = "data/" + upload["name"].str();
		(*campaign)["author"] = upload["author"];
		(*campaign)["description"] = upload["description"];
		(*campaign)["version"] = upload["version"];
		//#TODO: add the gfgtdf's suggested part of min wesnoth version support
		(*campaign)["icon"] = upload["icon"];
		(*campaign)["translate"] = upload["translate"];
		(*campaign)["dependencies"] = upload["dependencies"];
		(*campaign)["upload_ip"] = req.addr;
		(*campaign)["type"] = upload["type"];
		(*campaign)["tags"] = upload["tags"];
		(*campaign)["email"] = upload["email"];

		if(!existing_upload) {
			set_passphrase(*campaign, upload["passphrase"]);
		}

		if((*campaign)["downloads"].empty()) {
			(*campaign)["downloads"] = 0;
		}
		(*campaign)["timestamp"] = upload_ts;

		int uploads = (*campaign)["uploads"].to_int() + 1;
		(*campaign)["uploads"] = uploads;

		(*campaign).clear_children("feedback");
		if(const config& url_params = upload.child("feedback")) {
			(*campaign).add_child("feedback", url_params);
		}

		(*campaign).clear_children("translation");
		for(const config& locale_params : upload.child_range("translation")) {
			if(!locale_params["language"].empty()) {
				config& locale = (*campaign).add_child("translation");
				locale["language"] = locale_params["language"].str();
				locale["supported"] = false;

				if(!locale_params["title"].empty()) {
					locale["title"] = locale_params["title"].str();
				}
				if(!locale_params["description"].empty()) {
					locale["description"] = locale_params["description"].str();
				}
			}
		}

		const std::string& filename = (*campaign)["filename"].str();
		data["name"] = "";
		find_translations(data, *campaign);

		add_license(data);
		
		const std::string& new_version = (*campaign)["version"].str();
		//#TODO: probably make hash from new_version + required_wesnoth_version ?
		const std::string& file_hash = utils::md5(new_version).hex_digest();

		//sorted version map
		auto version_map = get_version_map(*campaign);

		//#TODO: add gfgtdf's stuff about required_wesnoth_version here
		config version_cfg = config("version", new_version);
		version_cfg["filename"] = "/full_pack_" + file_hash + ".gz";

		version_map.erase(version_info(new_version));
		version_map.emplace(version_info(new_version), version_cfg);
		(*campaign).remove_children("version", [&new_version](const config& old_cfg) 
			{
				return old_cfg["version"].str() == new_version; 
			}
		);
		(*campaign).add_child("version", version_cfg);

		//Let's write the full_pack file
		{
			filesystem::atomic_commit campaign_file(filename + version_cfg["filename"].str());
			config_writer writer(*campaign_file.ostream(), true, compress_level_);
			writer.write(data);
			campaign_file.commit();
		}

		(*campaign)["size"] = filesystem::file_size(filename + version_cfg["filename"].str()) + filesystem::file_size(filename + "/addon.cfg");
		
		//Remove the update packs with expired lifespan
		for(const config& pack : (*campaign).child_range("update_pack")) {
			if(upload_ts > pack["expire"].to_time_t() || pack["from"].str() == new_version || pack["to"].str() == new_version) {
				const std::string& pack_filename = pack["filename"].str();
				filesystem::delete_file(filename + pack_filename);
				(*campaign).remove_children("update_pack", [&pack_filename](const config& child) 
					{
						return child["filename"].str() == pack_filename; 
					}
				);
			}
		}

		//Now let's fill in the gaps with missing incremental packs 
		//(which should mainly include the u-pack from the previous version to the present,
		//and from the present to the future version if either of them exists)
		auto iter = version_map.begin();

		while(std::distance(iter, version_map.end()) > 1) {
			const config& prev_version = iter->second;
			iter++;
			const config& next_version = iter->second;
			const std::string& prev_version_name = prev_version["version"].str();
			const std::string& next_version_name = next_version["version"].str();

			bool found = false;

			for(const config& pack : (*campaign).child_range("update_pack")) {
				if(pack["from"].str() == prev_version_name && pack["to"].str() == next_version_name) {
					found = true;
					break;
				}
			}

			// If we found it (meaning it hasn't expired yet) leave it for now,
			// else we'll bake a new pack
			if(!found) {
				if(filesystem::file_size(filename + prev_version["filename"].str()) <= 0
						|| filesystem::file_size(filename + next_version["filename"].str()) <= 0) {
					ERR_CS << "Unable to create an update pack for the addon " << (*campaign)["filename"].str()
							<< " when updating from version " << prev_version_name << " to " << next_version_name
							<< "!\n";
					continue;
				}
				config pack_info = config("from", prev_version_name, "to", next_version_name);
				pack_info["expire"] = upload_ts + update_pack_lifespan_;
				pack_info["filename"]
						= "/update_pack_" + utils::md5(prev_version_name + next_version_name).hex_digest() + ".gz";
				(*campaign).add_child("update_pack", pack_info);

				// Gather the full packs and create an update
				config pack;
				config from;
				config to;
				filesystem::scoped_istream in = filesystem::istream_file(filename + prev_version["filename"].str());
				read_gz(from, *in);
				in = filesystem::istream_file(filename + next_version["filename"].str());
				read_gz(to, *in);
				make_updatepack(pack, from, to);

				// Now write the update_pack archive in cached form
				{
					filesystem::atomic_commit pack_file(filename + pack_info["filename"].str());
					config_writer writer(*pack_file.ostream(), true, compress_level_);
					writer.write(pack);
					pack_file.commit();
				}
			}
		}

		// Mark the addon's info to be updated
		dirty_addons_.emplace((*campaign)["name"]);
		write_config();

		send_message(message, req.sock);

		fire("hook_post_upload", upload["name"]);
	}
}

void server::handle_delete(const server::request& req)
{
	const config& erase = req.cfg;
	const std::string& id = erase["name"].str();

	if(read_only_) {
		LOG_CS << "in read-only mode, request to delete '" << id << "' from " << req.addr << " denied\n";
		send_error("Cannot delete add-on: The server is currently in read-only mode.", req.sock);
		return;
	}

	LOG_CS << "deleting campaign '" << id << "' requested from " << req.addr << "\n";

	config& campaign = get_addon(id);

	if(!campaign) {
		send_error("The add-on does not exist.", req.sock);
		return;
	}

	const config::attribute_value& pass = erase["passphrase"];

	if(pass.empty()) {
		send_error("No passphrase was specified.", req.sock);
		return;
	}

	if(!authenticate(campaign, pass)) {
		send_error("The passphrase is incorrect.", req.sock);
		return;
	}

	if(campaign["hidden"].to_bool()) {
		LOG_CS << "Add-on removal denied - hidden add-on.\n";
		send_error("Add-on deletion denied. Please contact the server administration for assistance.", req.sock);
		return;
	}

	delete_addon(id);

	send_message("Add-on deleted.", req.sock);
}

void server::handle_change_passphrase(const server::request& req)
{
	const config& cpass = req.cfg;

	if(read_only_) {
		LOG_CS << "in read-only mode, request to change passphrase denied\n";
		send_error("Cannot change passphrase: The server is currently in read-only mode.", req.sock);
		return;
	}

	config& campaign = get_addon(cpass["name"]);

	if(!campaign) {
		send_error("No add-on with that name exists.", req.sock);
	} else if(!authenticate(campaign, cpass["passphrase"])) {
		send_error("Your old passphrase was incorrect.", req.sock);
	} else if(campaign["hidden"].to_bool()) {
		LOG_CS << "Passphrase change denied - hidden add-on.\n";
		send_error("Add-on passphrase change denied. Please contact the server administration for assistance.", req.sock);
	} else if(cpass["new_passphrase"].empty()) {
		send_error("No new passphrase was supplied.", req.sock);
	} else {
		set_passphrase(campaign, cpass["new_passphrase"]);
		dirty_addons_.emplace(campaign["name"]);
		write_config();
		send_message("Passphrase changed.", req.sock);
	}
}

} // end namespace campaignd

int main()
{
	game_config::path = filesystem::get_cwd();

	lg::set_log_domain_severity("campaignd", lg::info());
	lg::set_log_domain_severity("server", lg::info());
	lg::timestamps(true);

	try {
		std::cerr << "Wesnoth campaignd v" << game_config::revision << " starting...\n";

		const std::string cfg_path = filesystem::normalize_path("server.cfg");

		campaignd::server(cfg_path).run();
	} catch(const config::error& /*e*/) {
		std::cerr << "Could not parse config file\n";
		return 1;
	} catch(const filesystem::io_exception& e) {
		std::cerr << "File I/O error: " << e.what() << "\n";
		return 2;
	} catch(const std::bad_function_call& /*e*/) {
		std::cerr << "Bad request handler function call\n";
		return 4;
	}

	return 0;
}
