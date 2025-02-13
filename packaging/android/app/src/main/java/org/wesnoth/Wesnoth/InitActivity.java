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

import java.io.DataInputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.IOException;
import java.net.URL;
import java.net.MalformedURLException;
import java.net.HttpURLConnection;

import android.app.Activity;
import android.app.DownloadManager;
import android.content.pm.PackageManager;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.widget.ProgressBar;
import android.widget.TextView;

public class InitActivity extends Activity {
	private int length = 0;
	private int progress = 0;
	private long max = 0;
	
	private String toSizeString(long bytes) {
		return String.format("%4.2f MB", (bytes * 1.0f)/(1e6));
	}

	@Override
	protected void onCreate(Bundle savedState) {
		super.onCreate(savedState);
		this.setContentView(R.layout.activity_init);
		
		TextView progressText = (TextView) findViewById(R.id.download_msg);
		progressText.setText("Connecting...");

		Thread thread = new Thread(new Runnable() {
			@Override
			public void run() {
				try {
					downloadFile(
						"https://sourceforge.net/projects/wesnoth/files/wesnoth/wesnoth-1.19.8/wesnoth-1.19.8.tar.bz2/download",
						getExternalFilesDir(null) + "/gamedata/wesnoth-1.19.8.tar.bz2"
					);
				} catch (Exception e) {
					Log.e("Download", "security error", e);
				}
			}
		});

		thread.start();
	}

	public void launchWesnothActivity() {
		Intent launchIntent = new Intent(InitActivity.this, WesnothActivity.class);
		startActivity(launchIntent);
	}

	public void updateUI(int progress) {
		TextView progressText = (TextView) findViewById(R.id.download_msg);
		ProgressBar progressBar = (ProgressBar) findViewById(R.id.download_progress);
		progressBar.setProgress(progress);
		progressText.setText(
			"Downloading game data... ("
			+ toSizeString(progress)
			+ "/"
			+ toSizeString(max)
			+ ")"
		);
	}

	public void downloadFile(String url, String destpath) {
		// based on https://stackoverflow.com/a/4896527/22060628
		try {
			URL downloadURL = new URL(url);
			HttpURLConnection conn = (HttpURLConnection) downloadURL.openConnection();
			conn.setRequestProperty("Accept", "*/*");
			conn.setRequestProperty("User-Agent", "Wget/1.13.4 (linux-gnu)");
			conn.setRequestMethod("GET");
			conn.setConnectTimeout(10000); // 10 seconds
			conn.setReadTimeout(10000); // 10 seconds

			int response = conn.getResponseCode();

			if (response != HttpURLConnection.HTTP_OK) {
				Log.e("Download", "Server returned response : " + response);
				return;
			}

			DataInputStream in = new DataInputStream(conn.getInputStream());
			OutputStream out = new FileOutputStream(new File(destpath));

			byte[] buffer = new byte[4096];
			
			max = conn.getContentLengthLong();
			progress = 0;

			runOnUiThread(() -> {
				ProgressBar progressBar = (ProgressBar) findViewById(R.id.download_progress);
				progressBar.setMax((int) max);
				progressBar.setProgress(0);
			});

			while ((length = in.read(buffer)) > 0) {
				progress += length;
				out.write(buffer, 0, length);
				runOnUiThread(() -> updateUI(progress));
			}
			
			out.close();
			in.close();
		} catch (MalformedURLException mue) {
			Log.e("Download", "Malformed url error", mue);
		} catch (IOException ioe) {
			Log.e("Download", "IO error", ioe);
		} catch (SecurityException se) {
			Log.e("Download", "Security error", se);
		}
	}
}
