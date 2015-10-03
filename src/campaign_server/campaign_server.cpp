/* $Id$ */
/*
   Copyright (C) 2003 - 2011 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
   */

/**
 * @file campaign_server/campaign_server.cpp
 * Wesnoth addon server.
 * Expects a "server.cfg" config file in the current directory
 * and saves addons under data/.
 */

#include "filesystem.hpp"
#include "foreach.hpp"
#include "log.hpp"
#include "network_worker.hpp"
#include "serialization/binary_or_text.hpp"
#include "serialization/binary_wml.hpp"
#include "serialization/parser.hpp"
#include "serialization/string_utils.hpp"
#include "game_config.hpp"
#include "addon_checks.hpp"
#include "version.hpp"
#include "server/input_stream.hpp"
#include "util.hpp"

#include <csignal>

#include <boost/iostreams/filter/gzip.hpp>

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

	config construct_message(const std::string& msg)
	{
		config cfg;
		cfg.add_child("message")["message"] = msg;
		return cfg;
	}

	config construct_error(const std::string& msg)
	{
		config cfg;
		cfg.add_child("error")["message"] = "#Error: " + msg;
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
			void convert_binary_to_gzip();
			int load_config(); // return the server port
			const config &campaigns() const { return cfg_.child("campaigns"); }
			config &campaigns() { return cfg_.child("campaigns"); }
			config cfg_;
			const std::string file_;
			const network::manager net_manager_;
			const network::server_manager server_manager_;
			std::map<std::string, std::string> hooks_;
			input_stream* input_;
			int compress_level_;

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
		bool network_use_system_sendfile = utils::string_bool(cfg_["network_use_system_sendfile"],false);
		network_worker_pool::set_use_system_sendfile(network_use_system_sendfile);
 		cfg_["network_use_system_sendfile"] = (network_use_system_sendfile?"yes":"no");
 		/** Seems like compression level above 6 is waste of cpu cycle */
 		compress_level_ = lexical_cast_default<int>(cfg_["compress_level"],6);
 		cfg_["compress_level"] = lexical_cast<std::string>(compress_level_);
		return lexical_cast_default<int>(cfg_["port"], default_campaignd_port);
	}

	campaign_server::campaign_server(const std::string& cfgfile,
			size_t min_thread, size_t max_thread) :
		cfg_(),
		file_(cfgfile),
		net_manager_(min_thread,max_thread),
		server_manager_(load_config()),
		hooks_(),
		input_(0),
		compress_level_(compress_level_) // Already set by load_config()
	{
#ifndef _MSC_VER
		signal(SIGHUP, exit_sighup);
#endif
		signal(SIGINT, exit_sigint);
		signal(SIGTERM, exit_sigterm);

		if(cfg_.child("campaigns") == NULL) {
			cfg_.add_child("campaigns");
		}

		// load the hooks
		hooks_.insert(std::make_pair(std::string("hook_post_upload"), cfg_["hook_post_upload"]));
		hooks_.insert(std::make_pair(std::string("hook_post_erase"), cfg_["hook_post_erase"]));
	}

	void find_translations(const config& cfg, config& campaign)
	{
		BOOST_FOREACH (const config &dir, cfg.child_range("dir"))
		{
			if (dir["name"] == "LC_MESSAGES") {
				config &language = campaign.add_child("translation");
				language["language"] = cfg["name"];
			} else {
				find_translations(dir, campaign);
			}
		}
	}

	// Given an uploaded campaign, rename all .py files to .py.unchecked, unless
	// the contents are the same as a .py file in an existing campaign.
	// This means, a .py.unchecked file can be approved by simply renaming it
	// on the CS, and it will remain approved as long as it is not changed (but
	// it can be moved/renamed). If the .py file changes, it needs to be approved
	// again.
	std::string check_python_scripts(config &data, std::string filename)
	{
		std::vector<config *> python_scripts;
		find_scripts(data, ".py", python_scripts);
		if (python_scripts.empty()) return std::string();
		// Campaign contains python scripts.
		config old_campaign;
		scoped_istream stream = istream_file(filename);
		read_gz(old_campaign, *stream);
		std::vector<config *> old_scripts;
		find_scripts(old_campaign, ".py", old_scripts);
		std::string script_names;
		// Go through all newly uploaded python scripts.
		BOOST_FOREACH (config *i, python_scripts)
		{
			bool already = false;
			// Compare to existing, approved scripts.
			BOOST_FOREACH (const config *j, old_scripts)
			{
				if ((*i)["contents"] != (*j)["contents"]) continue;
				already = true;
				break;
			}
			if (!already) {
				script_names += "\n" + (*i)["name"];
				(*i)["name"] += ".unchecked";
			}
		}
		if (!script_names.empty())
			return "\nScripts awaiting approval:\n" + script_names;
		return std::string();
	}

	// Go through all .py.unchecked files in the given campaign, and rename them to
	// .py. This is the opposite to check_python_scripts(), and while the latter is
	// done on campaign upload, this function is called for the validate_scripts
	// command.
	std::string validate_all_python_scripts(config &data)
	{
		std::vector<config *> python_scripts;
		find_scripts(data, ".py.unchecked", python_scripts);
		if (python_scripts.empty()) return std::string();
		// Campaign contains unchecked python scripts.
		std::string script_names;
		BOOST_FOREACH (config *i, python_scripts)
		{
			std::string name = (*i)["name"];
			name.resize(name.length() - 10);
			(*i)["name"] = name;
			script_names += "\n" + name;
		}
		return script_names;
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
 	void campaign_server::convert_binary_to_gzip()
 	{
 		if (cfg_["converted_to_gzipped_data"] != "yes")
 		{
			// Convert all addons to gzip
			config::const_child_itors camps = campaigns().child_range("campaign");
			LOG_CS << "Converting all stored addons to gzip format. Number of addons: "
				<< std::distance(camps.first, camps.second) << '\n';

			BOOST_FOREACH (const config &cm, camps)
			{
				LOG_CS << "Converting " << cm["name"] << '\n';
				std::string filename = cm["filename"];
				scoped_istream binary_stream = istream_file(filename);
 				config data;
 				if (binary_stream->peek() == 31) //This is gzip file allready
 				{
 					LOG_CS << "Already converted\n";
 					continue;
 				}
 				read_compressed(data, *binary_stream);

				scoped_ostream gzip_stream = ostream_file(filename);
				config_writer writer(*gzip_stream, true, compress_level_);
 				writer.write(data);
 			}

 			cfg_["converted_to_gzipped_data"] = "yes";
 		}
		if (cfg_["encoded"] != "yes")
		{
			// Convert all addons to gzip
			config::const_child_itors camps = campaigns().child_range("campaign");
			LOG_CS << "Encoding all stored addons. Number of addons: "
				<< std::distance(camps.first, camps.second) << '\n';

			BOOST_FOREACH (const config &cm, camps)
			{
				LOG_CS << "Encoding " << cm["name"] << '\n';
				std::string filename = cm["filename"], newfilename = filename + ".new";

				{
					scoped_istream in_file = istream_file(filename);
					boost::iostreams::filtering_stream<boost::iostreams::input> in_filter;
					in_filter.push(boost::iostreams::gzip_decompressor());
					in_filter.push(*in_file);

					scoped_ostream out_file = ostream_file(newfilename);
					boost::iostreams::filtering_stream<boost::iostreams::output> out_filter;
					out_filter.push(boost::iostreams::gzip_compressor(boost::iostreams::gzip_params(compress_level_)));
					out_filter.push(*out_file);

					unsigned char c = in_filter.get();
					while( in_filter.good())
					{
						if (needs_escaping(c) && c != '\x01')
						{
							out_filter.put('\x01');
						   	out_filter.put(c+1);
						} else {
							out_filter.put(c);
						}
						c = in_filter.get();
					}
				}

				std::remove(filename.c_str());
				std::rename(newfilename.c_str(), filename.c_str());
			}

			cfg_["encoded"] = "yes";
		}
 	}

	void campaign_server::run()
	{
 		convert_binary_to_gzip();

 		if (!cfg_["control_socket"].empty())
 			input_ = new input_stream(cfg_["control_socket"]);
		bool gzipped;
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
				while((sock = network::receive_data(data, 0, &gzipped)) != network::null_connection) {
					if (const config &req = data.child("request_campaign_list"))
					{
						LOG_CS << "sending campaign list to " << network::ip_address(sock) << (gzipped?" using gzip":"");
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
						BOOST_FOREACH (const config &i, campaigns().child_range("campaign"))
						{
							if (!name.empty() && name != i["name"]) continue;
							std::string tm = i["timestamp"];
							if (before_flag && (tm.empty() || lexical_cast_default<time_t>(tm, 0) >= before)) continue;
							if (after_flag && (tm.empty() || lexical_cast_default<time_t>(tm, 0) <= after)) continue;
							if (!lang.empty()) {
								bool found = false;
								BOOST_FOREACH (const config &j, i.child_range("translation")) {
									if (j["language"] == lang) {
										found = true;
										break;
									}
								}
								if (!found) continue;
							}
							campaign_list.add_child("campaign", i);
						}

						BOOST_FOREACH (config &j, campaign_list.child_range("campaign")) {
							j["passphrase"] = t_string();
							j["upload_ip"] = t_string();
							j["email"] = t_string();
						}

						config response;
						response.add_child("campaigns",campaign_list);
						std::cerr << " size: " << (network::send_data(response, sock, gzipped)/1024) << "kb\n";
					}
					else if (const config &req = data.child("request_campaign"))
					{
						LOG_CS << "sending campaign '" << req["name"] << "' to " << network::ip_address(sock)  << (gzipped?" using gzip":"");
						config &campaign = campaigns().find_child("campaign", "name", req["name"]);
						if (!campaign) {
							network::send_data(construct_error("Add-on '" + req["name"] + "'not found."), sock, gzipped);
						} else {
 							if (gzipped)
 							{
								std::cerr << " size: " << (file_size(campaign["filename"])/1024) << "kb\n";
								network::send_file(campaign["filename"], sock);
 							} else {
								scoped_istream stream = istream_file(campaign["filename"]);
 								config cfg;
 								read_gz(cfg, *stream);
								std::cerr << " size: " <<
									(network::send_data(cfg, sock, false)/1024)
									<< "kb\n";
							}
							const int downloads = lexical_cast_default<int>(campaign["downloads"], 0) + 1;
							campaign["downloads"] = lexical_cast<std::string>(downloads);
						}

					}
					else if (data.child("request_terms"))
					{
						LOG_CS << "sending terms " << network::ip_address(sock) << "\n";
						network::send_data(construct_message("All add-ons uploaded to this server must be licensed under the terms of the GNU General Public License (GPL). By uploading content to this server, you certify that you have the right to place the content under the conditions of the GPL, and choose to do so."), sock, gzipped);
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
						BOOST_FOREACH (config &c, campaigns().child_range("campaign")) {
							if (utils::lowercase(c["name"]) == lc_name) {
								campaign = &c;
								break;
							}
						}
						if (!data) {
							LOG_CS << "Upload aborted - no add-on data.\n";
							network::send_data(construct_error("Add-on rejected: No add-on data was supplied."), sock, gzipped);
						} else if (!addon_name_legal(upload["name"])) {
							LOG_CS << "Upload aborted - invalid add-on name.\n";
							network::send_data(construct_error("Add-on rejected: The name of the add-on is invalid."), sock, gzipped);
						} else if (upload["title"].empty()) {
							LOG_CS << "Upload aborted - no add-on title specified.\n";
							network::send_data(construct_error("Add-on rejected: You did not specify the title of the add-on in the pbl file!"), sock, gzipped);
						} else if (get_addon_type(upload["type"]) == ADDON_UNKNOWN) {
							LOG_CS << "Upload aborted - unknown add-on type specified.\n";
							network::send_data(construct_error("Add-on rejected: You did not specify a known type for the add-on in the pbl file! (See PblWML: wiki.wesnoth.org/PblWML)"), sock, gzipped);
						} else if (upload["author"].empty()) {
							LOG_CS << "Upload aborted - no add-on author specified.\n";
							network::send_data(construct_error("Add-on rejected: You did not specify the author(s) of the add-on in the pbl file!"), sock, gzipped);
						} else if (upload["version"].empty()) {
							LOG_CS << "Upload aborted - no add-on version specified.\n";
							network::send_data(construct_error("Add-on rejected: You did not specify the version of the add-on in the pbl file!"), sock, gzipped);
						} else if (upload["description"].empty()) {
							LOG_CS << "Upload aborted - no add-on description specified.\n";
							network::send_data(construct_error("Add-on rejected: You did not specify a description of the add-on in the pbl file!"), sock, gzipped);
						} else if (!check_names_legal(data)) {
							LOG_CS << "Upload aborted - invalid file names in add-on data.\n";
							network::send_data(construct_error("Add-on rejected: The add-on contains an illegal file or directory name."
									" File or directory names may not contain any of the following characters: '/ \\ : ~'"), sock, gzipped);
						} else if (campaign && (*campaign)["passphrase"] != upload["passphrase"]) {
							// the user password failed, now test for the master password, in master password
							// mode the upload behaves different since it's only intended to update translations.
							// In a later version the translations will be separated from the addon.
							LOG_CS << "Upload is admin upload.\n";
							if (!campaigns()["master_password"].empty()
							    && campaigns()["master_password"] == upload["passphrase"])
							{
								std::string message = "Add-on accepted.";

								if (!version_info(upload["version"]).good()) {
									message += "\n#Note: The version you specified is invalid. This addon will be ignored for automatic update checks.";
								}

								std::string filename = (*campaign)["filename"];
								data["title"] = (*campaign)["title"];
								data["name"] = "";
								data["campaign_name"] = (*campaign)["name"];
								data["author"] = (*campaign)["author"];
								data["description"] = (*campaign)["description"];
								data["version"] = (*campaign)["version"];
								data["timestamp"] = (*campaign)["timestamp"];
								data["icon"] = (*campaign)["icon"];
								data["translate"] = (*campaign)["translate"];
								data["type"] = (*campaign)["type"];
								(*campaign).clear_children("translation");
								find_translations(data, *campaign);

								add_license(data);
								{
									scoped_ostream campaign_file = ostream_file(filename);
									config_writer writer(*campaign_file, true, compress_level_);
									writer.write(data);
								}
//								write_compressed(*campaign_file, *data);

								(*campaign)["size"] = lexical_cast<std::string>(
										file_size(filename));
								scoped_ostream cfgfile = ostream_file(file_);
								write(*cfgfile, cfg_);
								network::send_data(construct_message(message), sock, gzipped);

							} else {
								LOG_CS << "Upload aborted - incorrect passphrase.\n";
								network::send_data(construct_error("Add-on rejected: The add-on already exists, and your passphrase was incorrect."), sock, gzipped);
							}
						} else {
							LOG_CS << "Upload is owner upload.\n";
							std::string message = "Add-on accepted.";

							if (!version_info(upload["version"]).good()) {
								message += "\n<255,255,0>Note: The version you specified is invalid. This add-on will be ignored for automatic update checks.";
							}

							if(campaign == NULL) {
								campaign = &campaigns().add_child("campaign");
							}

							(*campaign)["title"] = upload["title"];
							(*campaign)["name"] = upload["name"];
							(*campaign)["filename"] = "data/" + upload["name"];
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
								(*campaign)["downloads"] = "0";
							}
							(*campaign)["timestamp"] = lexical_cast<std::string>(time(NULL));

							const int uploads = lexical_cast_default<int>((*campaign)["uploads"],0) + 1;
							(*campaign)["uploads"] = lexical_cast<std::string>(uploads);

							std::string filename = (*campaign)["filename"];
							data["title"] = (*campaign)["title"];
							data["name"] = "";
							data["campaign_name"] = (*campaign)["name"];
							data["author"] = (*campaign)["author"];
							data["description"] = (*campaign)["description"];
							data["version"] = (*campaign)["version"];
							data["timestamp"] = (*campaign)["timestamp"];
							data["icon"] = (*campaign)["icon"];
							data["type"] = (*campaign)["type"];
							(*campaign).clear_children("translation");
							find_translations(data, *campaign);

							// Campaigns which have python="allowed" are not checked
							if ((*campaign)["python"] != "allowed") {
								message += check_python_scripts(data, filename);
							}

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
							network::send_data(construct_message(message), sock, gzipped);

							fire("hook_post_upload", upload["name"]);
						}
					}
					else if (const config &erase = data.child("delete"))
					{
						LOG_CS << "deleting campaign '" << erase["name"] << "' requested from " << network::ip_address(sock) << "\n";
						const config &campaign = campaigns().find_child("campaign", "name", erase["name"]);
						if (!campaign) {
							network::send_data(construct_error("The add-on does not exist."), sock, gzipped);
							continue;
						}

						if (campaign["passphrase"] != erase["passphrase"]
								&& (campaigns()["master_password"].empty()
								|| campaigns()["master_password"] != erase["passphrase"]))
						{
							network::send_data(construct_error("The passphrase is incorrect."), sock, gzipped);
							continue;
						}

						//erase the campaign
						write_file(campaign["filename"], std::string());
						remove(campaign["filename"].c_str());

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
						network::send_data(construct_message("Add-on deleted."), sock, gzipped);

						fire("hook_post_erase", erase["name"]);
					}
					else if (const config &cpass = data.child("change_passphrase"))
					{
						config &campaign = campaigns().find_child("campaign", "name", cpass["name"]);
						if (!campaign) {
							network::send_data(construct_error("No add-on with that name exists."), sock, gzipped);
						} else if (campaign["passphrase"] != cpass["passphrase"]) {
							network::send_data(construct_error("Your old passphrase was incorrect."), sock, gzipped);
						} else if (cpass["new_passphrase"].empty()) {
							network::send_data(construct_error("No new passphrase was supplied."), sock, gzipped);
						} else {
							campaign["passphrase"] = cpass["new_passphrase"];
							scoped_ostream cfgfile = ostream_file(file_);
							write(*cfgfile, cfg_);
							network::send_data(construct_message("Passphrase changed."), sock, gzipped);
						}
					}
					else if (const config &cvalidate = data.child("validate_scripts"))
					{
						config &campaign = campaigns().find_child("campaign", "name", cvalidate["name"]);
						if (!campaign) {
							network::send_data(construct_error(
										"No add-on with that name exists."), sock, gzipped);
						} else if (campaigns()["master_password"].empty()) {
							network::send_data(construct_error(
										"Sever does not allow scripts."), sock, gzipped);
						} else if (campaigns()["master_password"] != cvalidate["master_password"]) {
							network::send_data(construct_error(
										"Password was incorrect."), sock, gzipped);
						} else {
							// Read the campaign from disk.
							config campaign_file;
							scoped_istream stream = istream_file(campaign["filename"]);
							read_gz(campaign_file, *stream);
							std::string scripts = validate_all_python_scripts(campaign_file);
							if (!scripts.empty()) {
								// Write the campaign with changed filenames back to disk
								{
									scoped_ostream ostream = ostream_file(campaign["filename"]);
									config_writer writer(*ostream, true, compress_level_);
									writer.write(campaign_file);
								}
//								write_compressed(*ostream, campaign_file);
								campaign["size"] = lexical_cast<std::string>(
										file_size(campaign["filename"]));

								network::send_data(construct_message("The following scripts have been validated: " +
											scripts), sock, gzipped);
							} else {
								network::send_data(construct_message("No unchecked scripts found!"), sock, gzipped);
							}
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
			} catch(config::error& /*e*/) {
				LOG_CS << "error in receiving data...\n";
				network::disconnect(sock);
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
