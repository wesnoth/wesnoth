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
