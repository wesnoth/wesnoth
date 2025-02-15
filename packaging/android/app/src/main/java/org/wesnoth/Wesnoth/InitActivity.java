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
import java.io.FileNotFoundException;
import java.io.InputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.net.URL;
import java.net.MalformedURLException;
import java.net.HttpURLConnection;
import java.util.Enumeration;
import java.util.Map;
import java.util.HashMap;
import java.util.concurrent.Executors;
import java.util.zip.ZipEntry;
import java.util.zip.ZipException;
import java.util.zip.ZipFile;
import java.util.zip.ZipOutputStream;

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

	private static HashMap<String, String> packages = new HashMap<String, String>();
	private String archiveURL = "https://sourceforge.net/projects/wesnoth/files/android/%s/download";

	private String dataDir;
	private String archivePath;
	
	static {
		packages.put("Core", "master.zip");
		packages.put("Music", "music.zip");
	}

	private String toSizeString(long bytes) {
		return String.format("%4.2f MB", (bytes * 1.0f) / (1e6));
	}

	@Override
	protected void onCreate(Bundle savedState) {
		super.onCreate(savedState);
		this.setContentView(R.layout.activity_init);

		// Initialize gamedata directory
		dataDir = getExternalFilesDir(null) + "/gamedata";
		File fDataDir = new File(dataDir);
		if (!fDataDir.exists()) {
			fDataDir.mkdir();
		}
		Log.d("InitActivity", "Creating " + dataDir);

		TextView progressText = (TextView) findViewById(R.id.download_msg);
		progressText.setText("Connecting...");
		
		Executors.newSingleThreadExecutor().execute(() -> {
			//TODO record if the data files have been downloaded and unpacked successfully.
			//Only download if 1. previous download failed, 2. new version/patch is available.
			
			for (Map.Entry<String, String> entry : InitActivity.packages.entrySet()) {
				String uiname = entry.getKey();
				String name = entry.getValue();
				
				File f = new File(dataDir + "/" + name);
				
				// Download file
				if (!f.exists()) {
					Log.d("InitActivity", "Start download " + name);
					try {
						downloadFile(
							String.format(archiveURL, name),
							dataDir + "/" + name,
							uiname
						);
					} catch (Exception e) {
						Log.e("Download", "security error", e);
					}
				}
				
				// Unpack archive
				Log.d("InitActivity", "Start unpack " + name);
				
				unpackArchive(dataDir + "/" + name, dataDir, uiname);
				f.delete();
			}
			
			runOnUiThread(() -> progressText.setText("Unpacking finished..."));
			Log.d("InitActivity", "Stop unpack");
			
			// Launch Wesnoth
			// TODO: check data existence
			runOnUiThread(() -> progressText.setText("Launching Wesnoth..."));
			Log.d("InitActivity", "Launch wesnoth");
			runOnUiThread(() -> {
				Intent launchIntent = new Intent(this, WesnothActivity.class);
				launchIntent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
				startActivity(launchIntent);
				finish();
			});
		});
	}

	private void launchWesnothActivity() {
		Intent launchIntent = new Intent(InitActivity.this, WesnothActivity.class);
		startActivity(launchIntent);
	}

	private void updateDownloadProgress(int progress, String type) {
		TextView progressText = (TextView) findViewById(R.id.download_msg);
		ProgressBar progressBar = (ProgressBar) findViewById(R.id.download_progress);
		progressBar.setProgress(progress);
		progressText.setText("Downloading " + type + " data... (" + toSizeString(progress) + "/" + toSizeString(max) + ")");
	}
	
	private void updateUnpackProgress(int progress, String type) {
		TextView progressText = (TextView) findViewById(R.id.download_msg);
		ProgressBar progressBar = (ProgressBar) findViewById(R.id.download_progress);
		progressBar.setProgress(progress);
		// progress starts from 0 but asset counting starts from 1.
		progressText.setText("Unpacking " + type + " assets... (" + (progress+1) + "/" + max + ")");
	}

	private void downloadFile(String url, String destpath, String type) {
		// based on https://stackoverflow.com/a/4896527/22060628
		Log.d("Download", "URL: " + url);
		
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
				Log.e("Download", "Server returned response: " + response);
				return;
			}

			DataInputStream in = new DataInputStream(conn.getInputStream());
			OutputStream out = new FileOutputStream(new File(destpath));

			byte[] buffer = new byte[8192];

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
				runOnUiThread(() -> updateDownloadProgress(progress, type));
			}

			out.close();
			in.close();
		} catch (MalformedURLException mue) {
			Log.e("Download", "Malformed url exception", mue);
		} catch (IOException ioe) {
			Log.e("Download", "IO exception", ioe);
		} catch (SecurityException se) {
			Log.e("Download", "Security exception", se);
		}
		
		Log.d("Download", "Download success from URL: " + url);
	}

	private void unpackArchive(String path, String destdir, String type) {
		File sFile = new File(path);
		Log.d("Unpack", "Start");
		
		if (sFile == null) {
			Log.e("Unpack", "File for " + type + " is null!");
			return;
		}
		
		try {
			ZipFile zf = new ZipFile(sFile);
			Enumeration<? extends ZipEntry> e = zf.entries();
			
			progress = 0;
			max = zf.size();
			
			runOnUiThread(() -> {
				ProgressBar progressBar = (ProgressBar) findViewById(R.id.download_progress);
				progressBar.setMax((int) max);
				progressBar.setProgress(0);
			});
			
			while (e.hasMoreElements()) {
				ZipEntry ze = (ZipEntry) e.nextElement();
				
				runOnUiThread(() -> updateUnpackProgress(progress, type));
				
				if (ze.isDirectory()) {
					File dir = new File(destdir + "/" + ze.getName());
					if (!dir.exists()) {
						dir.mkdir();
					}
				} else {
					InputStream in = zf.getInputStream(ze);
					OutputStream out = new FileOutputStream(new File(destdir + "/" + ze.getName()));
					byte[] buffer = new byte[8192];
					int length;
					while ((length = in.read(buffer)) > 0) {
						out.write(buffer, 0, length);
					}
					out.close();
					in.close();
				}
				
				Log.d("Unpack", "Unpacking " + type + ":" + progress + "/" + max);
				progress++;
			}
			Log.d("Unpack", "Done unpacking " + type);
			zf.close();
		} catch (ZipException e) {
			Log.e("Unpack", "ZIP exception", e);
		} catch (FileNotFoundException e) {
			Log.e("Unpack", "File not found", e);
		} catch (IOException e) {
			Log.e("Unpack", "IO exception", e);
		} finally {
			Log.e("Unpack", "Done (finally)");
		}
		
		Log.d("Unpack", "Exit function!");
	}

}
