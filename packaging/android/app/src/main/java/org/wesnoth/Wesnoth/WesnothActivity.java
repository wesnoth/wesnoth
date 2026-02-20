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

import java.io.File;

import android.content.Intent;
import android.content.IntentFilter;
import android.net.Uri;
import android.os.BatteryManager;
import android.os.Build;
import android.util.Log;

import androidx.core.content.FileProvider;

import org.libsdl.app.SDLActivity;

public class WesnothActivity extends SDLActivity
{

	// Needs to be inside an activity so we can use `startActivity`.
	public void open(String url) {
		Log.d("WesnothActivity", "opening " + url);
		Intent openIntent = new Intent(Intent.ACTION_VIEW);
		if (url.startsWith("http://") || url.startsWith("https://")) {
			openIntent.setData(Uri.parse(url));
		} else {
			File file = new File(url);
			Uri uri = FileProvider.getUriForFile(this, getPackageName() + ".fileprovider", file);
			openIntent.setDataAndType(uri, "*/*");
			openIntent.setFlags(
				Intent.FLAG_GRANT_READ_URI_PERMISSION
				| Intent.FLAG_GRANT_WRITE_URI_PERMISSION
			);
		}
		startActivity(openIntent);
	}

	public double getBatteryPercentage() {
		// From https://stackoverflow.com/a/42327441
		if (Build.VERSION.SDK_INT >= 21) {
			BatteryManager bm = (BatteryManager) getSystemService(BATTERY_SERVICE);
			return (double) bm.getIntProperty(BatteryManager.BATTERY_PROPERTY_CAPACITY);
		} else {
			IntentFilter iFilter = new IntentFilter(Intent.ACTION_BATTERY_CHANGED);
			Intent batteryStatus = registerReceiver(null, iFilter);
			int level = batteryStatus != null ? batteryStatus.getIntExtra(BatteryManager.EXTRA_LEVEL, -1) : -1;
			int scale = batteryStatus != null ? batteryStatus.getIntExtra(BatteryManager.EXTRA_SCALE, -1) : -1;
			double batteryPct = level / (double) scale;
			return batteryPct * 100;
		}
	}

}
