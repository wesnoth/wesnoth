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

/**
 * @file
 * Networked add-ons (campaignd) client interface.
 *
 * This API revolves around the campaignd client functionality. It depends on
 * the add-ons management and Asio network APIs.
 */

#pragma once

#include "addon/info.hpp"
#include "gui/dialogs/network_transmission.hpp"
#include "network_asio.hpp"

#include <set>

/**
 * Add-ons (campaignd) client class.
 *
 * This class encapsulates much of the logic behind the campaignd
 * add-ons server interaction for the client-side. Most networking
 * operations with it are implemented here.
 */
class addons_client
{
public:
	struct invalid_server_address : public std::exception {};
	struct not_connected_to_server : public std::exception {};
	struct user_exit : public std::exception {};
	struct user_disconnect : public std::exception {};

	addons_client(const addons_client&) = delete;
	addons_client& operator=(const addons_client&) = delete;

	/**
	 * Constructor.
	 *
	 * @param address Server address (e.g. "localhost:15999").
	 */
	explicit addons_client(const std::string& address);

	/**
	 * Tries to establish a connection to the add-ons server.
	 */
	void connect();

	/**
	 * Disconnects from the add-on server.
	 */
	void disconnect()
	{
		conn_.reset();
		clear_last_error();
		clear_server_info();
	}

	/** Returns the current hostname:port used for this connection. */
	const std::string& addr() const { return addr_; }

	/** Returns the last error message sent by the server, or an empty string. */
	const std::string& get_last_server_error() const { return last_error_; }

	/** Returns the last error message extra data sent by the server, or an empty string. */
	const std::string& get_last_server_error_data() const { return last_error_data_; }

	/** Returns true if the client is connected to the server. */
	bool is_connected() { return conn_ != nullptr; }

	/**
	 * Request the add-ons list from the server.
	 *
	 * @return @a true on success, @a false on failure. Retrieve the error message with @a get_last_server_error.
	 *
	 * @param cfg A config object whose contents are replaced with
	 *            the server's list if available, cleared otherwise.
	 * @param icons Whether to have the add-ons server populate the icon
	 */
	bool request_addons_list(config& cfg, bool icons);

	std::map<std::string, int> get_addon_count_by_type();
	config get_addon_downloads_by_version(const std::string& addon);
	config get_forum_auth_usage();
	config get_addon_admins();
	bool hide_addon(const std::string& addon, const std::string& username, const std::string& passphrase);
	bool unhide_addon(const std::string& addon, const std::string& username, const std::string& passphrase);
	config get_hidden_addons(const std::string& username, const std::string& passphrase);

	/**
	 * Retrieves the add-ons server web URL if available.
	 */
	const std::string& server_url() const
	{
		return server_url_;
	}

	/**
	 * Request the add-ons server distribution terms message.
	 */
	bool request_distribution_terms(std::string& terms);

	/**
	 * Installation outcome values.
	 */
	enum class install_outcome
	{
		/** The add-on was correctly installed. */
		success,
		/** The add-on could not be downloaded from the server. */
		failure,
		/** User aborted the operation because of an issue with dependencies or chose not to overwrite the add-on. */
		abort,
	};

	/**
	 * Contains the outcome of an add-on install operation.
	 */
	struct install_result
	{
		/**
		 * Overall outcome of the operation.
		 */
		install_outcome outcome;

		/**
		 * Specifies if WML on disk was altered and needs to be reloaded.
		 *
		 * @note Failure to install an add-on properly may not necessarily mean
		 * that WML on disk was left unchanged (e.g. if any dependencies were
		 * succesfully installed first.)
		 */
		bool wml_changed;
	};

	/**
	 * Performs an add-on download and install cycle.
	 *
	 * This checks and prompts the user through the UI before overwriting an
	 * existing add-on with a .pbl file or version control system files (.git/,
	 * .svn/, etc.). It also resolves add-on dependencies and downloads them
	 * using the same system before downloading the original target add-on.
	 *
	 * @param addons             Add-ons list used for resolving dependencies.
	 * @param addon              Identity of the singular add-on that will be
	 *                           downloaded.
	 *
	 * @return An install_result with the outcome of the operation.
	 */
	install_result install_addon_with_checks(const addons_list& addons, const addon_info& addon);

	/**
	 * Uploads an add-on to the server.
	 *
	 * This method reads the add-on upload passphrase and other data
	 * from the associated .pbl file. If the .pbl file doesn't have a
	 * passphrase, a new, random one will be automatically generated
	 * and written to the file for the user.
	 *
	 * @todo Notify the user about the automatic passphrase.
	 *
	 * @return @a true on success, @a false on failure. Retrieve the error message with @a get_last_server_error.
	 *
	 * @param id               Id. of the add-on to upload.
	 * @param response_message The server response message on success, such as "add-on accepted".
	 * @param cfg              The pbl config of the add-on with the specified id.
	 * @param local_only       Whether the addon is not present on the server.
	 */
	bool upload_addon(const std::string& id, std::string& response_message, config& cfg, bool local_only);

	/**
	 * Requests the specified add-on to be removed from the server.
	 *
	 * This method reads the add-on upload passphrase from the associated
	 * .pbl file.
	 *
	 * @return @a true on success, @a false on failure. Retrieve the error message with @a get_last_server_error.
	 *
	 * @param id               ID of the add-on to take down.
	 * @param response_message The server response message on success, such as "add-on accepted".
	 * @param admin_set        The list of admin usernames as provided by the server, if doing an admin-restricted action
	 */
	bool delete_remote_addon(const std::string& id, std::string& response_message, const std::set<std::string>& admin_set = {});

	/**
	 * Returns whether the server supports the given named capability.
	 */
	bool server_supports(const std::string& cap_id) const
	{
		return server_capabilities_.find(cap_id) != server_capabilities_.end();
	}

	/**
	 * Returns whether the server supports incremental (delta) downloads and uploads.
	 */
	bool server_supports_delta() const
	{
		return server_supports("delta");
	}

	/**
	 * Returns whether the server supports passphrase authentication on an add-on basis.
	 */
	bool server_supports_legacy_auth() const
	{
		return server_supports("auth:legacy");
	}

	/**
	 * Returns whether the current connection uses TLS.
	 */
	bool using_tls() const
	{
		return conn_ && conn_->using_tls();
	}

	const std::string& server_id() const
	{
		return server_id_;
	}

	const std::string& server_version() const
	{
		return server_version_;
	}

private:
	enum class transfer_mode {download, connect, upload};

	std::string addr_;
	std::string host_;
	std::string port_;
	std::unique_ptr<network_asio::connection> conn_;
	std::string last_error_;
	std::string last_error_data_;

	std::string server_id_;
	std::string server_version_;
	std::set<std::string> server_capabilities_;
	std::string server_url_;
	std::string license_notice_;

	/**
	* Downloads the specified add-on from the server.
	*
	* @return @a true on success, @a false on failure. Retrieve the error message with @a get_last_server_error.
	*
	* @param archive_cfg         Config object to hold the downloaded add-on archive data.
	* @param id                  Add-on id.
	* @param title               Add-on title, used for status display.
	* @param version             Specifies an add-on version to download.
	* @param increase_downloads  Whether to request the server to increase the add-on's
	*                            download count or not (e.g. when upgrading).
	*/
	bool download_addon(config& archive_cfg, const std::string& id, const std::string& title, const version_info& version, bool increase_downloads = true);

	/**
	* Installs the specified add-on using an archive received from the server.
	*
	* An _info.cfg file will be added to the local directory for the add-on
	* to keep track of version and dependency information.
	*/
	bool install_addon(config& archive_cfg, const addon_info& info);

	// Asks the client to download and install an addon, reporting errors in a gui dialog. Returns true if new content was installed, false otherwise.
	bool try_fetch_addon(const addon_info& addon);

	/**
	* Warns the user about unresolved dependencies and installs them if they choose to do so.
	* Returns: outcome: abort in case the user chose to abort because of an issue
	*                   success otherwise
	*          wml_change: indicates if new wml content was installed
	*/
	install_result do_resolve_addon_dependencies(const addons_list& addons, const addon_info& addon);

	/** Checks whether the given add-on has local .pbl or VCS information and asks before overwriting it. */
	bool do_check_before_overwriting_addon(const addon_info& addon);

	/** Makes sure the add-ons server connection is working. */
	void check_connected() const;

	/**
	 * Sends a request to the add-ons server.
	 *
	 * @note This is an asynchronous operation. @a display_status_window
	 * should be called afterwards to wait for it to finish.
	 *
	 * @param request  The client request WML.
	 * @param response A config object whose contents are replaced
	 *                 with the server response WML.
	 */
	void send_request(const config& request, config& response);

	/**
	 * Sends a simple request message to the add-ons server.
	 *
	 * The real request sent consists of a WML object with an empty
	 * child node whose name corresponds to @a request_string
	 *
	 * @note This is an asynchronous operation. @a display_status_window
	 * should be called afterwards to wait for it to finish.
	 *
	 * @param request_string  The client request string.
	 * @param response        A config object whose contents are replaced
	 *                        with the server response WML.
	 */
	void send_simple_request(const std::string& request_string, config& response);

	/**
	 * Waits for a network transfer, displaying a status window.
	 *
	 * The window is displayed with the specified contents. This
	 * method doesn't return until the network transfer is complete. It
	 * will throw a @a user_exit exception if the user cancels the
	 * transfer by canceling the status window.
	 */
	void wait_for_transfer_done(const std::string& status_message, transfer_mode mode = transfer_mode::download);

	/**
	 * If the response has the [error] child, then check for the status_code attribute.
	 * If the error response has the status_code attribute, then the status_code attribute is assigned to last_error_, else the message attribute is assigned to last_error_.
	 * If the error response doesn't have the status_code attribute, then assign the message attribute to last_error.
	 * Also assign the error response's extra_data attribute to last_error_data_.
	 * If there is no [error] child, then clear last_error_ and last_error_data_.
	 *
	 * @param response_cfg The config returned by the add-ons server.
	 * @return true if response_cfg had an [error] child, false otherwise.
	 */
	bool is_error_response(const config& response_cfg);

	void clear_last_error();

	void clear_server_info();
};
