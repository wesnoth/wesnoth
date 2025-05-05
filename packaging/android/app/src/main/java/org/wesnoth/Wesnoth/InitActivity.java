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
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.Enumeration;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Properties;
import java.util.concurrent.Executors;
import java.util.zip.ZipEntry;
import java.util.zip.ZipException;
import java.util.zip.ZipFile;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.animation.AnimationUtils;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.PopupMenu;
import android.widget.ProgressBar;
import android.widget.TextView;

public class InitActivity extends Activity {
	private int length = 0;
	private int progress = 0;
	private long max = 0;

	private final static LinkedHashMap<String, String> packages = new LinkedHashMap<String, String>();
	private final String archiveURL =
		"https://sourceforge.net/projects/wesnoth/files/android/%s/download";

	private File dataDir;

	static {
		packages.put("Core Data", "master.zip");
		packages.put("Music", "music.zip");
		packages.put("Patch", "patch.zip");
	}

	private String toSizeString(long bytes) {
		return String.format("%4.2f MB", (bytes * 1.0f) / (1e6));
	}
	
	private boolean isUnpacked(File dataDir) {
		return (new File(dataDir, "data").exists()
			    && new File(dataDir, "fonts").exists()
			    && new File(dataDir, "images").exists()
			    && new File(dataDir, "sounds").exists());
	}

	@Override
	protected void onCreate(Bundle savedState) {
		super.onCreate(savedState);
		this.setContentView(R.layout.activity_init);
		
		ImageButton btnSettings = findViewById(R.id.settings_btn);
		btnSettings.setOnClickListener(e -> {
			PopupMenu settingsMenu = new PopupMenu(InitActivity.this, btnSettings);
			settingsMenu.getMenuInflater().inflate(R.menu.main_menu, settingsMenu.getMenu());
			settingsMenu.setOnMenuItemClickListener(menuItem -> {
				return true;
			});
			settingsMenu.show();
		});

		// Initialize gamedata directory
		dataDir = new File(getExternalFilesDir(null), "gamedata");
		if (!dataDir.exists()) {
			dataDir.mkdir();
		}
		Log.d("InitActivity", "Creating " + dataDir);

		TextView progressText = (TextView) findViewById(R.id.download_msg);
		progressText.setText("Connecting...");

		Executors.newSingleThreadExecutor().execute(() -> {
			//TODO Update mechanism when patch is available.

			Properties status = new Properties();
			File statusFile = new File(dataDir, "status.properties");
			try {
				if (statusFile.exists()) {
					status.load(new FileInputStream(statusFile));
				} else {
					statusFile.createNewFile();
				}
			} catch (IOException ioe) {
				Log.e("InitActivity", "IO exception", ioe);
			}

			for (Map.Entry<String, String> entry : InitActivity.packages.entrySet()) {
				String name = entry.getValue();
				String uiname = entry.getKey();
	
				File f = new File(dataDir, name);
				long lastModified = Long.parseLong(status.getProperty("modified." + name, "0"));

				// Download file
				if (status.getProperty("unpack." + name, "false").equalsIgnoreCase("false")) {
					Log.d("InitActivity", "Start download " + name);
					try {
						lastModified = downloadFile(
							String.format(archiveURL, name),
							f,
							uiname,
							lastModified);
						
						status.setProperty("modified." + name, "" + lastModified);
					} catch (Exception e) {
						Log.e("Download", "security error", e);
					}

					// Unpack archive
					// TODO Checksum verification?
					if (f.exists()) {
						Log.d("InitActivity", "Start unpack " + name);

						unpackArchive(f, dataDir, uiname);
						f.delete();

						status.setProperty("unpack." + name, "true");
					}
				}
			}

			// Extract certificates file
			try {
				File certDir = new File(dataDir, "certificates");
				if (!certDir.exists()) {
					certDir.mkdir();
				}
				File certFile = new File(certDir, "cacert.pem");
				if (!certFile.exists()) {
					certFile.createNewFile();
					InputStream cin = getResources().openRawResource(R.raw.cacert);
					OutputStream cout = new FileOutputStream(certFile);
					byte[] buffer = new byte[8192];
					int length;
					while ((length = cin.read(buffer)) > 0) {
						cout.write(buffer, 0, length);
					}
					cout.close();
					cin.close();

				}
			} catch (Exception e) {
				Log.e("InitActivity", "Exception", e);
			}

			runOnUiThread(() -> progressText.setText("Unpacking finished..."));
			try {
				status.store(new FileOutputStream(statusFile), "Wesnoth Assets Status");
			} catch (IOException ioe) {
				Log.e("InitActivity", "IO exception", ioe);
			}

			Log.d("InitActivity", "Stop unpack");

			// Launch Wesnoth
			// TODO: check data existence
			runOnUiThread(()-> {
				ProgressBar progressBar = (ProgressBar) findViewById(R.id.download_progress);
				progressBar.setVisibility(View.INVISIBLE);
				progressText.setVisibility(View.INVISIBLE);
				Button btnMain = (Button) findViewById(R.id.main_btn);
				btnMain.setText("Launch");
				btnMain.startAnimation(AnimationUtils.loadAnimation(this, R.anim.fade));
				btnMain.setOnClickListener(e -> {
					progressText.setText("Launching Wesnoth...");
					Log.d("InitActivity", "Launch wesnoth");
					Intent launchIntent = new Intent(this, WesnothActivity.class);
					launchIntent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
					startActivity(launchIntent);
					finish();
				});
			});
		});
	}

	private void updateDownloadProgress(int progress, String type) {
		TextView progressText = (TextView) findViewById(R.id.download_msg);
		ProgressBar progressBar = (ProgressBar) findViewById(R.id.download_progress);
		progressBar.setProgress(progress);
		progressText.setText("Downloading " + type + " ... (" + toSizeString(progress) + "/" + toSizeString(max) + ")");
	}

	private void updateUnpackProgress(int progress, String type) {
		TextView progressText = (TextView) findViewById(R.id.download_msg);
		ProgressBar progressBar = (ProgressBar) findViewById(R.id.download_progress);
		progressBar.setProgress(progress);
		// progress starts from 0 but asset counting starts from 1.
		progressText.setText("Unpacking " + type + " assets... (" + (progress+1) + "/" + max + ")");
	}

	private long downloadFile(String url, File destpath, String type, long modified) {
		long newModified = 0;
		// based on https://stackoverflow.com/a/4896527/22060628
		Log.d("Download", "URL: " + url);

		try {
			URL downloadURL = new URL(url);
			HttpURLConnection conn = (HttpURLConnection) downloadURL.openConnection();
			conn.setRequestProperty("Accept", "*/*");
			conn.setRequestProperty("User-Agent", "Wget/1.13.4 (linux-gnu)");
			conn.setRequestMethod("GET");
			conn.setConnectTimeout(10000); // 10 seconds
			conn.setReadTimeout(10000);    // 10 seconds

			int response = conn.getResponseCode();

			if (response != HttpURLConnection.HTTP_OK) {
				Log.e("Download", "Server returned response: " + response);
				return newModified;
			}

			max = conn.getContentLengthLong();
			progress = 0;
			newModified = conn.getLastModified();
			// File did not change on server, don't download.
			if (newModified == modified) {
				return newModified;
			}

			DataInputStream in = new DataInputStream(conn.getInputStream());
			OutputStream out = new FileOutputStream(destpath);

			byte[] buffer = new byte[8192];

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
		return newModified;
	}

	private void unpackArchive(File zipfile, File destdir, String type) {
		Log.d("Unpack", "Start");

		if (zipfile == null) {
			Log.e("Unpack", "File for " + type + " is null!");
			return;
		}

		try {
			ZipFile zf = new ZipFile(zipfile);
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
