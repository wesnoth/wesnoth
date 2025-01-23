/*
	Copyright (C) 2015 - 2024
	by Iris Morelle <shadowm2006@gmail.com>
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
#include "serialization/chrono.hpp"
#include "serialization/parser.hpp"
#include "serialization/string_utils.hpp"
#include "serialization/unicode.hpp"
#include "addon/validation.hpp"
#include "server/campaignd/addon_utils.hpp"
#include "server/campaignd/auth.hpp"
#include "server/campaignd/blacklist.hpp"
#include "server/campaignd/control.hpp"
#include "server/campaignd/fs_commit.hpp"
#include "server/campaignd/options.hpp"
#include "game_version.hpp"
#include "hash.hpp"
#include "utils/optimer.hpp"

#ifdef HAVE_MYSQLPP
#include "server/common/forum_user_handler.hpp"
#endif

#include <csignal>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <utility>

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

namespace campaignd {

namespace {

/**
 * campaignd capabilities supported by this version of the server.
 *
 * These are advertised to clients using the @a [server_id] command. They may
 * be disabled or re-enabled at runtime.
 */
const std::set<std::string> cap_defaults = {
	// Legacy item and passphrase-based authentication
	"auth:legacy",
	// Delta WML packs
	"delta",
};

/**
 * Default URL to the add-ons server web index.
 */
const std::string default_web_url = "https://add-ons.wesnoth.org/";

/**
 * Default license terms for content uploaded to the server.
 *
 * This used by both the @a [server_id] command and @a [request_terms] in
 * their responses.
 *
 * The text is intended for display on the client with Pango markup enabled and
 * sent by the server as-is, so it ought to be formatted accordingly.
 */
const std::string default_license_notice = R"(<span size='x-large'>General Rules</span>

The current version of the server rules can be found at: https://r.wesnoth.org/t51347

<span color='#f88'>Any content that does not conform to the rules listed at the link above, as well as the licensing terms below, may be removed at any time without prior notice.</span>

<span size='x-large'>Licensing</span>

All content within add-ons uploaded to this server must be licensed under the terms of the GNU General Public License (GPL), version 2 or later, with the sole exception of graphics and audio explicitly denoted as released under a Creative Commons license either in:

  a) a combined toplevel file, e.g. “<span font_family='monospace'>My_Addon/ART_LICENSE</span>”; <b>or</b>
  b) a file with the same path as the asset with “<span font_family='monospace'>.license</span>” appended, e.g. “<span font_family='monospace'>My_Addon/images/units/axeman.png.license</span>”.

<b>By uploading content to this server, you certify that you have the right to:</b>

  a) release all included art and audio explicitly denoted with a Creative Commons license in the prescribed manner under that license; <b>and</b>
  b) release all other included content under the terms of the chosen versions of the GNU GPL.)";

bool timing_reports_enabled = false;

void timing_report_function(const utils::ms_optimer& tim, const campaignd::server::request& req, const std::string& label = {})
{
	if(timing_reports_enabled) {
		if(label.empty()) {
			LOG_CS << req << "Time elapsed: " << tim << " ms";
		} else {
			LOG_CS << req << "Time elapsed [" << label << "]: " << tim << " ms";
		}
	}
}

inline utils::ms_optimer service_timer(const campaignd::server::request& req, const std::string& label = {})
{
	return utils::ms_optimer{std::bind(timing_report_function, std::placeholders::_1, req, label)};
}

//
// Auxiliary shortcut functions
//

/**
 * WML version of campaignd::auth::verify_passphrase().
 * The salt and hash are retrieved from the @a passsalt and @a passhash attributes, respectively, if not using forum_auth.
 *
 * @param addon The config of the addon being authenticated for upload.
 * @param passphrase The password being validated.
 * @return Whether the password is correct for the addon.
 */
inline bool authenticate(config& addon, const config::attribute_value& passphrase)
{
	if(!addon["forum_auth"].to_bool()) {
		return auth::verify_passphrase(passphrase, addon["passsalt"], addon["passhash"]);
	}
	return false;
}

/**
 * WML version of campaignd::auth::generate_hash().
 * The salt and hash are written into the @a passsalt and @a passhash attributes, respectively, if not using forum_auth.
 *
 * @param addon The addon whose password is being set.
 * @param passphrase The password being hashed.
 */
inline void set_passphrase(config& addon, const std::string& passphrase)
{
	// don't do anything if using forum authorization
	if(!addon["forum_auth"].to_bool()) {
		std::tie(addon["passsalt"], addon["passhash"]) = auth::generate_hash(passphrase);
	}
}

/**
 * Returns the update pack filename for the specified old/new version pair.
 *
 * The filename is in the form @p "update_pack_<VERSION_MD5>.gz".
 */
inline std::string make_update_pack_filename(const std::string& old_version, const std::string& new_version)
{
	return "update_pack_" + utils::md5(old_version + new_version).hex_digest() + ".gz";
}

/**
 * Returns the full pack filename for the specified version.
 *
 * The filename is in the form @p "full_pack_<VERSION_MD5>.gz".
 */
inline std::string make_full_pack_filename(const std::string& version)
{
	return "full_pack_" + utils::md5(version).hex_digest() + ".gz";
}

/**
 * Returns the index filename for the specified version.
 *
 * The filename is in the form @p "full_pack_<VERSION_MD5>.hash.gz".
 */
inline std::string make_index_filename(const std::string& version)
{
	return "full_pack_" + utils::md5(version).hex_digest() + ".hash.gz";
}

/**
 * Returns the index counterpart for the specified full pack file.
 *
 * The result is in the same form as make_index_filename().
 */
inline std::string index_from_full_pack_filename(std::string pack_fn)
{
	auto dot_pos = pack_fn.find_last_of('.');
	if(dot_pos != std::string::npos) {
		pack_fn.replace(dot_pos, std::string::npos, ".hash.gz");
	}
	return pack_fn;
}

/**
 * Returns @a false if @a cfg is null or empty.
 */
bool have_wml(const utils::optional_reference<const config>& cfg)
{
	return cfg && !cfg->empty();
}

/**
 * Scans multiple WML pack-like trees for illegal names.
 *
 * Null WML objects are skipped.
 */
template<typename... Vals>
utils::optional<std::vector<std::string>> multi_find_illegal_names(const Vals&... args)
{
	std::vector<std::string> names;
	((args && check_names_legal(*args, &names)), ...);

	return !names.empty() ? utils::optional(names) : utils::nullopt;
}

/**
 * Scans multiple WML pack-like trees for case conflicts.
 *
 * Null WML objects are skipped.
 */
template<typename... Vals>
utils::optional<std::vector<std::string>> multi_find_case_conflicts(const Vals&... args)
{
	std::vector<std::string> names;
	((args && check_case_insensitive_duplicates(*args, &names)), ...);

	return !names.empty() ? utils::optional(names) : utils::nullopt;
}

/**
 * Escapes double quotes intended to be passed into simple_wml.
 *
 * Just why does simple_wml have to be so broken to force us to use this, though?
 */
std::string simple_wml_escape(const std::string& text)
{
	std::string res;
	auto it = text.begin();

	while(it != text.end()) {
		res.append(*it == '"' ? 2 : 1, *it);
		++it;
	}

	return res;
}

} // end anonymous namespace

server::server(const std::string& cfg_file, unsigned short port)
	: server_base(default_campaignd_port, true)
	, user_handler_(nullptr)
	, capabilities_(cap_defaults)
	, addons_()
	, dirty_addons_()
	, cfg_()
	, cfg_file_(cfg_file)
	, read_only_(false)
	, compress_level_(0)
	, update_pack_lifespan_(0)
	, strict_versions_(true)
	, hooks_()
	, handlers_()
	, server_id_()
	, feedback_url_format_()
	, web_url_()
	, license_notice_()
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

	// Command line config override. This won't get saved back to disk since we
	// leave the WML intentionally untouched.
	if(port != 0) {
		port_ = port;
	}

	LOG_CS << "Port: " << port_;
	LOG_CS << "Server directory: " << game_config::path << " (" << addons_.size() << " add-ons)";

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
	LOG_CS << "Reading configuration from " << cfg_file_ << "...";

	filesystem::scoped_istream in = filesystem::istream_file(cfg_file_);
	read(cfg_, *in);

	read_only_ = cfg_["read_only"].to_bool(false);

	if(read_only_) {
		LOG_CS << "READ-ONLY MODE ACTIVE";
	}

	strict_versions_ = cfg_["strict_versions"].to_bool(true);

	// Seems like compression level above 6 is a waste of CPU cycles.
	compress_level_ = cfg_["compress_level"].to_int(6);
	// One month probably will be fine (#TODO: testing needed)
	constexpr std::chrono::seconds seconds_in_a_month{30 * 24 * 60 * 60};
	update_pack_lifespan_ = chrono::parse_duration(cfg_["update_pack_lifespan"], seconds_in_a_month);

	const auto& svinfo_cfg = server_info();

	server_id_ = svinfo_cfg["id"].str();
	feedback_url_format_ = svinfo_cfg["feedback_url_format"].str();
	web_url_ = svinfo_cfg["web_url"].str(default_web_url);
	license_notice_ = svinfo_cfg["license_notice"].str(default_license_notice);

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
				ERR_CS << "could not make fifo at '" << path << "' (" << strerror(errno) << ")";
			} else {
				input_.close();
				int fifo = open(path.c_str(), O_RDWR|O_NONBLOCK);
				input_.assign(fifo);
				LOG_CS << "opened fifo at '" << path << "'. Server commands may be written to this file.";
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
		if(!meta.empty()) {
			addons_.emplace(meta["name"].str(), meta);
		} else {
			throw filesystem::io_exception("Failed to load addon from dir '" + addon_dir + "'\n");
		}
	}

	// Convert all legacy addons to the new format on load
	if(cfg_.has_child("campaigns")) {
		config& campaigns = cfg_.mandatory_child("campaigns");
		WRN_CS << "Old format addons have been detected in the config! They will be converted to the new file format! "
		       << campaigns.child_count("campaign") << " entries to be processed.";
		for(config& campaign : campaigns.child_range("campaign")) {
			const std::string& addon_id = campaign["name"].str();
			const std::string& addon_file = campaign["filename"].str();
			if(get_addon(addon_id)) {
				throw filesystem::io_exception("The addon '" + addon_id
					   + "' already exists in the new form! Possible code or filesystem interference!\n");
			}
			if(std::find(legacy_addons.begin(), legacy_addons.end(), addon_id) == legacy_addons.end()) {
				throw filesystem::io_exception("No file has been found for the legacy addon '" + addon_id
					   + "'. Check the file structure!\n");
			}

			config data;
			in = filesystem::istream_file(filesystem::normalize_path(addon_file));
			read_gz(data, *in);
			if (data.empty()) {
				throw filesystem::io_exception("Couldn't read the content file for the legacy addon '" + addon_id + "'!\n");
			}
			config version_cfg = config("version", campaign["version"].str());
			version_cfg["filename"] = make_full_pack_filename(campaign["version"]);
			campaign.add_child("version", version_cfg);

			data.remove_attributes("title", "campaign_name", "author", "description", "version", "timestamp", "original_timestamp", "icon", "type", "tags");
			filesystem::delete_file(filesystem::normalize_path(addon_file));
			{
				filesystem::atomic_commit campaign_file(addon_file + "/" + version_cfg["filename"].str());
				config_writer writer(*campaign_file.ostream(), true, compress_level_);
				writer.write(data);
				campaign_file.commit();
			}
			{
				filesystem::atomic_commit campaign_hash_file(addon_file + "/" + make_index_filename(campaign["version"]));
				config_writer writer(*campaign_hash_file.ostream(), true, compress_level_);
				config data_hash = config("name", "");
				write_hashlist(data_hash, data);
				writer.write(data_hash);
				campaign_hash_file.commit();
			}

			addons_.emplace(addon_id, campaign);
			mark_dirty(addon_id);
		}
		cfg_.clear_children("campaigns");
		LOG_CS << "Legacy addons processing finished.";
		write_config();
	}

	LOG_CS << "Loaded addons metadata. " << addons_.size() << " addons found.";

#ifdef HAVE_MYSQLPP
	if(auto user_handler = cfg_.optional_child("user_handler")) {
		if(server_id_ == "") {
			ERR_CS << "The server id must be set when database support is used.";
			exit(1);
		}
		user_handler_.reset(new fuh(*user_handler));
		LOG_CS << "User handler initialized.";
	}
#endif

	load_tls_config(cfg_);
}

std::ostream& operator<<(std::ostream& o, const server::request& r)
{
	o << '[' << (utils::holds_alternative<tls_socket_ptr>(r.sock) ? "+" : "") << r.addr << ' ' << r.cmd << "] ";
	return o;
}

void server::handle_new_client(tls_socket_ptr socket)
{
	boost::asio::spawn(
		io_service_, [this, socket](boost::asio::yield_context yield) { serve_requests(socket, std::move(yield)); }
#if BOOST_VERSION >= 108000
		, [](const std::exception_ptr& e) { if (e) std::rethrow_exception(e); }
#endif
	);
}

void server::handle_new_client(socket_ptr socket)
{
	boost::asio::spawn(
		io_service_, [this, socket](boost::asio::yield_context yield) { serve_requests(socket, std::move(yield)); }
#if BOOST_VERSION >= 108000
		, [](const std::exception_ptr& e) { if (e) std::rethrow_exception(e); }
#endif
	);
}

template<class Socket>
void server::serve_requests(Socket socket, boost::asio::yield_context yield)
{
	while(true) {
		auto doc { coro_receive_doc(socket, yield) };
		if(!doc) {
			socket->lowest_layer().close();
			return;
		}

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
				request req{c.key, c.cfg, socket, yield};
				auto st = service_timer(req);
				j->second(this, req);
			} else {
				send_error("Unrecognized [" + c.key + "] request.",socket);
			}
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
		ERR_CS << "Error reading from fifo: " << error.message();
		return;
	}

	std::istream is(&admin_cmd_);
	std::string cmd;
	std::getline(is, cmd);

	const control_line ctl = cmd;

	if(ctl == "shut_down") {
		LOG_CS << "Shut down requested by admin, shutting down...";
		BOOST_THROW_EXCEPTION(server_shutdown("Shut down via fifo command"));
	} else if(ctl == "readonly") {
		if(ctl.args_count()) {
			cfg_["read_only"] = read_only_ = utils::string_bool(ctl[1], true);
		}

		LOG_CS << "Read only mode: " << (read_only_ ? "enabled" : "disabled");
	} else if(ctl == "flush") {
		LOG_CS << "Flushing config to disk...";
		write_config();
	} else if(ctl == "reload") {
		if(ctl.args_count()) {
			if(ctl[1] == "blacklist") {
				LOG_CS << "Reloading blacklist...";
				load_blacklist();
			} else {
				ERR_CS << "Unrecognized admin reload argument: " << ctl[1];
			}
		} else {
			LOG_CS << "Reloading all configuration...";
			load_config();
			LOG_CS << "Reloaded configuration";
		}
	} else if(ctl == "delete") {
		if(ctl.args_count() != 1) {
			ERR_CS << "Incorrect number of arguments for 'delete'";
		} else {
			const std::string& addon_id = ctl[1];

			LOG_CS << "deleting add-on '" << addon_id << "' requested from control FIFO";
			delete_addon(addon_id);
		}
	} else if(ctl == "hide" || ctl == "unhide") {
		// there are also hides/unhides handler methods
		if(ctl.args_count() != 1) {
			ERR_CS << "Incorrect number of arguments for '" << ctl.cmd() << "'";
		} else {
			const std::string& addon_id = ctl[1];
			auto addon = get_addon(addon_id);

			if(!addon) {
				ERR_CS << "Add-on '" << addon_id << "' not found, cannot " << ctl.cmd();
			} else {
				addon["hidden"] = (ctl.cmd() == "hide");
				mark_dirty(addon_id);
				write_config();
				LOG_CS << "Add-on '" << addon_id << "' is now " << (ctl.cmd() == "hide" ? "hidden" : "unhidden");
			}
		}
	} else if(ctl == "setpass") {
		if(ctl.args_count() != 2) {
			ERR_CS << "Incorrect number of arguments for 'setpass'";
		} else {
			const std::string& addon_id = ctl[1];
			const std::string& newpass = ctl[2];
			auto addon = get_addon(addon_id);

			if(!addon) {
				ERR_CS << "Add-on '" << addon_id << "' not found, cannot set passphrase";
			} else if(newpass.empty()) {
				// Shouldn't happen!
				ERR_CS << "Add-on passphrases may not be empty!";
			} else if(addon["forum_auth"].to_bool()) {
				ERR_CS << "Can't set passphrase for add-on using forum_auth.";
			} else {
				set_passphrase(*addon, newpass);
				mark_dirty(addon_id);
				write_config();
				LOG_CS << "New passphrase set for '" << addon_id << "'";
			}
		}
	} else if(ctl == "setattr") {
		if(ctl.args_count() < 3) {
			ERR_CS << "Incorrect number of arguments for 'setattr'";
		} else {
			const std::string& addon_id = ctl[1];
			const std::string& key = ctl[2];
			std::string value;
			for (std::size_t i = 3; i <= ctl.args_count(); ++i) {
				if(i > 3) {
					value += ' ';
				}
				value += ctl[i];
			}

			auto addon = get_addon(addon_id);

			if(!addon) {
				ERR_CS << "Add-on '" << addon_id << "' not found, cannot set attribute";
			} else if(key == "name" || key == "version") {
				ERR_CS << "setattr cannot be used to rename add-ons or change their version";
			} else if(key == "passhash"|| key == "passsalt") {
				ERR_CS << "setattr cannot be used to set auth data -- use setpass instead";
			} else if(!addon->has_attribute(key)) {
				// NOTE: This is a very naive approach for validating setattr's
				//       input, but it should generally work since add-on
				//       uploads explicitly set all recognized attributes to
				//       the values provided by the .pbl data or the empty
				//       string if absent, and this is normally preserved by
				//       the config serialization.
				ERR_CS << "Attribute '" << key << "' is not a recognized add-on attribute";
			} else {
				addon[key] = value;
				mark_dirty(addon_id);
				write_config();
				LOG_CS << "Set attribute on add-on '" << addon_id << "':\n"
				       << key << "=\"" << value << "\"";
			}
		}
	} else if(ctl == "log") {
		static const std::map<std::string, lg::severity> log_levels = {
			{ "error",   lg::err().get_severity() },
			{ "warning", lg::warn().get_severity() },
			{ "info",    lg::info().get_severity() },
			{ "debug",   lg::debug().get_severity() },
			{ "none",    lg::severity::LG_NONE }
		};

		if(ctl.args_count() != 2) {
			ERR_CS << "Incorrect number of arguments for 'log'";
		} else if(ctl[1] == "precise") {
			if(ctl[2] == "on") {
				lg::precise_timestamps(true);
				LOG_CS << "Precise timestamps enabled";
			} else if(ctl[2] == "off") {
				lg::precise_timestamps(false);
				LOG_CS << "Precise timestamps disabled";
			} else {
				ERR_CS << "Invalid argument for 'log precise': " << ctl[2];
			}
		} else if(log_levels.find(ctl[1]) == log_levels.end()) {
			ERR_CS << "Invalid log level '" << ctl[1] << "'";
		} else {
			auto sev = log_levels.find(ctl[1])->second;
			for(const auto& domain : utils::split(ctl[2])) {
				if(!lg::set_log_domain_severity(domain, sev)) {
					ERR_CS << "Unknown log domain '" << domain << "'";
				} else {
					LOG_CS << "Set log level for domain '" << domain << "' to " << ctl[1];
				}
			}
		}
	} else if(ctl == "timings") {
		if(ctl.args_count() != 1) {
			ERR_CS << "Incorrect number of arguments for 'timings'";
		} else if(ctl[1] == "on") {
			campaignd::timing_reports_enabled = true;
			LOG_CS << "Request servicing timing reports enabled";
		} else if(ctl[1] == "off") {
			campaignd::timing_reports_enabled = false;
			LOG_CS << "Request servicing timing reports disabled";
		} else {
			ERR_CS << "Invalid argument for 'timings': " << ctl[1];
		}
	} else {
		ERR_CS << "Unrecognized admin command: " << ctl.full();
	}

	read_from_fifo();
}

void server::handle_sighup(const boost::system::error_code&, int)
{
	LOG_CS << "SIGHUP caught, reloading config.";

	load_config(); // TODO: handle port number config changes

	LOG_CS << "Reloaded configuration";

	sighup_.async_wait(std::bind(&server::handle_sighup, this, std::placeholders::_1, std::placeholders::_2));
}

#endif

void server::flush_cfg()
{
	flush_timer_.expires_after(std::chrono::minutes(10));
	flush_timer_.async_wait(std::bind(&server::handle_flush, this, std::placeholders::_1));
}

void server::handle_flush(const boost::system::error_code& error)
{
	if(error) {
		ERR_CS << "Error from reload timer: " << error.message();
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
		LOG_CS << "using blacklist from " << blacklist_file_;
	} catch(const config::error&) {
		ERR_CS << "failed to read blacklist from " << blacklist_file_ << ", blacklist disabled";
	}
}

void server::write_config()
{
	DBG_CS << "writing configuration and add-ons list to disk...";
	filesystem::atomic_commit out(cfg_file_);
	write(*out.ostream(), cfg_);
	out.commit();

	for(const std::string& name : dirty_addons_) {
		auto addon = get_addon(name);
		if(addon && !addon["filename"].empty()) {
			filesystem::atomic_commit addon_out(filesystem::normalize_path(addon["filename"].str() + "/addon.cfg"));
			write(*addon_out.ostream(), *addon);
			addon_out.commit();
		}
	}

	dirty_addons_.clear();
	DBG_CS << "... done";
}

void server::fire(const std::string& hook, [[maybe_unused]] const std::string& addon)
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
	ERR_CS << "Tried to execute a script on an unsupported platform";
	return;
#else
	pid_t childpid;

	if((childpid = fork()) == -1) {
		ERR_CS << "fork failed while updating add-on " << addon;
		return;
	}

	if(childpid == 0) {
		// We are the child process. Execute the script. We run as a
		// separate thread sharing stdout/stderr, which will make the
		// log look ugly.
		execlp(script.c_str(), script.c_str(), addon.c_str(), static_cast<char *>(nullptr));

		// exec() and family never return; if they do, we have a problem
		PLAIN_LOG << "ERROR: exec failed with errno " << errno << " for addon " << addon;
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

void server::send_message(const std::string& msg, const any_socket_ptr& sock)
{
	const auto& escaped_msg = simple_wml_escape(msg);
	simple_wml::document doc;
	doc.root().add_child("message").set_attr_dup("message", escaped_msg.c_str());
	utils::visit([this, &doc](auto&& sock) { async_send_doc_queued(sock, doc); }, sock);
}

inline std::string client_address(const any_socket_ptr& sock) {
	return utils::visit([](auto&& sock) { return ::client_address(sock); }, sock);
}

void server::send_error(const std::string& msg, const any_socket_ptr& sock)
{
	ERR_CS << "[" << client_address(sock) << "] " << msg;
	const auto& escaped_msg = simple_wml_escape(msg);
	simple_wml::document doc;
	doc.root().add_child("error").set_attr_dup("message", escaped_msg.c_str());
	utils::visit([this, &doc](auto&& sock) { async_send_doc_queued(sock, doc); }, sock);
}

void server::send_error(const std::string& msg, const std::string& extra_data, unsigned int status_code, const any_socket_ptr& sock)
{
	const std::string& status_hex = formatter()
		<< "0x" << std::setfill('0') << std::setw(2*sizeof(unsigned int)) << std::hex
		<< std::uppercase << status_code;
	ERR_CS << "[" << client_address(sock) << "]: (" << status_hex << ") " << msg;

	const auto& escaped_status_str = simple_wml_escape(std::to_string(status_code));
	const auto& escaped_msg = simple_wml_escape(msg);
	const auto& escaped_extra_data = simple_wml_escape(extra_data);

	simple_wml::document doc;
	simple_wml::node& err_cfg = doc.root().add_child("error");

	err_cfg.set_attr_dup("message", escaped_msg.c_str());
	err_cfg.set_attr_dup("extra_data", escaped_extra_data.c_str());
	err_cfg.set_attr_dup("status_code", escaped_status_str.c_str());

	utils::visit([this, &doc](auto&& sock) { async_send_doc_queued(sock, doc); }, sock);
}

optional_config server::get_addon(const std::string& id)
{
	auto addon = addons_.find(id);
	if(addon != addons_.end()) {
		return addon->second;
	} else {
		return optional_config();
	}
}

void server::delete_addon(const std::string& id)
{
	optional_const_config cfg = get_addon(id);

	if(!cfg) {
		ERR_CS << "Cannot delete unrecognized add-on '" << id << "'";
		return;
	}

	if(cfg["forum_auth"].to_bool()) {
		user_handler_->db_delete_addon_authors(server_id_, cfg["name"].str());
	}

	std::string fn = cfg["filename"].str();

	if(fn.empty()) {
		ERR_CS << "Add-on '" << id << "' does not have an associated filename, cannot delete";
	}

	if(!filesystem::delete_directory(fn)) {
		ERR_CS << "Could not delete the directory for addon '" << id
		       << "' (" << fn << "): " << strerror(errno);
	}

	addons_.erase(id);
	write_config();

	fire("hook_post_erase", id);

	LOG_CS << "Deleted add-on '" << id << "'";
}

#define REGISTER_CAMPAIGND_HANDLER(req_id) \
	handlers_[#req_id] = std::bind(&server::handle_##req_id, \
		std::placeholders::_1, std::placeholders::_2)

void server::register_handlers()
{
	REGISTER_CAMPAIGND_HANDLER(server_id);
	REGISTER_CAMPAIGND_HANDLER(request_campaign_list);
	REGISTER_CAMPAIGND_HANDLER(request_campaign);
	REGISTER_CAMPAIGND_HANDLER(request_campaign_hash);
	REGISTER_CAMPAIGND_HANDLER(request_terms);
	REGISTER_CAMPAIGND_HANDLER(upload);
	REGISTER_CAMPAIGND_HANDLER(delete);
	REGISTER_CAMPAIGND_HANDLER(change_passphrase);
	REGISTER_CAMPAIGND_HANDLER(hide_addon);
	REGISTER_CAMPAIGND_HANDLER(unhide_addon);
	REGISTER_CAMPAIGND_HANDLER(list_hidden);
	REGISTER_CAMPAIGND_HANDLER(addon_downloads_by_version);
	REGISTER_CAMPAIGND_HANDLER(forum_auth_usage);
	REGISTER_CAMPAIGND_HANDLER(admins_list);
}

void server::handle_server_id(const server::request& req)
{
	DBG_CS << req << "Sending server identification";

	std::ostringstream ostr;
	write(ostr, config{"server_id", config{
		"id",					server_id_,
		"cap",					utils::join(capabilities_),
		"version",				game_config::revision,
		"url",					web_url_,
		"license_notice",		license_notice_,
	}});

	const auto& wml = ostr.str();
	simple_wml::document doc(wml.c_str(), simple_wml::INIT_STATIC);
	doc.compress();

	utils::visit([this, &doc](auto&& sock) { async_send_doc_queued(sock, doc); }, req.sock);
}

void server::handle_request_campaign_list(const server::request& req)
{
	LOG_CS << req << "Sending add-ons list";

	auto now = std::chrono::system_clock::now();
	const bool relative_to_now = req.cfg["times_relative_to"] == "now";

	config addons_list;
	addons_list["timestamp"] = chrono::serialize_timestamp(now);

	bool before_flag = !req.cfg["before"].empty();
	std::chrono::system_clock::time_point before;
	if(before_flag) {
		if(relative_to_now) {
			auto time_delta = chrono::parse_duration<std::chrono::seconds>(req.cfg["before"]);
			before = now + time_delta; // delta may be negative
		} else {
			before = chrono::parse_timestamp(req.cfg["before"]);
		}
	}

	bool after_flag = !req.cfg["after"].empty();
	std::chrono::system_clock::time_point after;
	if(after_flag) {
		if(relative_to_now) {
			auto time_delta = chrono::parse_duration<std::chrono::seconds>(req.cfg["after"]);
			after = now + time_delta; // delta may be negative
		} else {
			after = chrono::parse_timestamp(req.cfg["after"]);
		}
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

		if(before_flag && (tm.empty() || chrono::parse_timestamp(tm) >= before)) {
			continue;
		}
		if(after_flag && (tm.empty() || chrono::parse_timestamp(tm) <= after)) {
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

		// don't include icons if requested
		if(!req.cfg["send_icons"].to_bool(true)) {
			j.remove_attribute("icon");
		}

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

	utils::visit([this, &doc](auto&& sock) { async_send_doc_queued(sock, doc); }, req.sock);
}

void server::handle_request_campaign(const server::request& req)
{
	auto addon = get_addon(req.cfg["name"]);

	if(!addon || addon["hidden"].to_bool()) {
		send_error("Add-on '" + req.cfg["name"].str() + "' not found.", req.sock);
		return;
	}

	const auto& name = req.cfg["name"].str();
	auto version_map = get_version_map(*addon);

	if(version_map.empty()) {
		send_error("No versions of the add-on '" + name + "' are available on the server.", req.sock);
		return;
	}

	// Base the payload against the latest version if no particular version is being requested
	const auto& from = req.cfg["from_version"].str();
	const auto& to = req.cfg["version"].str(version_map.rbegin()->first);

	const version_info from_parsed{from};
	const version_info to_parsed{to};

	auto to_version_iter = version_map.find(to_parsed);
	if(to_version_iter == version_map.end()) {
		send_error("Could not find requested version " + to + " of the addon '" + name +
					"'.", req.sock);
		return;
	}

	auto full_pack_path = addon["filename"].str() + '/' + to_version_iter->second["filename"].str();
	const int full_pack_size = filesystem::file_size(full_pack_path);

	// Assert `From < To` before attempting to do build an update sequence, since std::distance(A, B)
	// requires A <= B to avoid UB with std::map (which doesn't support random access iterators) and
	// we're going to be using that a lot next. We also can't do anything fancy with downgrades anyway,
	// and same-version downloads can be regarded as though no From version was specified in order to
	// keep things simple.

	if(!from.empty() && from_parsed < to_parsed && version_map.count(from_parsed) != 0) {
		// Build a sequence of updates beginning from the client's old version to the
		// requested version. Every pair of incrementing versions on the server should
		// have an update pack written to disk during the original upload(s).
		//
		// TODO: consider merging update packs instead of building a linear
		// and possibly redundant sequence out of them.

		config delta;
		int delivery_size = 0;
		bool force_use_full = false;

		auto start_point = version_map.find(from_parsed); // Already known to exist
		auto end_point = std::next(to_version_iter, 1); // May be end()

		if(std::distance(start_point, end_point) <= 1) {
			// This should not happen, skip the sequence build entirely
			ERR_CS << "Bad update sequence bounds in version " << from << " -> " << to << " update sequence for the add-on '" << name << "', sending a full pack instead";
			force_use_full = true;
		}

		for(auto iter = start_point; !force_use_full && std::distance(iter, end_point) > 1;) {
			const auto& prev_version_cfg = iter->second;
			const auto& next_version_cfg = (++iter)->second;

			for(const config& pack : addon->child_range("update_pack")) {
				if(pack["from"].str() != prev_version_cfg["version"].str() ||
				   pack["to"].str() != next_version_cfg["version"].str()) {
					continue;
				}

				config step_delta;
				const auto& update_pack_path = addon["filename"].str() + '/' + pack["filename"].str();
				auto in = filesystem::istream_file(update_pack_path);

				read_gz(step_delta, *in);

				if(!step_delta.empty()) {
					// Don't copy arbitrarily large data around
					delta.append(std::move(step_delta));
					delivery_size += filesystem::file_size(update_pack_path);
				} else {
					ERR_CS << "Broken update sequence from version " << from << " to "
							<< to << " for the add-on '" << name << "', sending a full pack instead";
					force_use_full = true;
					break;
				}

				// No point in sending an overlarge delta update.
				// FIXME: This doesn't take into account over-the-wire compression
				// from async_send_doc() though, maybe some heuristics based on
				// individual update pack size would be useful?
				if(delivery_size > full_pack_size && full_pack_size > 0) {
					force_use_full = true;
					break;
				}
			}
		}

		if(!force_use_full && !delta.empty()) {
			std::ostringstream ostr;
			write(ostr, delta);
			const auto& wml_text = ostr.str();

			simple_wml::document doc(wml_text.c_str(), simple_wml::INIT_STATIC);
			doc.compress();

			LOG_CS << req << "Sending add-on '" << name << "' version: " << from << " -> " << to << " (delta)";

			utils::visit([this, &req, &doc](auto && sock) {
				coro_send_doc(sock, doc, req.yield);
			}, req.sock);

			full_pack_path.clear();
		}
	}

	// Send a full pack if the client's previous version was not specified, is
	// not known by the server, or if any other condition above caused us to
	// give up on the update pack option.
	if(!full_pack_path.empty()) {
		if(full_pack_size < 0) {
			send_error("Add-on '" + name + "' could not be read by the server.", req.sock);
			return;
		}

		LOG_CS << req << "Sending add-on '" << name << "' version: " << to << " size: " << full_pack_size / 1024 << " KiB";
		utils::visit([this, &req, &full_pack_path](auto&& socket) {
			coro_send_file(socket, full_pack_path, req.yield);
		}, req.sock);
	}

	// Clients doing upgrades or some other specific thing shouldn't bump
	// the downloads count. Default to true for compatibility with old
	// clients that won't tell us what they are trying to do.
	if(req.cfg["increase_downloads"].to_bool(true) && !ignore_address_stats(req.addr)) {
		addon["downloads"] = 1 + addon["downloads"].to_int();
		mark_dirty(name);
		if(user_handler_) {
			user_handler_->db_update_addon_download_count(server_id_, name, to);
		}
	}
}

void server::handle_request_campaign_hash(const server::request& req)
{
	auto addon = get_addon(req.cfg["name"]);

	if(!addon || addon["hidden"].to_bool()) {
		send_error("Add-on '" + req.cfg["name"].str() + "' not found.", req.sock);
		return;
	}

	std::string path = addon["filename"].str() + '/';

	auto version_map = get_version_map(*addon);

	if(version_map.empty()) {
		send_error("No versions of the add-on '" + req.cfg["name"].str() + "' are available on the server.", req.sock);
		return;
	} else {
		const auto& version_str = addon["version"].str();
		version_info version_parsed{version_str};
		auto version = version_map.find(version_parsed);
		if(version != version_map.end()) {
			path += version->second["filename"].str();
		} else {
			// Selecting the latest version before the selected version or the overall latest version if unspecified
			if(version_str.empty()) {
				path += version_map.rbegin()->second["filename"].str();
			} else {
				path += (--version_map.upper_bound(version_parsed))->second["filename"].str();
			}
		}

		path = index_from_full_pack_filename(path);
		const int file_size = filesystem::file_size(path);

		if(file_size < 0) {
			send_error("Missing index file for the add-on '" + req.cfg["name"].str() + "'.", req.sock);
			return;
		}

		LOG_CS << req << "Sending add-on hash index for '" << req.cfg["name"] << "' size: " << file_size / 1024 << " KiB";
		utils::visit([this, &path, &req](auto&& socket) {
			coro_send_file(socket, path, req.yield);
		}, req.sock);
	}
}

void server::handle_request_terms(const server::request& req)
{
	// This usually means the client wants to upload content, so tell it
	// to give up when we're in read-only mode.
	if(read_only_) {
		LOG_CS << "in read-only mode, request for upload terms denied";
		send_error("The server is currently in read-only mode, add-on uploads are disabled.", req.sock);
		return;
	}

	LOG_CS << req << "Sending license terms";
	send_message(license_notice_, req.sock);
}

ADDON_CHECK_STATUS server::validate_addon(const server::request& req, config*& existing_addon, std::string& error_data)
{
	if(read_only_) {
		LOG_CS << "Validation error: uploads not permitted in read-only mode.";
		return ADDON_CHECK_STATUS::SERVER_READ_ONLY;
	}

	const config& upload = req.cfg;

	const auto data = upload.optional_child("data");
	const auto removelist = upload.optional_child("removelist");
	const auto addlist = upload.optional_child("addlist");

	const bool is_upload_pack = have_wml(removelist) || have_wml(addlist);

	const std::string& name = upload["name"].str();

	existing_addon = nullptr;
	error_data.clear();

	bool passed_name_utf8_check = false;

	try {
		const std::string& lc_name = utf8::lowercase(name);
		passed_name_utf8_check = true;

		for(auto& c : addons_) {
			if(utf8::lowercase(c.first) == lc_name) {
				existing_addon = &c.second;
				break;
			}
		}
	} catch(const utf8::invalid_utf8_exception&) {
		if(!passed_name_utf8_check) {
			LOG_CS << "Validation error: bad UTF-8 in add-on name";
			return ADDON_CHECK_STATUS::INVALID_UTF8_NAME;
		} else {
			ERR_CS << "Validation error: add-ons list has bad UTF-8 somehow, this is a server side issue, it's bad, and you should probably fix it ASAP";
			return ADDON_CHECK_STATUS::SERVER_ADDONS_LIST;
		}
	}

	// Auth and block-list based checks go first

	if(upload["passphrase"].empty()) {
		LOG_CS << "Validation error: no passphrase specified";
		return ADDON_CHECK_STATUS::NO_PASSPHRASE;
	}

	if(existing_addon && upload["forum_auth"].to_bool() != (*existing_addon)["forum_auth"].to_bool()) {
		LOG_CS << "Validation error: forum_auth is " << upload["forum_auth"].to_bool() << " but was previously uploaded set to " << (*existing_addon)["forum_auth"].to_bool();
		return ADDON_CHECK_STATUS::AUTH_TYPE_MISMATCH;
	} else if(upload["forum_auth"].to_bool()) {
		if(!user_handler_) {
			LOG_CS << "Validation error: client requested forum authentication but server does not support it";
			return ADDON_CHECK_STATUS::SERVER_FORUM_AUTH_DISABLED;
		} else {
			if(!user_handler_->user_exists(upload["uploader"].str())) {
				LOG_CS << "Validation error: forum auth requested for an author who doesn't exist";
				return ADDON_CHECK_STATUS::USER_DOES_NOT_EXIST;
			}

			for(const std::string& primary_author : utils::split(upload["primary_authors"].str(), ',')) {
				if(!user_handler_->user_exists(primary_author)) {
					LOG_CS << "Validation error: forum auth requested for a primary author who doesn't exist";
					return ADDON_CHECK_STATUS::USER_DOES_NOT_EXIST;
				}
			}

			for(const std::string& secondary_author : utils::split(upload["secondary_authors"].str(), ',')) {
				if(!user_handler_->user_exists(secondary_author)) {
					LOG_CS << "Validation error: forum auth requested for a secondary author who doesn't exist";
					return ADDON_CHECK_STATUS::USER_DOES_NOT_EXIST;
				}
			}

			if(!authenticate_forum(upload, upload["passphrase"].str(), false)) {
				LOG_CS << "Validation error: forum passphrase does not match";
				return ADDON_CHECK_STATUS::UNAUTHORIZED;
			}
		}
	} else if(existing_addon && !authenticate(*existing_addon, upload["passphrase"])) {
		LOG_CS << "Validation error: campaignd passphrase does not match";
		return ADDON_CHECK_STATUS::UNAUTHORIZED;
	}

	if(existing_addon && (*existing_addon)["hidden"].to_bool()) {
		LOG_CS << "Validation error: add-on is hidden";
		return ADDON_CHECK_STATUS::DENIED;
	}

	try {
		if(blacklist_.is_blacklisted(name,
									 upload["title"].str(),
									 upload["description"].str(),
									 upload["author"].str(),
									 req.addr,
									 upload["email"].str()))
		{
			LOG_CS << "Validation error: blacklisted uploader or publish information";
			return ADDON_CHECK_STATUS::DENIED;
		}
	} catch(const utf8::invalid_utf8_exception&) {
		LOG_CS << "Validation error: invalid UTF-8 sequence in publish information while checking against the blacklist";
		return ADDON_CHECK_STATUS::INVALID_UTF8_ATTRIBUTE;
	}

	// Structure and syntax checks follow

	if(!is_upload_pack && !have_wml(data)) {
		LOG_CS << "Validation error: no add-on data.";
		return ADDON_CHECK_STATUS::EMPTY_PACK;
	}

	if(is_upload_pack && !have_wml(removelist) && !have_wml(addlist)) {
		LOG_CS << "Validation error: no add-on data.";
		return ADDON_CHECK_STATUS::EMPTY_PACK;
	}

	if(!addon_name_legal(name)) {
		LOG_CS << "Validation error: invalid add-on name.";
		return ADDON_CHECK_STATUS::BAD_NAME;
	}

	if(is_text_markup_char(name[0])) {
		LOG_CS << "Validation error: add-on name starts with an illegal formatting character.";
		return ADDON_CHECK_STATUS::NAME_HAS_MARKUP;
	}

	if(upload["title"].empty()) {
		LOG_CS << "Validation error: no add-on title specified";
		return ADDON_CHECK_STATUS::NO_TITLE;
	}

	if(addon_icon_too_large(upload["icon"].str())) {
		LOG_CS << "Validation error: icon too large";
		return ADDON_CHECK_STATUS::ICON_TOO_LARGE;
	}

	if(is_text_markup_char(upload["title"].str()[0])) {
		LOG_CS << "Validation error: add-on title starts with an illegal formatting character.";
		return ADDON_CHECK_STATUS::TITLE_HAS_MARKUP;
	}

	if(get_addon_type(upload["type"]) == ADDON_UNKNOWN) {
		LOG_CS << "Validation error: unknown add-on type specified";
		return ADDON_CHECK_STATUS::BAD_TYPE;
	}

	if(upload["author"].empty()) {
		LOG_CS << "Validation error: no add-on author specified";
		return ADDON_CHECK_STATUS::NO_AUTHOR;
	}

	if(upload["version"].empty()) {
		LOG_CS << "Validation error: no add-on version specified";
		return ADDON_CHECK_STATUS::NO_VERSION;
	}

	if(existing_addon) {
		version_info new_version{upload["version"].str()};
		version_info old_version{(*existing_addon)["version"].str()};

		if(strict_versions_ ? new_version <= old_version : new_version < old_version) {
			LOG_CS << "Validation error: add-on version not incremented";
			return ADDON_CHECK_STATUS::VERSION_NOT_INCREMENTED;
		}
	}

	if(upload["description"].empty()) {
		LOG_CS << "Validation error: no add-on description specified";
		return ADDON_CHECK_STATUS::NO_DESCRIPTION;
	}

	// if using forum_auth, email will be pulled from the forum database later
	if(upload["email"].empty() && !upload["forum_auth"].to_bool()) {
		LOG_CS << "Validation error: no add-on email specified";
		return ADDON_CHECK_STATUS::NO_EMAIL;
	}

	if(const auto badnames = multi_find_illegal_names(data, addlist, removelist)) {
		error_data = utils::join(*badnames, "\n");
		LOG_CS << "Validation error: invalid filenames in add-on pack (" << badnames->size() << " entries)";
		return ADDON_CHECK_STATUS::ILLEGAL_FILENAME;
	}

	if(const auto badnames = multi_find_case_conflicts(data, addlist, removelist)) {
		error_data = utils::join(*badnames, "\n");
		LOG_CS << "Validation error: case conflicts in add-on pack (" << badnames->size() << " entries)";
		return ADDON_CHECK_STATUS::FILENAME_CASE_CONFLICT;
	}

	if(is_upload_pack && !existing_addon) {
		LOG_CS << "Validation error: attempted to send an update pack for a non-existent add-on";
		return ADDON_CHECK_STATUS::UNEXPECTED_DELTA;
	}

	if(auto url_params = upload.optional_child("feedback")) {
		try {
			int topic_id = std::stoi(url_params["topic_id"].str("0"));
			if(user_handler_ && topic_id != 0) {
				if(!user_handler_->db_topic_id_exists(topic_id)) {
					LOG_CS << "Validation error: feedback topic ID does not exist in forum database";
					return ADDON_CHECK_STATUS::FEEDBACK_TOPIC_ID_NOT_FOUND;
				}
			}
		} catch(...) {
			LOG_CS << "Validation error: feedback topic ID is not a valid number";
			return ADDON_CHECK_STATUS::BAD_FEEDBACK_TOPIC_ID;
		}
	}

	return ADDON_CHECK_STATUS::SUCCESS;
}

void server::handle_upload(const server::request& req)
{
	const auto upload_ts = std::chrono::system_clock::now();
	const config& upload = req.cfg;
	const auto& name = upload["name"].str();

	LOG_CS << req << "Validating add-on '" << name << "'...";

	config* addon_ptr = nullptr;
	std::string val_error_data;
	const auto val_status = validate_addon(req, addon_ptr, val_error_data);

	if(val_status != ADDON_CHECK_STATUS::SUCCESS) {
		LOG_CS << "Upload of '" << name << "' aborted due to a failed validation check";
		const auto msg = std::string("Add-on rejected: ") + addon_check_status_desc(val_status);
		send_error(msg, val_error_data, static_cast<unsigned int>(val_status), req.sock);
		return;
	}

	LOG_CS << req << "Processing add-on '" << name << "'...";

	const auto full_pack    = upload.optional_child("data");
	const auto delta_remove = upload.optional_child("removelist");
	const auto delta_add    = upload.optional_child("addlist");

	const bool is_delta_upload = have_wml(delta_remove) || have_wml(delta_add);
	const bool is_existing_upload = addon_ptr != nullptr;

	if(!is_existing_upload) {
		// Create a new add-ons list entry and work with that from now on
		auto entry = addons_.emplace(name, config("original_timestamp", chrono::serialize_timestamp(upload_ts)));
		addon_ptr = &(*entry.first).second;
	}

	config& addon = *addon_ptr;

	LOG_CS << req << "Upload type: "
		   << (is_delta_upload ? "delta" : "full") << ", "
		   << (is_existing_upload ? "update" : "new");

	// Write general metadata attributes

	addon.copy_or_remove_attributes(upload,
		"title", "name", "uploader", "author", "primary_authors", "secondary_authors", "description", "version", "icon",
		"translate", "dependencies", "core", "type", "tags", "email", "forum_auth"
	);

	const std::string& pathstem = "data/" + name;
	addon["filename"] = pathstem;
	addon["upload_ip"] = req.addr;

	if(!is_existing_upload && !addon["forum_auth"].to_bool()) {
		set_passphrase(addon, upload["passphrase"]);
	}

	if(addon["downloads"].empty()) {
		addon["downloads"] = 0;
	}

	addon["timestamp"] = chrono::serialize_timestamp(upload_ts);
	addon["uploads"] = 1 + addon["uploads"].to_int();

	addon.clear_children("feedback");
	int topic_id = 0;
	if(auto url_params = upload.optional_child("feedback")) {
		addon.add_child("feedback", *url_params);
		// already validated that this can be converted to an int in validate_addon()
		topic_id = url_params["topic_id"].to_int();
	}

	if(user_handler_) {
		if(addon["forum_auth"].to_bool()) {
			addon["email"] = user_handler_->get_user_email(upload["uploader"].str());

			// if no author information exists, insert data since that of course means no primary author can be found
			// or if the author is the primary uploader, replace the author information
			bool do_authors_exist = user_handler_->db_do_any_authors_exist(server_id_, name);
			bool is_primary = user_handler_->db_is_user_primary_author(server_id_, name, upload["uploader"].str());
			if(!do_authors_exist || is_primary) {
				user_handler_->db_delete_addon_authors(server_id_, name);
				// author instead of uploader here is intentional, since this allows changing the primary author
				// if p1 is primary, p2 is secondary, and p1 uploads, then uploader and author are p1 while p2 is a secondary author
				// if p1 is primary, p2 is secondary, and p2 uploads, then this is skipped because the uploader is not the primary author
				// if next time p2 is primary, p1 is secondary, and p1 uploads, then p1 is both uploader and secondary author
				//   therefore p2's author information would not be reinserted if the uploader attribute were used instead
				user_handler_->db_insert_addon_authors(server_id_, name, utils::split(addon["primary_authors"].str(), ','), utils::split(addon["secondary_authors"].str(), ','));
			}
		}
		user_handler_->db_insert_addon_info(server_id_, name, addon["title"].str(), addon["type"].str(), addon["version"].str(), addon["forum_auth"].to_bool(), topic_id, upload["uploader"].str());
	}

	// Copy in any metadata translations provided directly in the .pbl.
	// Catalogue detection is done later -- in the meantime we just mark
	// translations with valid metadata as not supported until we find out
	// whether the add-on ships translation catalogues for them or not.

	addon.clear_children("translation");

	for(const config& locale_params : upload.child_range("translation")) {
		if(!locale_params["language"].empty()) {
			config& locale = addon.add_child("translation");
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

	// We need to alter the WML pack slightly, but we don't want to do a deep
	// copy of data that's larger than 5 MB in the average case (and as large
	// as 100 MB in the worst case). On the other hand, if the upload is a
	// delta then need to leave this empty and fill it in later instead.

	config rw_full_pack;
	if(have_wml(full_pack)) {
		// Void the warranty
		rw_full_pack = std::move(const_cast<config&>(*full_pack));
	}

	// Versioning support

	const auto& new_version = addon["version"].str();
	auto version_map = get_version_map(addon);

	if(is_delta_upload) {
		// Create the full pack by grabbing the one for the requested 'from'
		// version (or latest available) and applying the delta on it. We
		// proceed from there by fill in rw_full_pack with the result.

		if(version_map.empty()) {
			// This should NEVER happen
			ERR_CS << "Add-on '" << name << "' has an empty version table, this should not happen";
			send_error("Server error: Cannot process update pack with an empty version table.", "", static_cast<unsigned int>(ADDON_CHECK_STATUS::SERVER_DELTA_NO_VERSIONS), req.sock);
			return;
		}

		auto prev_version = upload["from"].str();

		if(prev_version.empty()) {
			prev_version = version_map.rbegin()->first;
		} else {
			// If the requested 'from' version doesn't exist, select the newest
			// older version available.
			version_info prev_version_parsed{prev_version};
			auto vm_entry = version_map.find(prev_version_parsed);
			if(vm_entry == version_map.end()) {
				prev_version = (--version_map.upper_bound(prev_version_parsed))->first;
			}
		}

		// Remove any existing update packs targeting the new version. This is
		// really only needed if the server allows multiple uploads of an
		// add-on with the same version number.

		std::set<std::string> delete_packs;
		for(const auto& pack : addon.child_range("update_pack")) {
			if(pack["to"].str() == new_version) {
				const auto& pack_filename = pack["filename"].str();
				filesystem::delete_file(pathstem + '/' + pack_filename);
				delete_packs.insert(pack_filename);
			}
		}

		if(!delete_packs.empty()) {
			addon.remove_children("update_pack", [&delete_packs](const config& p) {
				return delete_packs.find(p["filename"].str()) != delete_packs.end();
			});
		}

		const auto& update_pack_fn = make_update_pack_filename(prev_version, new_version);

		config& pack_info = addon.add_child("update_pack");

		pack_info["from"] = prev_version;
		pack_info["to"] = new_version;
		pack_info["expire"] = chrono::serialize_timestamp(upload_ts + update_pack_lifespan_);
		pack_info["filename"] = update_pack_fn;

		// Write the update pack to disk

		{
			LOG_CS << "Saving provided update pack for " << prev_version << " -> " << new_version << "...";

			filesystem::atomic_commit pack_file{pathstem + '/' + update_pack_fn};
			config_writer writer{*pack_file.ostream(), true, compress_level_};
			static const config empty_config;

			writer.open_child("removelist");
			writer.write(have_wml(delta_remove) ? *delta_remove : empty_config);
			writer.close_child("removelist");

			writer.open_child("addlist");
			writer.write(have_wml(delta_add) ? *delta_add : empty_config);
			writer.close_child("addlist");

			pack_file.commit();
		}

		// Apply it to the addon data from the previous version to generate a
		// new full pack, which will be written later near the end of this
		// request servicing routine.

		version_info prev_version_parsed{prev_version};
		auto it = version_map.find(prev_version_parsed);
		if(it == version_map.end()) {
			// This REALLY should never happen
			ERR_CS << "Previous version dropped off the version map?";
			send_error("Server error: Previous version disappeared.", "", static_cast<unsigned int>(ADDON_CHECK_STATUS::SERVER_UNSPECIFIED), req.sock);
			return;
		}

		auto in = filesystem::istream_file(pathstem + '/' + it->second["filename"].str());
		rw_full_pack.clear();
		read_gz(rw_full_pack, *in);

		if(have_wml(delta_remove)) {
			data_apply_removelist(rw_full_pack, *delta_remove);
		}

		if(have_wml(delta_add)) {
			data_apply_addlist(rw_full_pack, *delta_add);
		}
	}

	// Detect translation catalogues and toggle their supported status accordingly

	find_translations(rw_full_pack, addon);

	// Add default license information if needed

	add_license(rw_full_pack);

	// Update version map, first removing any identical existing versions

	version_info new_version_parsed{new_version};
	config version_cfg{"version", new_version};
	version_cfg["filename"] = make_full_pack_filename(new_version);

	version_map.erase(new_version_parsed);
	addon.remove_children("version", [&new_version](const config& old_cfg)
		{
			return old_cfg["version"].str() == new_version;
		}
	);

	version_map.emplace(new_version_parsed, version_cfg);
	addon.add_child("version", version_cfg);

	// Clean-up

	rw_full_pack["name"] = ""; // [dir] syntax expects this to be present and empty

	// Write the full pack and its index file

	const auto& full_pack_path = pathstem + '/' + version_cfg["filename"].str();
	const auto& index_path = pathstem + '/' + make_index_filename(new_version);

	{
		config pack_index{"name", ""}; // [dir] syntax expects this to be present and empty
		write_hashlist(pack_index, rw_full_pack);

		filesystem::atomic_commit addon_pack_file{full_pack_path};
		config_writer{*addon_pack_file.ostream(), true, compress_level_}.write(rw_full_pack);
		addon_pack_file.commit();

		filesystem::atomic_commit addon_index_file{index_path};
		config_writer{*addon_index_file.ostream(), true, compress_level_}.write(pack_index);
		addon_index_file.commit();
	}

	addon["size"] = filesystem::file_size(full_pack_path);

	// Expire old update packs and delete them

	std::set<std::string> expire_packs;

	for(const config& pack : addon.child_range("update_pack")) {
		if(upload_ts > chrono::parse_timestamp(pack["expire"]) || pack["from"].str() == new_version || (!is_delta_upload && pack["to"].str() == new_version)) {
			LOG_CS << "Expiring upate pack for " << pack["from"].str() << " -> " << pack["to"].str();
			const auto& pack_filename = pack["filename"].str();
			filesystem::delete_file(pathstem + '/' + pack_filename);
			expire_packs.insert(pack_filename);
		}
	}

	if(!expire_packs.empty()) {
		addon.remove_children("update_pack", [&expire_packs](const config& p) {
			return expire_packs.find(p["filename"].str()) != expire_packs.end();
		});
	}

	// Create any missing update packs between consecutive versions. This covers
	// cases where clients were not able to upload those update packs themselves.

	for(auto iter = version_map.begin(); std::distance(iter, version_map.end()) > 1;) {
		const config& prev_version = iter->second;
		const config& next_version = (++iter)->second;

		const auto& prev_version_name = prev_version["version"].str();
		const auto& next_version_name = next_version["version"].str();

		bool found = false;

		for(const auto& pack : addon.child_range("update_pack")) {
			if(pack["from"].str() == prev_version_name && pack["to"].str() == next_version_name) {
				found = true;
				break;
			}
		}

		if(found) {
			// Nothing to do
			continue;
		}

		LOG_CS << "Automatically generating update pack for " << prev_version_name << " -> " << next_version_name << "...";

		const auto& prev_path = pathstem + '/' + prev_version["filename"].str();
		const auto& next_path = pathstem + '/' + next_version["filename"].str();

		if(filesystem::file_size(prev_path) <= 0 || filesystem::file_size(next_path) <= 0) {
			ERR_CS << "Unable to automatically generate an update pack for '" << name
					<< "' for version " << prev_version_name << " to " << next_version_name
					<< "!";
			continue;
		}

		const auto& update_pack_fn = make_update_pack_filename(prev_version_name, next_version_name);

		config& pack_info = addon.add_child("update_pack");
		pack_info["from"] = prev_version_name;
		pack_info["to"] = next_version_name;
		pack_info["expire"] = chrono::serialize_timestamp(upload_ts + update_pack_lifespan_);
		pack_info["filename"] = update_pack_fn;

		// Generate the update pack from both full packs

		config pack, from, to;

		filesystem::scoped_istream in = filesystem::istream_file(prev_path);
		read_gz(from, *in);
		in = filesystem::istream_file(next_path);
		read_gz(to, *in);

		make_updatepack(pack, from, to);

		{
			filesystem::atomic_commit pack_file{pathstem + '/' + update_pack_fn};
			config_writer{*pack_file.ostream(), true, compress_level_}.write(pack);
			pack_file.commit();
		}
	}

	mark_dirty(name);
	write_config();

	LOG_CS << req << "Finished uploading add-on '" << upload["name"] << "'";

	send_message("Add-on accepted.", req.sock);

	fire("hook_post_upload", name);
}

void server::handle_delete(const server::request& req)
{
	const config& erase = req.cfg;
	const std::string& id = erase["name"].str();

	if(read_only_) {
		LOG_CS << req << "in read-only mode, request to delete '" << id << "' denied";
		send_error("Cannot delete add-on: The server is currently in read-only mode.", req.sock);
		return;
	}

	LOG_CS << req << "Deleting add-on '" << id << "'";

	auto addon = get_addon(id);
	if(!addon) {
		send_error("The add-on does not exist.", req.sock);
		return;
	}

	const config::attribute_value& pass = erase["passphrase"];

	if(pass.empty()) {
		send_error("No passphrase was specified.", req.sock);
		return;
	}

	if(erase["admin"].to_bool()) {
		if(!authenticate_admin(erase["uploader"].str(), pass.str())) {
			send_error("The passphrase is incorrect.", req.sock);
			return;
		}
	} else {
		if(addon["forum_auth"].to_bool()) {
			if(!authenticate_forum(erase, pass, true)) {
				send_error("The passphrase is incorrect.", req.sock);
				return;
			}
		} else {
			if(!authenticate(*addon, pass)) {
				send_error("The passphrase is incorrect.", req.sock);
				return;
			}
		}
	}

	if(addon["hidden"].to_bool()) {
		LOG_CS << "Add-on removal denied - hidden add-on.";
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
		LOG_CS << "in read-only mode, request to change passphrase denied";
		send_error("Cannot change passphrase: The server is currently in read-only mode.", req.sock);
		return;
	}

	auto addon = get_addon(cpass["name"]);

	if(!addon) {
		send_error("No add-on with that name exists.", req.sock);
	} else if(addon["forum_auth"].to_bool()) {
		send_error("Changing the password for add-ons using forum_auth is not supported.", req.sock);
	} else if(!authenticate(*addon, cpass["passphrase"])) {
		send_error("Your old passphrase was incorrect.", req.sock);
	} else if(addon["hidden"].to_bool()) {
		LOG_CS << "Passphrase change denied - hidden add-on.";
		send_error("Add-on passphrase change denied. Please contact the server administration for assistance.", req.sock);
	} else if(cpass["new_passphrase"].empty()) {
		send_error("No new passphrase was supplied.", req.sock);
	} else {
		set_passphrase(*addon, cpass["new_passphrase"]);
		dirty_addons_.emplace(addon["name"]);
		write_config();
		send_message("Passphrase changed.", req.sock);
	}
}

// the fifo handler also hides add-ons
void server::handle_hide_addon(const server::request& req)
{
	std::string addon_id = req.cfg["addon"].str();
	std::string username = req.cfg["username"].str();
	std::string passphrase = req.cfg["passphrase"].str();

	auto addon = get_addon(addon_id);

	if(!addon) {
		ERR_CS << "Add-on '" << addon_id << "' not found, cannot hide";
		send_error("The add-on was not found.", req.sock);
		return;
	} else {
		if(!authenticate_admin(username, passphrase)) {
			send_error("The passphrase is incorrect.", req.sock);
			return;
		}

		addon["hidden"] = true;
		mark_dirty(addon_id);
		write_config();
		LOG_CS << "Add-on '" << addon_id << "' is now hidden";
	}

	send_message("Add-on hidden.", req.sock);
}

// the fifo handler also unhides add-ons
void server::handle_unhide_addon(const server::request& req)
{
	std::string addon_id = req.cfg["addon"].str();
	std::string username = req.cfg["username"].str();
	std::string passphrase = req.cfg["passphrase"].str();

	auto addon = get_addon(addon_id);

	if(!addon) {
		ERR_CS << "Add-on '" << addon_id << "' not found, cannot unhide";
		send_error("The add-on was not found.", req.sock);
		return;
	} else {
		if(!authenticate_admin(username, passphrase)) {
			send_error("The passphrase is incorrect.", req.sock);
			return;
		}

		addon["hidden"] = false;
		mark_dirty(addon_id);
		write_config();
		LOG_CS << "Add-on '" << addon_id << "' is now unhidden";
	}

	send_message("Add-on unhidden.", req.sock);
}

void server::handle_list_hidden(const server::request& req)
{
	config response;
	std::string username = req.cfg["username"].str();
	std::string passphrase = req.cfg["passphrase"].str();

	if(!authenticate_admin(username, passphrase)) {
		send_error("The passphrase is incorrect.", req.sock);
		return;
	}

	for(const auto& addon : addons_) {
		if(addon.second["hidden"].to_bool()) {
			config& child = response.add_child("hidden");
			child["addon"] = addon.second["name"].str();
		}
	}

	std::ostringstream ostr;
	write(ostr, response);

	const auto& wml = ostr.str();
	simple_wml::document doc(wml.c_str(), simple_wml::INIT_STATIC);
	doc.compress();

	utils::visit([this, &doc](auto&& sock) { async_send_doc_queued(sock, doc); }, req.sock);
}

void server::handle_addon_downloads_by_version(const server::request& req)
{
	config response;

	if(user_handler_) {
		response = user_handler_->db_get_addon_downloads_info(server_id_, req.cfg["addon"].str());
	}

	std::ostringstream ostr;
	write(ostr, response);

	const auto& wml = ostr.str();
	simple_wml::document doc(wml.c_str(), simple_wml::INIT_STATIC);
	doc.compress();

	utils::visit([this, &doc](auto&& sock) { async_send_doc_queued(sock, doc); }, req.sock);
}

void server::handle_forum_auth_usage(const server::request& req)
{
	config response;

	if(user_handler_) {
		response = user_handler_->db_get_forum_auth_usage(server_id_);
	}

	std::ostringstream ostr;
	write(ostr, response);

	const auto& wml = ostr.str();
	simple_wml::document doc(wml.c_str(), simple_wml::INIT_STATIC);
	doc.compress();

	utils::visit([this, &doc](auto&& sock) { async_send_doc_queued(sock, doc); }, req.sock);
}

void server::handle_admins_list(const server::request& req)
{
	config response;

	if(user_handler_) {
		response = user_handler_->db_get_addon_admins();
	}

	std::ostringstream ostr;
	write(ostr, response);

	const auto& wml = ostr.str();
	simple_wml::document doc(wml.c_str(), simple_wml::INIT_STATIC);
	doc.compress();

	utils::visit([this, &doc](auto&& sock) { async_send_doc_queued(sock, doc); }, req.sock);
}

bool server::authenticate_forum(const config& addon, const std::string& passphrase, bool is_delete) {
	if(!user_handler_) {
		return false;
	}

	std::string uploader = addon["uploader"].str();
	std::string id = addon["name"].str();
	bool do_authors_exist = user_handler_->db_do_any_authors_exist(server_id_, id);
	bool is_primary = user_handler_->db_is_user_primary_author(server_id_, id, uploader);
	bool is_secondary = user_handler_->db_is_user_secondary_author(server_id_, id, uploader);

	// allow if there is no author information - this is a new upload
	// don't allow other people to upload if author information does exist
	// don't allow secondary authors to remove the add-on from the server
	if((do_authors_exist && !is_primary && !is_secondary) || (is_secondary && is_delete)) {
		return false;
	}

	std::string author = addon["uploader"].str();
	std::string salt = user_handler_->extract_salt(author);
	std::string hashed_password = hash_password(passphrase, salt, author);

	return user_handler_->login(author, hashed_password);
}

bool server::authenticate_admin(const std::string& username, const std::string& passphrase)
{
	if(!user_handler_ || !user_handler_->user_is_addon_admin(username)) {
		return false;
	}

	std::string salt = user_handler_->extract_salt(username);
	std::string hashed_password = hash_password(passphrase, salt, username);

	return user_handler_->login(username, hashed_password);
}

} // end namespace campaignd

static int run_campaignd(int argc, char** argv)
{
	campaignd::command_line cmdline{argc, argv};
	std::string server_path = filesystem::get_cwd();
	std::string config_file = "server.cfg";
	unsigned short port = 0;

	//
	// Log defaults
	//

	for(auto domain : { "campaignd", "campaignd/blacklist", "server" }) {
		lg::set_log_domain_severity(domain, lg::info());
	}

	lg::timestamps(true);

	//
	// Process command line
	//

	if(cmdline.help) {
		std::cout << cmdline.help_text();
		return 0;
	}

	if(cmdline.version) {
		std::cout << "Wesnoth campaignd v" << game_config::revision << '\n';
		return 0;
	}

	if(cmdline.config_file) {
		// Don't fully resolve the path, so that filesystem::ostream_file() can
		// create path components as needed (dumb legacy behavior).
		config_file = filesystem::normalize_path(*cmdline.config_file, true, false);
	}

	if(cmdline.server_dir) {
		server_path = filesystem::normalize_path(*cmdline.server_dir, true, true);
	}

	if(cmdline.port) {
		port = *cmdline.port;
		// We use 0 as a placeholder for the default port for this version
		// otherwise, hence this check must only exists in this code path. It's
		// only meant to protect against user mistakes.
		if(!port) {
			PLAIN_LOG << "Invalid network port: " << port;
			return 2;
		}
	}

	if(cmdline.show_log_domains) {
		std::cout << lg::list_log_domains("");
		return 0;
	}

	for(const auto& ldl : cmdline.log_domain_levels) {
		if(!lg::set_log_domain_severity(ldl.first, static_cast<lg::severity>(ldl.second))) {
			PLAIN_LOG << "Unknown log domain: " << ldl.first;
			return 2;
		}
	}

	if(cmdline.log_precise_timestamps) {
		lg::precise_timestamps(true);
	}

	if(cmdline.report_timings) {
		campaignd::timing_reports_enabled = true;
	}

	PLAIN_LOG << "Wesnoth campaignd v" << game_config::revision << " starting...";

	if(server_path.empty() || !filesystem::is_directory(server_path)) {
		PLAIN_LOG << "Server directory '" << *cmdline.server_dir << "' does not exist or is not a directory.";
		return 1;
	}

	if(filesystem::is_directory(config_file)) {
		PLAIN_LOG << "Server configuration file '" << config_file << "' is not a file.";
		return 1;
	}

	// Everything does file I/O with pwd as the implicit starting point, so we
	// need to change it accordingly. We don't do this before because paths in
	// the command line need to remain relative to the original pwd.
	if(cmdline.server_dir && !filesystem::set_cwd(server_path)) {
		PLAIN_LOG << "Bad server directory '" << server_path << "'.";
		return 1;
	}

	game_config::path = server_path;

	//
	// Run the server
	//
	return campaignd::server(config_file, port).run();
}

int main(int argc, char** argv)
{
	try {
		run_campaignd(argc, argv);
	} catch(const boost::program_options::error& e) {
		PLAIN_LOG << "Error in command line: " << e.what();
		return 10;
	} catch(const config::error& e) {
		PLAIN_LOG << "Could not parse config file: " << e.message;
		return 1;
	} catch(const filesystem::io_exception& e) {
		PLAIN_LOG << "File I/O error: " << e.what();
		return 2;
	} catch(const std::bad_function_call& /*e*/) {
		PLAIN_LOG << "Bad request handler function call";
		return 4;
	}

	return 0;
}
