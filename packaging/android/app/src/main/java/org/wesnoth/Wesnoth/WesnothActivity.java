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

import android.app.Activity;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.Uri;
import android.os.BatteryManager;
import android.os.Build;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.View;
import android.view.WindowInsets;
import android.view.WindowInsetsController;
import android.content.pm.ActivityInfo;
import android.graphics.Rect;
import androidx.core.content.FileProvider;

import org.libsdl.app.SDLActivity;

public class WesnothActivity extends SDLActivity
{
	public static String getFullscreenResolution(Activity activity) {
		int w, h;

		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
			Rect bounds = activity.getWindowManager()
					.getMaximumWindowMetrics().getBounds();
			w = bounds.width();
			h = bounds.height();
		} else {
			DisplayMetrics metrics = new DisplayMetrics();
			activity.getWindowManager().getDefaultDisplay().getRealMetrics(metrics);
			w = metrics.widthPixels;
			h = metrics.heightPixels;
		}

		return w + "x" + h;
	}
	
	/**
	 * FIXME For some reason SDL3 migration broke the resolution detection.
	 * Workaround: calculate resolution in Java then pass it to Wesnoth.
	 */
	@Override
	protected String[] getArguments() {
		String resStr = "-r " + getFullscreenResolution(this);
		return new String[] { resStr };
	}

	/**
	 * Enforces immersive fullscreen on every focus gain.
	 * Needed because system UI can reappear after notifications/dialogs.
	 * API 30+: hides insets and extends content behind system bars.
	 * API <30: uses legacy immersive sticky flags with layout flags to
	 *           prevent gray padding where bars used to be.
	 */
	@Override
	public void onWindowFocusChanged(boolean hasFocus) {
		// hide system bars, navigation buttons, insets
		super.onWindowFocusChanged(hasFocus);
		if (hasFocus) {
			if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
				getWindow().setDecorFitsSystemWindows(false);
				getWindow().getInsetsController().hide(
					WindowInsets.Type.statusBars() | WindowInsets.Type.navigationBars()
				);
				getWindow().getInsetsController().setSystemBarsBehavior(
					WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE
				);
			} else {
				getWindow().getDecorView().setSystemUiVisibility(
					View.SYSTEM_UI_FLAG_FULLSCREEN
					| View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
					| View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
					| View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
					| View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
					| View.SYSTEM_UI_FLAG_LAYOUT_STABLE
				);
			}
		}
	}

	/** Enforce landscape orientation */
	@Override
	public void setOrientationBis(int w, int h, boolean resizable, String hint) {
		SDLActivity.mSingleton.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
	}

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

	public String getLocaleCode() {
		return SDLActivity.mCurrentLocale.toString();
	}
}
