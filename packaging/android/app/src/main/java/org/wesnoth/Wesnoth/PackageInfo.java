/*
	Copyright (C) 2025
	by Subhraman Sarkar (babaissarkar) <suvrax@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

package org.wesnoth.Wesnoth;

import java.util.Properties;

public class PackageInfo {
	private String id, uiname, version, url;
	
	private PackageInfo(String id, String uiname, String version, String url) {
		this.id = id;
		this.uiname = uiname;
		this.version = version;
		this.url = url;
	}
	
	public String getId() {
		return id;
	}
	
	public String getUIName() {
		return uiname;
	}
	
	public String getVersion() {
		return version;
	}
	
	public String getURL() {
		return url;
	}
	
	public static PackageInfo from(Properties prop, String id) {
		return new PackageInfo(
			id,
			prop.getProperty(id + ".name", id),
			prop.getProperty(id + ".version"),
			prop.getProperty(id + ".url")
		);
	}
}
