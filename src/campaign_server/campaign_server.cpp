/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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

#include "filesystem.hpp"
#include "log.hpp"
#include "network_worker.hpp"
#include "serialization/binary_or_text.hpp"
#include "serialization/parser.hpp"
#include "serialization/string_utils.hpp"
#include "serialization/unicode.hpp"
#include "game_config.hpp"
#include "addon/validation.hpp"
#include "version.hpp"
#include "server/input_stream.hpp"
#include "util.hpp"

#include <csignal>

#include <boost/foreach.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/exception/get_error_info.hpp>

// the fork execute is unix specific only tested on Linux quite sure it won't
// work on Windows not sure which other platforms have a problem with it.
#if !(defined(_WIN32))
#include <errno.h>
#endif

static lg::log_domain log_network("network");
#define LOG_CS if (lg::err.dont_log(log_network)) ; else lg::err(log_network, false)

//compatibility code for MS compilers
#ifndef SIGHUP
#define SIGHUP 20
#endif
/** @todo FIXME: should define SIGINT here too, but to what? */

static void exit_sighup(int signal) {
	assert(signal == SIGHUP);
	LOG_CS << "SIGHUP caught, exiting without cleanup immediately.\n";
	exit(128 + SIGHUP);
}

static void exit_sigint(int signal) {
	assert(signal == SIGINT);
	LOG_CS << "SIGINT caught, exiting without cleanup immediately.\n";
	exit(0);
}

static void exit_sigterm(int signal) {
	assert(signal == SIGTERM);
	LOG_CS << "SIGTERM caught, exiting without cleanup immediately.\n";
	exit(128 + SIGTERM);
}

namespace {
	// Markup characters recognized by GUI1 code. These must be
	// the same as the constants defined in marked-up_text.cpp.
	const std::string illegal_markup_chars = "*`~{^}|@#<&";

	inline bool is_text_markup_char(char c)
	{
		return illegal_markup_chars.find(c) != std::string::npos;
	}

	typedef std::map<std::string, std::string> plain_string_map;

	/**
	 * Quick and dirty alternative to @a uitls::interpolate_variables_into_string that
	 * doesn't require formula AI code. It is definitely NOT safe for normal
	 * use since it doesn't do strict checks on where variable placeholders
	 * ("$foobar") end and doesn't support pipe ("|") terminators.
	 *
	 * @param str     The format string.
	 * @param symbols The symbols table.
	 */
	std::string fast_interpolate_variables_into_string(const std::string &str, const plain_string_map * const symbols)
	{
		std::string res = str;

		if(symbols) {
			BOOST_FOREACH(const plain_string_map::value_type& sym, *symbols) {
				res = utils::replace(res, "$" + sym.first, sym.second);
			}
		}

		return res;
	}

	/**
	 * Format a feedback URL for an add-on.
	 *
	 * @param format        The format string for the URL, presumably obtained
	 *                      from the add-ons server identification.
	 *
	 * @param params        The URL format parameters table.
	 *
	 * @return A string containing a feedback URL or an empty string if that
	 *         is not possible (e.g. empty or invalid @a format, empty
	 *         @a params table, or a result that is identical in content to
	 *         the @a format suggesting that the @a params table contains
	 *         incorrect data).
	 */
	std::string format_addon_feedback_url(const std::string& format, const config& params)
	{
		if(!format.empty() && !params.empty()) {
#if 0
			utils::string_map escaped;
#else
			plain_string_map escaped;
#endif

			config::const_attr_itors attrs = params.attribute_range();

			// Percent-encode parameter values for URL interpolation. This is
			// VERY important since otherwise people could e.g. alter query
			// strings from the format string.
			BOOST_FOREACH(const config::attribute& a, attrs) {
				escaped[a.first] = utils::urlencode(a.second.str());
			}

			// FIXME: We cannot use utils::interpolate_variables_into_string
			//        because it is implemented using a lot of formula AI junk
			//        that really doesn't belong in campaignd.
#if 0
			const std::string& res =
				utils::interpolate_variables_into_string(format, &escaped);
#else
			const std::string& res =
				fast_interpolate_variables_into_string(format, &escaped);
#endif

			if(res != format) {
				return res;
			}

			// If we get here, that means that no interpolation took place; in
			// that case, the parameters table probably contains entries that
			// do not match the format string expectations.
		}

		return std::string();
	}

	config construct_message(const std::string& msg)
	{
		config cfg;
		cfg.add_child("message")["message"] = msg;
		return cfg;
	}

	config construct_error(const std::string& msg)
	{
		config cfg;
		cfg.add_child("error")["message"] = msg;
		LOG_CS << "ERROR: "<<msg<<"\n";
		return cfg;
	}

	class campaign_server
	{
		public:
			explicit campaign_server(const std::string& cfgfile,size_t min_thread = 10,size_t max_thread = 0);
			void run();
			~campaign_server()
			{
				delete input_;
				scoped_ostream cfgfile = ostream_file(file_);
				write(*cfgfile, cfg_);
			}
		private:
			/**
			 * Fires a script, if no script defined it will always return true
			 * If a script is defined but can't be executed it will return false
			 */
			void fire(const std::string& hook, const std::string& addon);
			int load_config(); // return the server port
			const config &campaigns() const { return cfg_.child("campaigns"); }
			config &campaigns() { return cfg_.child("campaigns"); }

			const config& server_info() const { return cfg_.child("server_info"); }
			config& server_info() { return cfg_.child("server_info"); }

			config cfg_;
			const std::string file_;
			const network::manager net_manager_;
			std::map<std::string, std::string> hooks_;
			input_stream* input_;
			int compress_level_;
			bool read_only_;

			/** Feedback URL format string used for add-ons. */
			std::string feedback_url_format_;

			const network::server_manager server_manager_;

	};

	void campaign_server::fire(const std::string& hook, const std::string& addon)
	{
		const std::map<std::string, std::string>::const_iterator itor = hooks_.find(hook);
		if(itor == hooks_.end()) return;

		const std::string& script = itor->second;
		if(script == "") return;

#if (defined(_WIN32))
		LOG_CS << "ERROR: Tried to execute a script on an unsupported platform\n";
		return;
#else
		pid_t childpid;

		if((childpid = fork()) == -1) {
			LOG_CS << "ERROR: fork failed while updating campaign " << addon << "\n";
			return;
		}

		if(childpid == 0) {
			/*** We're the child process ***/

			// execute the script, we run is a separate thread and share the
			// output which will make the logging look ugly.
			execlp(script.c_str(), script.c_str(), addon.c_str(), static_cast<char *>(NULL));

			// exec() and family never return; if they do, we have a problem
			std::cerr << "ERROR: exec failed with errno " << errno << " for addon " << addon
			          << '\n';
			exit(errno);

		} else {
			return;
		}

#endif
	}

	int campaign_server::load_config()
	{
		scoped_istream stream = istream_file(file_);
		read(cfg_, *stream);
		bool network_use_system_sendfile = cfg_["network_use_system_sendfile"].to_bool();
		network_worker_pool::set_use_system_sendfile(network_use_system_sendfile);
		cfg_["network_use_system_sendfile"] = network_use_system_sendfile;
		/** Seems like compression level above 6 is waste of cpu cycle */
		compress_level_ = cfg_["compress_level"].to_int(6);
		cfg_["compress_level"] = compress_level_;

		const config& svinfo_cfg = server_info();
		if(svinfo_cfg) {
			feedback_url_format_ = svinfo_cfg["feedback_url_format"].str();
		}

		return cfg_["port"].to_int(default_campaignd_port);
	}

	campaign_server::campaign_server(const std::string& cfgfile,
			size_t min_thread, size_t max_thread) :
		cfg_(),
		file_(cfgfile),
		net_manager_(min_thread,max_thread),
		hooks_(),
		input_(0),
		compress_level_(0), // Will be properly set by load_config()
		read_only_(false),
		feedback_url_format_(), // Will be properly set by load_config()
		server_manager_(load_config())
	{
#ifndef _MSC_VER
		signal(SIGHUP, exit_sighup);
#endif
		signal(SIGINT, exit_sigint);
		signal(SIGTERM, exit_sigterm);

		cfg_.child_or_add("campaigns");

		// load the hooks
		hooks_.insert(std::make_pair(std::string("hook_post_upload"), cfg_["hook_post_upload"]));
		hooks_.insert(std::make_pair(std::string("hook_post_erase"), cfg_["hook_post_erase"]));
	}

	void find_translations(const config& cfg, config& campaign)
	{
		BOOST_FOREACH(const config &dir, cfg.child_range("dir"))
		{
			if (dir["name"] == "LC_MESSAGES") {
				config &language = campaign.add_child("translation");
				language["language"] = cfg["name"];
			} else {
				find_translations(dir, campaign);
			}
		}
	}

	// Add a file COPYING.txt with the GPL to an uploaded campaign.
	void add_license(config &data)
	{
		config &dir = data.find_child("dir", "name", data["campaign_name"]);
		// No top-level directory? Hm..
		if (!dir) return;

		// Don't add if it already exists.
		if (dir.find_child("file", "name", "COPYING.txt")) return;
		if (dir.find_child("file", "name", "COPYING")) return;
		if (dir.find_child("file", "name", "copying.txt")) return;
		if (dir.find_child("file", "name", "Copying.txt")) return;
		if (dir.find_child("file", "name", "COPYING.TXT")) return;

		// Copy over COPYING.txt
		std::string contents = read_file("COPYING.txt");
		if (contents.empty()) {
			LOG_CS << "Could not find COPYING.txt, path is \""
				<< game_config::path << "\"\n";
			return;
		}
		config &copying = dir.add_child("file");
		copying["name"] = "COPYING.txt";
		copying["contents"] = contents;

	}

	void campaign_server::run()
	{
 		if (!cfg_["control_socket"].empty())
 			input_ = new input_stream(cfg_["control_socket"]);

		read_only_ = cfg_["read_only"].to_bool(false);

		if(read_only_) {
			LOG_CS << "READ-ONLY MODE ACTIVE\n";
		}

		network::connection sock = 0;
		for(int increment = 0; ; ++increment) {
			try {
 				std::string admin_cmd;
 				if (input_ && input_->read_line(admin_cmd))
 				{
 					// process command
 					if (admin_cmd == "shut_down")
 					{
 						break;
 					}
 				}
			//write config to disk every ten minutes
				if((increment%(60*10*50)) == 0) {
					scoped_ostream cfgfile = ostream_file(file_);
					write(*cfgfile, cfg_);
				}

				network::process_send_queue();

				sock = network::accept_connection();
				if(sock) {
					LOG_CS << "received connection from " << network::ip_address(sock) << "\n";
				}

				config data;
				while((sock = network::receive_data(data, 0)) != network::null_connection) {
					if (const config &req = data.child("request_campaign_list"))
					{
						LOG_CS << "sending campaign list to " << network::ip_address(sock) << " using gzip";
						time_t epoch = time(NULL);
						config campaign_list;
						campaign_list["timestamp"] = lexical_cast<std::string>(epoch);
						if (req["times_relative_to"] != "now") {
							epoch = 0;
						}

						bool before_flag = false;
						time_t before = epoch;
						try {
							before = before + lexical_cast<time_t>(req["before"]);
							before_flag = true;
						} catch(bad_lexical_cast) {}

						bool after_flag = false;
						time_t after = epoch;
						try {
							after = after + lexical_cast<time_t>(req["after"]);
							after_flag = true;
						} catch(bad_lexical_cast) {}

						std::string name = req["name"], lang = req["language"];
						BOOST_FOREACH(const config &i, campaigns().child_range("campaign"))
						{
							if (!name.empty() && name != i["name"]) continue;
							std::string tm = i["timestamp"];
							if (before_flag && (tm.empty() || lexical_cast_default<time_t>(tm, 0) >= before)) continue;
							if (after_flag && (tm.empty() || lexical_cast_default<time_t>(tm, 0) <= after)) continue;
							if (!lang.empty()) {
								bool found = false;
								BOOST_FOREACH(const config &j, i.child_range("translation")) {
									if (j["language"] == lang) {
										found = true;
										break;
									}
								}
								if (!found) continue;
							}
							campaign_list.add_child("campaign", i);
						}

						BOOST_FOREACH(config &j, campaign_list.child_range("campaign")) {
							j["passphrase"] = t_string();
							j["upload_ip"] = t_string();
							j["email"] = t_string();
							j["feedback_url"] = t_string();

							// Build a feedback_url string attribute from the
							// internal [feedback] data.
							config url_params = j.child_or_empty("feedback");
							j.clear_children("feedback");

							if(!url_params.empty() && !feedback_url_format_.empty()) {
								j["feedback_url"] = format_addon_feedback_url(feedback_url_format_, url_params);
							}
						}

						config response;
						response.add_child("campaigns",campaign_list);

						std::cerr << " size: " << (network::send_data(response, sock)/1024) << "KiB\n";
					}
					else if (const config &req = data.child("request_campaign"))
					{
						LOG_CS << "sending campaign '" << req["name"] << "' to " << network::ip_address(sock)  << " using gzip";
						config &campaign = campaigns().find_child("campaign", "name", req["name"]);
						if (!campaign) {
							network::send_data(construct_error("Add-on '" + req["name"].str() + "' not found."), sock);
						} else {
							const int size = file_size(campaign["filename"]);

							if(size < 0) {
								std::cerr << " size: <unknown> KiB\n";
								LOG_CS << "File size unknown, aborting send.\n";
								network::send_data(construct_error("Add-on '" + req["name"].str() + "' could not be read by the server."), sock);
								continue;
							}

							std::cerr << " size: " << size/1024 << "KiB\n";
							network::send_file(campaign["filename"], sock);
							// Clients doing upgrades or some other specific thing shouldn't bump
							// the downloads count. Default to true for compatibility with old
							// clients that won't tell us what they are trying to do.
							if(req["increase_downloads"].to_bool(true)) {
								int downloads = campaign["downloads"].to_int() + 1;
								campaign["downloads"] = downloads;
							}
						}

					}
					else if (data.child("request_terms"))
					{
						// This usually means the client wants to upload content, so tell it
						// to give up when we're in read-only mode.
						if(read_only_) {
							LOG_CS << "in read-only mode, request for upload terms denied\n";
							network::send_data(construct_error("The server is currently in read-only mode, add-on uploads are disabled."), sock);
							continue;
						}

						LOG_CS << "sending terms " << network::ip_address(sock) << "\n";
						network::send_data(construct_message("All add-ons uploaded to this server must be licensed under the terms of the GNU General Public License (GPL). By uploading content to this server, you certify that you have the right to place the content under the conditions of the GPL, and choose to do so."), sock);
						LOG_CS << " Done\n";
					}
					else if (config &upload = data.child("upload"))
					{
						LOG_CS << "uploading campaign '" << upload["name"] << "' from " << network::ip_address(sock) << ".\n";
						config &data = upload.child("data");
						const std::string& name = upload["name"];
						std::string lc_name(name.size(), ' ');
						std::transform(name.begin(), name.end(), lc_name.begin(), tolower);
						config *campaign = NULL;
						BOOST_FOREACH(config &c, campaigns().child_range("campaign")) {
							if (utf8::lowercase(c["name"]) == lc_name) {
								campaign = &c;
								break;
							}
						}

						if (read_only_) {
							LOG_CS << "Upload aborted - uploads not permitted in read-only mode.\n";
							network::send_data(construct_error("Add-on rejected: The server is currently in read-only mode."), sock);
						} else if (!data) {
							LOG_CS << "Upload aborted - no add-on data.\n";
							network::send_data(construct_error("Add-on rejected: No add-on data was supplied."), sock);
						} else if (!addon_name_legal(upload["name"])) {
							LOG_CS << "Upload aborted - invalid add-on name.\n";
							network::send_data(construct_error("Add-on rejected: The name of the add-on is invalid."), sock);
						} else if (is_text_markup_char(upload["name"].str()[0])) {
							LOG_CS << "Upload aborted - add-on name starts with an illegal formatting character.\n";
							network::send_data(construct_error("Add-on rejected: The name of the add-on starts with an illegal formatting character."), sock);
						} else if (upload["title"].empty()) {
							LOG_CS << "Upload aborted - no add-on title specified.\n";
							network::send_data(construct_error("Add-on rejected: You did not specify the title of the add-on in the pbl file!"), sock);
						} else if (is_text_markup_char(upload["title"].str()[0])) {
							LOG_CS << "Upload aborted - add-on title starts with an illegal formatting character.\n";
							network::send_data(construct_error("Add-on rejected: The title of the add-on starts with an illegal formatting character."), sock);
						} else if (get_addon_type(upload["type"]) == ADDON_UNKNOWN) {
							LOG_CS << "Upload aborted - unknown add-on type specified.\n";
							network::send_data(construct_error("Add-on rejected: You did not specify a known type for the add-on in the pbl file! (See PblWML: wiki.wesnoth.org/PblWML)"), sock);
						} else if (upload["author"].empty()) {
							LOG_CS << "Upload aborted - no add-on author specified.\n";
							network::send_data(construct_error("Add-on rejected: You did not specify the author(s) of the add-on in the pbl file!"), sock);
						} else if (upload["version"].empty()) {
							LOG_CS << "Upload aborted - no add-on version specified.\n";
							network::send_data(construct_error("Add-on rejected: You did not specify the version of the add-on in the pbl file!"), sock);
						} else if (upload["description"].empty()) {
							LOG_CS << "Upload aborted - no add-on description specified.\n";
							network::send_data(construct_error("Add-on rejected: You did not specify a description of the add-on in the pbl file!"), sock);
						} else if (upload["email"].empty()) {
							LOG_CS << "Upload aborted - no add-on email specified.\n";
							network::send_data(construct_error("Add-on rejected: You did not specify your email address in the pbl file!"), sock);
						} else if (!check_names_legal(data)) {
							LOG_CS << "Upload aborted - invalid file names in add-on data.\n";
							network::send_data(construct_error("Add-on rejected: The add-on contains an illegal file or directory name."
									" File or directory names may not contain whitespace or any of the following characters: '/ \\ : ~'"), sock);
						} else if (campaign && (*campaign)["passphrase"].str() != upload["passphrase"]) {
							LOG_CS << "Upload aborted - incorrect passphrase.\n";
							network::send_data(construct_error("Add-on rejected: The add-on already exists, and your passphrase was incorrect."), sock);
						} else {
							const time_t upload_ts = time(NULL);

							LOG_CS << "Upload is owner upload.\n";
							std::string message = "Add-on accepted.";

							if (!version_info(upload["version"]).good()) {
								message += "\n<255,255,0>Note: The version you specified is invalid. This add-on will be ignored for automatic update checks.";
							}

							if(campaign == NULL) {
								campaign = &campaigns().add_child("campaign");
								(*campaign)["original_timestamp"] = lexical_cast<std::string>(upload_ts);
							}

							(*campaign)["title"] = upload["title"];
							(*campaign)["name"] = upload["name"];
							(*campaign)["filename"] = "data/" + upload["name"].str();
							(*campaign)["passphrase"] = upload["passphrase"];
							(*campaign)["author"] = upload["author"];
							(*campaign)["description"] = upload["description"];
							(*campaign)["version"] = upload["version"];
							(*campaign)["icon"] = upload["icon"];
							(*campaign)["translate"] = upload["translate"];
							(*campaign)["dependencies"] = upload["dependencies"];
							(*campaign)["upload_ip"] = network::ip_address(sock);
							(*campaign)["type"] = upload["type"];
							(*campaign)["email"] = upload["email"];

							if((*campaign)["downloads"].empty()) {
								(*campaign)["downloads"] = 0;
							}
							(*campaign)["timestamp"] = lexical_cast<std::string>(upload_ts);

							int uploads = (*campaign)["uploads"].to_int() + 1;
							(*campaign)["uploads"] = uploads;

							(*campaign).clear_children("feedback");
							if(const config& url_params = upload.child("feedback")) {
								(*campaign).add_child("feedback", url_params);
							}

							std::string filename = (*campaign)["filename"];
							data["title"] = (*campaign)["title"];
							data["name"] = "";
							data["campaign_name"] = (*campaign)["name"];
							data["author"] = (*campaign)["author"];
							data["description"] = (*campaign)["description"];
							data["version"] = (*campaign)["version"];
							data["timestamp"] = (*campaign)["timestamp"];
							data["original_timestamp"] = (*campaign)["original_timestamp"];
							data["icon"] = (*campaign)["icon"];
							data["type"] = (*campaign)["type"];
							(*campaign).clear_children("translation");
							find_translations(data, *campaign);

							add_license(data);

							{
								scoped_ostream campaign_file = ostream_file(filename);
								config_writer writer(*campaign_file, true, compress_level_);
								writer.write(data);
							}
//							write_compressed(*campaign_file, *data);

							(*campaign)["size"] = lexical_cast<std::string>(
									file_size(filename));
							scoped_ostream cfgfile = ostream_file(file_);
							write(*cfgfile, cfg_);
							network::send_data(construct_message(message), sock);

							fire("hook_post_upload", upload["name"]);
						}
					}
					else if (const config &erase = data.child("delete"))
					{
						if(read_only_) {
							LOG_CS << "in read-only mode, request to delete '" << erase["name"] << "' from " << network::ip_address(sock) << " denied\n";
							network::send_data(construct_error("Cannot delete add-on: The server is currently in read-only mode."), sock);
							continue;
						}

						LOG_CS << "deleting campaign '" << erase["name"] << "' requested from " << network::ip_address(sock) << "\n";
						const config &campaign = campaigns().find_child("campaign", "name", erase["name"]);
						if (!campaign) {
							network::send_data(construct_error("The add-on does not exist."), sock);
							continue;
						}

						if (campaign["passphrase"] != erase["passphrase"]
								&& (campaigns()["master_password"].empty()
								|| campaigns()["master_password"] != erase["passphrase"]))
						{
							network::send_data(construct_error("The passphrase is incorrect."), sock);
							continue;
						}

						//erase the campaign
						write_file(campaign["filename"], std::string());
						remove(campaign["filename"].str().c_str());

						config::child_itors itors = campaigns().child_range("campaign");
						for (size_t index = 0; itors.first != itors.second;
						     ++index, ++itors.first)
						{
							if (&campaign == &*itors.first) {
								campaigns().remove_child("campaign", index);
								break;
							}
						}
						scoped_ostream cfgfile = ostream_file(file_);
						write(*cfgfile, cfg_);
						network::send_data(construct_message("Add-on deleted."), sock);

						fire("hook_post_erase", erase["name"]);
					}
					else if (const config &cpass = data.child("change_passphrase"))
					{
						if(read_only_) {
							LOG_CS << "in read-only mode, request to change passphrase denied\n";
							network::send_data(construct_error("Cannot change passphrase: The server is currently in read-only mode."), sock);
							continue;
						}

						config &campaign = campaigns().find_child("campaign", "name", cpass["name"]);
						if (!campaign) {
							network::send_data(construct_error("No add-on with that name exists."), sock);
						} else if (campaign["passphrase"] != cpass["passphrase"]) {
							network::send_data(construct_error("Your old passphrase was incorrect."), sock);
						} else if (cpass["new_passphrase"].empty()) {
							network::send_data(construct_error("No new passphrase was supplied."), sock);
						} else {
							campaign["passphrase"] = cpass["new_passphrase"];
							scoped_ostream cfgfile = ostream_file(file_);
							write(*cfgfile, cfg_);
							network::send_data(construct_message("Passphrase changed."), sock);
						}
					}
				}
			} catch(network::error& e) {
				if(!e.socket) {
					LOG_CS << "fatal network error: " << e.message << "\n";
					throw;
				} else {
					LOG_CS << "client disconnect: " << e.message << " " << network::ip_address(e.socket) << "\n";
					e.disconnect();
				}
			} catch(const config::error& e) {
				network::connection err_sock = 0;
				network::connection const * err_connection = boost::get_error_info<network::connection_info>(e);
				if(err_connection != NULL) {
					err_sock = *err_connection;
				}
				if(err_sock == 0 && sock > 0)
					err_sock = sock;
				if(err_sock) {
					LOG_CS << "client disconnect due to exception: " << e.what() << " " << network::ip_address(err_sock) << "\n";
					network::disconnect(err_sock);
				} else {
					throw;
				}
			}

			SDL_Delay(20);
		}
	}

}

int main(int argc, char**argv)
{
	game_config::path = get_cwd();
	lg::timestamps(true);
	try {
		printf("argc %d argv[0] %s 1 %s\n",argc,argv[0],argv[1]);
		std::string cfg_path = normalize_path("server.cfg");
		if(argc >= 2 && atoi(argv[1])){
			campaign_server(cfg_path, atoi(argv[1])).run();
		}else {
			campaign_server(cfg_path).run();
		}
	} catch(config::error& /*e*/) {
		std::cerr << "Could not parse config file\n";
		return 1;
	} catch(io_exception& /*e*/) {
		std::cerr << "File I/O error\n";
		return 2;
	} catch(network::error& e) {
		std::cerr << "Aborted with network error: " << e.message << '\n';
		return 3;
	}
	return 0;
}
