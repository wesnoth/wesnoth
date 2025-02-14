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

	private final String name = "master.zip";
	private final String[] packages = {"master.zip", "music.zip"};
	private final String archiveURL =
		"https://sourceforge.net/projects/wesnoth/files/android/" + name + "/download";

	private String dataDir;
	private String archivePath;

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
		Log.e("Initialize", dataDir);

		// Download file
		File f = new File(dataDir + "/" + name);
		TextView progressText = (TextView) findViewById(R.id.download_msg);
		progressText.setText("Connecting...");
		if (!f.exists()) {
			Thread threadDnld = new Thread(new Runnable() {
				@Override
				public void run() {
					try {
						downloadFile(archiveURL, dataDir + "/" + name);
					} catch (Exception e) {
						Log.e("Download", "security error", e);
					}
				}
			});

			threadDnld.start();
		}

		// Unpack archive
		if (f.exists()) {
			progressText.setText("Unpacking assets...");
			Thread threadUnpk = new Thread(new Runnable() {
				@Override
				public void run() {
					unpackArchive(dataDir + "/" + name, dataDir);
				}
			});
			
			threadUnpk.start();
			progressText.setText("Unpacking finished...");
		}
		
		// Launch Wesnoth
		// TODO: check data existence
		progressText.setText("Launching Wesnoth...");
		// TODO: launch WesnothActivity
	}

	private void launchWesnothActivity() {
		Intent launchIntent = new Intent(InitActivity.this, WesnothActivity.class);
		startActivity(launchIntent);
	}

	private void updateDownloadProgress(int progress) {
		TextView progressText = (TextView) findViewById(R.id.download_msg);
		ProgressBar progressBar = (ProgressBar) findViewById(R.id.download_progress);
		progressBar.setProgress(progress);
		progressText.setText("Downloading game data... (" + toSizeString(progress) + "/" + toSizeString(max) + ")");
	}
	
	private void updateUnpackProgress(int progress) {
		TextView progressText = (TextView) findViewById(R.id.download_msg);
		ProgressBar progressBar = (ProgressBar) findViewById(R.id.download_progress);
		progressBar.setProgress(progress);
		// progress starts from 0 but asset counting starts from 1.
		progressText.setText("Unpacking assets... (" + (progress+1) + "/" + max + ")");
	}

	private void downloadFile(String url, String destpath) {
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
				runOnUiThread(() -> updateDownloadProgress(progress));
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
	}

	private void unpackArchive(String path, String destdir) {
		File sFile = new File(path);
		if (sFile != null) {
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
					
					runOnUiThread(() -> updateUnpackProgress(progress));
					
					if (ze.isDirectory()) {
						File dir = new File(destdir + "/" + ze.getName());
						if (!dir.exists()) {
							dir.mkdir();
						}
					} else {
						InputStream in = zf.getInputStream(ze);
						OutputStream out = new FileOutputStream(new File(destdir + "/" + ze.getName()));
						byte[] buffer = new byte[4096];
						int length;
						while ((length = in.read(buffer)) > 0) {
							out.write(buffer, 0, length);
						}
						out.close();
						in.close();
					}
					
					progress++;
				}
				zf.close();
			} catch (ZipException e) {
				Log.e("Unpack", "ZIP exception", e);
			} catch (FileNotFoundException e) {
				Log.e("Unpack", "File not found", e);
			} catch (IOException e) {
				Log.e("Unpack", "IO exception", e);
			}
		}
	}

}
