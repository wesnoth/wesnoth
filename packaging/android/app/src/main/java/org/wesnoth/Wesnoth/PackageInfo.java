/*
	Copyright (C) 2025
	by Subhraman Sarkar (babaissarkar) <sbmskmm@protonmail.com>
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
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Properties;

import android.util.Log;

public class PackageInfo {
	private String id, uiname, version, url;
	private HashMap<String, String> depends, excludes;
	
	private PackageInfo(String id, String uiname, String version, String url, String deps, String excludes) {
		this.id = id;
		this.uiname = uiname;
		this.version = version;
		this.url = url;
		this.depends = parsePackageList(deps);
		this.excludes = parsePackageList(excludes);
	}
	
	// example `deps` format: core==1.19.19, music==1.19.18
	private HashMap<String, String> parsePackageList(String depsString) {
		HashMap<String, String> list = new HashMap<String, String>();
		if (depsString.isEmpty()) {
			return list; 
		}

		for (String pkg : depsString.split(",\\s*")) {
			String[] parts = pkg.split("==");
			if (parts.length >= 2) {
				//example: list.put("core", "1.19.19")
				list.put(parts[0].strip(), parts[1].strip());
			}
		}
		return list;
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
	
	// any occurance of %s in url will be replaced with version
	public String getURL() {
		return String.format(url, version);
	}
	
	public int getPatchVersion() {
		return getPatchVersion(version);
	}
	
	public HashMap<String, String> getDependencies() {
		return this.depends;
	}
	
	public HashMap<String, String> getExcluded() {
		return this.excludes;
	}
	
	// example: 1.19.18.2 -> 18
	public static int getPatchVersion(String version) {
		// in versionParts, 0 -> major version, 1 -> minor version, 2 -> patch version
		List<Integer> versionParts = decodeNumericVersion(version);
		return versionParts.size() >= 3 ? versionParts.get(2) : 0;
	}
	
	// `version` format: 1.19.18.1 etc.
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
			prop.getProperty(id + ".url"),
			prop.getProperty(id + ".depends", ""),
			prop.getProperty(id + ".excludes", "")
		);
	}
	
	@Override
	public String toString() {
		return String.format("Package[id=%s,uiname=%s,version=%s,url=%s,deps=%s,excl=%s]",
			id, uiname, version, url, depends, excludes);
	}
}
