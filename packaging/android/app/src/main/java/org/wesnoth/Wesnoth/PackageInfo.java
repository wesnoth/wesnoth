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

import java.util.ArrayList;
import java.util.List;
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
	
	public int getPatchVersion() {
		return getPatchVersion(version);
	}
	
	// 0 -> major version, 1 -> minor version, 2 -> patch version
	public static int getPatchVersion(String version) {
		List<Integer> versionParts = decodeNumericVersion(version);
		return versionParts.size() >= 3 ? versionParts.get(2) : 0;
	}
	
	// format: 1.19.18.1 etc.
	public static List<Integer> decodeNumericVersion(String version) {
		List<Integer> nversion = new ArrayList<Integer>();
		for (String part : version.split("\\.")) {
			nversion.add(Integer.parseInt(part));
		}
		return nversion;
	}
	
	public static PackageInfo from(Properties prop, String id) {
		return new PackageInfo(
			id,
			prop.getProperty(id + ".name", id),
			prop.getProperty(id + ".version"),
			prop.getProperty(id + ".url")
		);
	}
	
	@Override
	public String toString() {
		return String.format("Package[id=%s,uiname=%s,version=%s,url=%s]", id, uiname, version, url);
	}
}
