/* $Id$ */
/*
   Copyright (C) 2003-5 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "config.hpp"
#include "filesystem.hpp"
#include "log.hpp"
#include "network.hpp"
#include "publish_campaign.hpp"
#include "util.hpp"
#include "serialization/binary_wml.hpp"
#include "serialization/parser.hpp"

#include "SDL.h"

#include <iostream>

#define LOG_CS lg::err(lg::network, false)

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
	cfg.add_child("error")["message"] = msg;
	return cfg;
}

class campaign_server
{
public:
	explicit campaign_server(const std::string& cfgfile);
	void run();
private:
	int load_config(); // return the server port
	const config& campaigns() const { return *cfg_.child("campaigns"); }
	config& campaigns() { return *cfg_.child("campaigns"); }
	config cfg_;
	const std::string file_;
	const network::manager net_manager_;
	const network::server_manager server_manager_;

};

int campaign_server::load_config()
{
	scoped_istream stream = istream_file(file_);
	read(cfg_, *stream);
	return lexical_cast_default<int>(cfg_["port"], 15002);
}

campaign_server::campaign_server(const std::string& cfgfile)
     : file_(cfgfile), net_manager_(5), server_manager_(load_config())
{
	if(cfg_.child("campaigns") == NULL) {
		cfg_.add_child("campaigns");
	}
}

void find_translations(const config& cfg, config& campaign)
{
        const config::child_list& dirs = cfg.get_children("dir");
        for(config::child_list::const_iterator i = dirs.begin(); i != dirs.end(); ++i) {
		if((const t_string)(**i)["name"] == "LC_MESSAGES") {
			config* language = &campaign.add_child("translation");
			(*language)["language"] = cfg["name"];
		}
		else {
			find_translations(**i, campaign);
		}
        }
}


void campaign_server::run()
{
	for(int increment = 0; ; ++increment) {
		try {
			//write config to disk every ten minutes
			if((increment%(60*10*2)) == 0) {
				scoped_ostream cfgfile = ostream_file(file_);
				write(*cfgfile, cfg_);
			}

			network::process_send_queue();

			network::connection sock = network::accept_connection();
			if(sock) {
				LOG_CS << "received connection from " << network::ip_address(sock) << "\n";
			}

			config data;
			while((sock = network::receive_data(data)) != network::null_connection) {
				if(const config* req = data.child("request_campaign_list")) {
					time_t epoch = time(NULL);
					config campaign_list;
					(campaign_list)["timestamp"] = lexical_cast<std::string>(epoch);
					if((const t_string)(*req)["times_relative_to"] != "now") {
						epoch = 0;
					}
					int before_flag = 0;
					time_t before = epoch;
					if((const t_string)(*req)["before"] != "") {
						try {
							before = before + lexical_cast<time_t>((*req)["before"]);
							before_flag = 1;
						}
						catch(bad_lexical_cast) {
						}
					}
					int after_flag = 0;
					time_t after = epoch;
					if((const t_string)(*req)["after"] != "") {
						try {
							after = after + lexical_cast<time_t>((*req)["after"]);
							after_flag = 1;
						}
						catch(bad_lexical_cast) {
						}
					}
					config::child_list cmps = campaigns().get_children("campaign");
					for(config::child_list::iterator i = cmps.begin(); i != cmps.end(); ++i) {
						if((const t_string)(*req)["name"] != "" && (*req)["name"] != (**i)["name"]) continue;
						if(before_flag && ((const t_string)(**i)["timestamp"] == "" || lexical_cast_default<time_t>((**i)["timestamp"],0) >= before)) continue;
						if(after_flag && ((const t_string)(**i)["timestamp"] == "" || lexical_cast_default<time_t>((**i)["timestamp"],0) <= after)) continue;
						int found = 1;
						if((const t_string)(*req)["language"] != "") {
							found = 0;
							config::child_list translation = (**i).get_children("translation");
							for(config::child_list::iterator j = translation.begin(); j != translation.end(); ++j) {
								if((*req)["language"] == (**j)["language"]) {
									found = 1;
									break;
								}
							}
						}
						if(found == 0) continue;
						campaign_list.add_child("campaign", (**i));
					}
					cmps = campaign_list.get_children("campaign");
					for(config::child_list::iterator i = cmps.begin(); i != cmps.end(); ++i) {
						(**i)["passphrase"] = "";
					}

					config response;
					response.add_child("campaigns",campaign_list);
					network::send_data(response,sock);
				} else if(const config* req = data.child("request_campaign")) {
					config* const campaign = campaigns().find_child("campaign","name",(*req)["name"]);
					if(campaign == NULL) {
						network::send_data(construct_error("Campaign not found."),sock);
					} else {
						config cfg;
						scoped_istream stream = istream_file((*campaign)["filename"]);
						read_compressed(cfg, *stream);
						network::queue_data(cfg,sock);

						const int downloads = lexical_cast_default<int>((*campaign)["downloads"],0)+1;
						(*campaign)["downloads"] = lexical_cast<std::string>(downloads);
					}

				} else if(data.child("request_terms") != NULL) {
					network::send_data(construct_message("All campaigns uploaded to this server must be licensed under the terms of the GNU General Public License (GPL). By uploading content to this server, you certify that you have the right to place the content under the conditions of the GPL, and choose to do so."),sock);
				} else if(config* upload = data.child("upload")) {
					config* campaign = campaigns().find_child("campaign","name",(*upload)["name"]);
					if(campaign != NULL && (*campaign)["passphrase"] != (*upload)["passphrase"]) {
						network::send_data(construct_error("The campaign already exists, and your passphrase was incorrect."),sock);
					} else if(campaign_name_legal((*upload)["name"]) == false) {
						network::send_data(construct_error("The name of the campaign is invalid"),sock);
					} else {
						if(campaign == NULL) {
							campaign = &campaigns().add_child("campaign");
						}

						(*campaign)["title"] = (*upload)["title"];
						(*campaign)["name"] = (*upload)["name"];
						(*campaign)["filename"] = (*upload)["name"];
						(*campaign)["passphrase"] = (*upload)["passphrase"];
						(*campaign)["author"] = (*upload)["author"];
						(*campaign)["description"] = (*upload)["description"];
						(*campaign)["version"] = (*upload)["version"];
						(*campaign)["icon"] = (*upload)["icon"];

						if((*campaign)["downloads"].empty()) {
							(*campaign)["downloads"] = "0";
						}
						(*campaign)["timestamp"] = lexical_cast<std::string>(time(NULL));

						config* data = upload->child("data");
						if(data != NULL) {
							std::string filename = (*campaign)["filename"];
							(*data)["title"] = (*campaign)["title"];
							(*data)["name"] = (*campaign)["name"];
							(*data)["author"] = (*campaign)["author"];
							(*data)["description"] = (*campaign)["description"];
							(*data)["version"] = (*campaign)["version"];
							(*data)["timestamp"] = (*campaign)["timestamp"];
							(*data)["icon"] = (*campaign)["icon"];
							find_translations(*data, *campaign);
							{
								scoped_ostream campaign_file = ostream_file(filename);
								write_compressed(*campaign_file, *data);
							}
							(*campaign)["size"] = lexical_cast<std::string>(
								file_size(filename));
						}

						scoped_ostream cfgfile = ostream_file(file_);
						write(*cfgfile, cfg_);
						network::send_data(construct_message("Campaign accepted."),sock);
					}
				} else if(const config* erase = data.child("delete")) {
					config* const campaign = campaigns().find_child("campaign","name",(*erase)["name"]);
					if(campaign == NULL) {
						network::send_data(construct_error("The campaign does not exist."),sock);
						continue;
					}

					if((*campaign)["passphrase"] != (*erase)["passphrase"]) {
						network::send_data(construct_error("The passphrase is incorrect."),sock);
						continue;
					}

					//erase the campaign
					write_file((*campaign)["filename"],"");

					const config::child_list& campaigns_list = campaigns().get_children("campaign");
					const size_t index = std::find(campaigns_list.begin(),campaigns_list.end(),campaign) - campaigns_list.begin();
					campaigns().remove_child("campaign",index);
					scoped_ostream cfgfile = ostream_file(file_);
					write(*cfgfile, cfg_);
					network::send_data(construct_message("Campaign erased."),sock);
				}
			}
		} catch(network::error& e) {
			if(!e.socket) {
				LOG_CS << "fatal network error\n";
				break;
			} else {
				e.disconnect();
			}
		} catch(config::error& e) {
			LOG_CS << "error in receiving data...\n";
		}

		SDL_Delay(500);
	}
}

}

int main(int argc, char** argv)
{
	lg::timestamps(true);
	try {
		campaign_server("server.cfg").run();
	} catch(config::error& e) {
		std::cerr << "Could not parse config file\n";
	} catch(io_exception& e) {
		std::cerr << "File I/O error\n";
	}
}
