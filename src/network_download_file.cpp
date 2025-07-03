/*
	Copyright (C) 2003 - 2025
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "network_download_file.hpp"

#include "filesystem.hpp"
#include "gettext.hpp"
#include "gui/dialogs/message.hpp"
#include "log.hpp"

#ifdef __ANDROID__
#include <SDL2/SDL_system.h>
#endif

#include <curl/curl.h>

static lg::log_domain log_network("network");
#define ERR_NW LOG_STREAM(err, log_network)
#define DBG_NW LOG_STREAM(debug, log_network)

namespace network
{
	static std::size_t write_callback(char* contents, std::size_t size, std::size_t nmemb, void* buffer)
	{
		std::size_t amount = size * nmemb;
		static_cast<std::string*>(buffer)->append(contents, amount);
		DBG_NW << "Downloaded " << amount << " bytes.";
		return amount;
	}

	void gui_download(const std::string& url, const std::string& local_path) {
		if(filesystem::file_exists(local_path)) {
			const int res = gui2::show_message(_("Confirm overwrite"), _("Overwrite existing file?"), gui2::dialogs::message::yes_no_buttons);
			if(res != gui2::retval::OK) {
				return;
			}
		}
		if (download(url, local_path)) {
			gui2::show_message(_("Download complete"), _("The file has been downloaded."), gui2::dialogs::message::button_style::auto_close);
		} else {
			gui2::show_message(_("Download error"), _("An error occurred when downloading the file. Check the game logs for more information."), gui2::dialogs::message::button_style::auto_close);
		}
	}

	bool download(const std::string& url, const std::string& local_path)
	{
		std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl(curl_easy_init(), curl_easy_cleanup);
		std::string buffer;
		// curl doesn't initialize the error buffer until version 7.60.0, which isn't currently available on all supported macOS versions
		char error[CURL_ERROR_SIZE];
		std::fill_n(error, CURL_ERROR_SIZE-1, ' ');
		error[CURL_ERROR_SIZE-1] = '\0';

		if(!curl) {
			ERR_NW << "curl_easy_init failed initialization, unable to download file.";
			return false;
		}

		CURLcode res;

		if(
#ifdef __ANDROID__
			(res = curl_easy_setopt(curl.get(), CURLOPT_CAINFO, (game_config::path + "/certificates/cacert.pem").c_str()) ) != CURLE_OK ||
#endif
			(res = curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str())) != CURLE_OK ||
			(res = curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, write_callback)) != CURLE_OK ||
			(res = curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &buffer)) != CURLE_OK ||
			(res = curl_easy_setopt(curl.get(), CURLOPT_ERRORBUFFER, error)) != CURLE_OK ||
			(res = curl_easy_setopt(curl.get(), CURLOPT_FORBID_REUSE, 1L)) != CURLE_OK ||
			(res = curl_easy_setopt(curl.get(), CURLOPT_FRESH_CONNECT, 1L)) != CURLE_OK ||
			(res = curl_easy_setopt(curl.get(), CURLOPT_FAILONERROR, 1L)) != CURLE_OK ||
			(res = curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT_MS, 5000L)) != CURLE_OK ||
#if LIBCURL_VERSION_NUM >= 0x075500
			(res = curl_easy_setopt(curl.get(), CURLOPT_PROTOCOLS_STR, "https")) != CURLE_OK
#else
			(res = curl_easy_setopt(curl.get(), CURLOPT_PROTOCOLS, CURLPROTO_HTTPS)) != CURLE_OK
#endif
			) {
			ERR_NW << "Error setting curl option: " << curl_easy_strerror(res);
			return false;
		}

		res = curl_easy_perform(curl.get());
		if(res != CURLE_OK) {
			ERR_NW << "Error downloading file from url `" << url << "`.\n"
				<< "Short error: " << curl_easy_strerror(res) << "\n"
				<< "Long error: " << std::string(error);
			return false;
		}

		try {
			filesystem::write_file(local_path, buffer);
			DBG_NW << "Wrote downloaded file to: " << local_path;
		} catch(const filesystem::io_exception& e) {
			ERR_NW << "io_exception writing downloaded data to file at: " << local_path << "\n" << e.what() << " : " << e.message;
			return false;
		}
		return true;
	}
}
