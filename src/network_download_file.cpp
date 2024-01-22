/*
	Copyright (C) 2003 - 2024
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

#include <curl/curl.h>

static lg::log_domain log_network("network");
#define ERR_NW LOG_STREAM(err, log_network)
#define DBG_NW LOG_STREAM(debug, log_network)

namespace network
{
    static size_t write_callback(char* contents, size_t size, size_t nmemb, void* buffer)
    {
        size_t amount = size * nmemb;
        static_cast<std::string*>(buffer)->append(contents, amount);
        DBG_NW << "Downloaded " << amount << " bytes.";
        return amount;
    }

    void download(const std::string& url, const std::string& local_path)
    {
        CURL* curl = curl_easy_init();
        std::string buffer;
        // curl doesn't initialize the error buffer until version 7.60.0, which isn't currently available on all supported macOS versions
        char error[CURL_ERROR_SIZE];
        std::fill_n(error, CURL_ERROR_SIZE-1, ' ');
        error[CURL_ERROR_SIZE-1] = '\0';

        if(curl) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
            curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error);
            curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1L);
            curl_easy_setopt(curl, CURLOPT_FRESH_CONNECT, 1L);
            curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 5000L);
            curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);

            CURLcode res = curl_easy_perform(curl);

            if(res != CURLE_OK) {
                ERR_NW << "Error downloading file from url `" << url << "`.\n"
                    << "Short error: " << curl_easy_strerror(res) << "\n"
                    << "Long error: " << std::string(error);
                gui2::show_message(_("Download error"), _("An error occurred when downloading the file. Check the game logs for more information."), gui2::dialogs::message::button_style::auto_close);
            } else {
                try {
                    if(filesystem::file_exists(local_path)) {
                        const int res = gui2::show_message(_("Confirm overwrite"), _("Overwrite existing file?"), gui2::dialogs::message::yes_no_buttons);
                        if(res == gui2::retval::OK) {
                            filesystem::write_file(local_path, buffer);
                        }
                    } else {
                        filesystem::write_file(local_path, buffer);
                    }
                    DBG_NW << "Wrote downloaded file to: " << local_path;
                } catch(const filesystem::io_exception& e) {
                    ERR_NW << "io_exception writing downloaded data to file at: " << local_path
                        << "\n" << e.what() << " : " << e.message;
                    gui2::show_message(_("Download error"), _("An error occurred when downloading the file. Check the game logs for more information."), gui2::dialogs::message::button_style::auto_close);
                }
                gui2::show_message(_("Download complete"), _("The file has been downloaded."), gui2::dialogs::message::button_style::auto_close);
            }

            curl_easy_cleanup(curl);
        } else {
            ERR_NW << "curl_easy_init failed initialization, unable to download file.";
            gui2::show_message(_("Download error"), _("An error occurred when downloading the file. Check the game logs for more information."), gui2::dialogs::message::button_style::auto_close);
        }
    }
}
