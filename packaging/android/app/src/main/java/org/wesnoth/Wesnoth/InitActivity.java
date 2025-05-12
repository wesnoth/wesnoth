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
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Comparator;
import java.util.Enumeration;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Properties;
import java.util.concurrent.Executors;
import java.util.zip.ZipEntry;
import java.util.zip.ZipException;
import java.util.zip.ZipFile;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.PowerManager;
import android.provider.Settings;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.view.animation.AnimationUtils;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.PopupMenu;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

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

	@Override
	protected void onCreate(Bundle savedState) {
		super.onCreate(savedState);
		setContentView(R.layout.activity_init);
		// Keep the screen on while this task runs
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		
		// Initialize gamedata directory
		dataDir = new File(getExternalFilesDir(null), "gamedata");
		
		// Settings menu
		ImageButton btnSettings = findViewById(R.id.settings_btn);
		btnSettings.setOnClickListener(e -> {
			PopupMenu settingsMenu = new PopupMenu(InitActivity.this, btnSettings);
			settingsMenu.getMenuInflater().inflate(R.menu.main_menu, settingsMenu.getMenu());
			settingsMenu.setOnMenuItemClickListener(menuItem -> {
				// TODO show an alert dialog to ask the user first
				if (menuItem.getItemId() == R.id.mnuClear) {
					clearGameData(dataDir);
					return true;
				} else if (menuItem.getItemId() == R.id.mnuLocalInstall) {
					openFile();
					return true;
				}
				// TODO implement other menu items
				return false;
			});
			settingsMenu.show();
		});
		
		// Battery saver check
		PowerManager powerManager = (PowerManager) getSystemService(Context.POWER_SERVICE);
		if (powerManager.isPowerSaveMode()) {
			new AlertDialog.Builder(this)
				.setTitle("Power Saver Detected")
				.setMessage("Battery Saver is on. Data download may be interrupted. Consider whitelisting this app from battery saver or turning it off.")
				// onActivityResult will be called (with reqCode = 1)
				// after this intent finishes, that is,
				// the user returns from Battery Saver settings
				.setPositiveButton("Settings", (dialog, which) -> startActivityForResult(new Intent(Settings.ACTION_BATTERY_SAVER_SETTINGS), 1))
				.setNegativeButton("Ignore", (dialog, which) -> initialize())
				.setCancelable(false)
				.show();
		} else {
			initialize();
		}
	}
	
	public void onActivityResult(int reqCode, int resCode, Intent intent) {
		// Start the asset download only after the user has returned from
		// battery save settings, if they already went there via the
		// AlertDialog in onCreate().
		if (reqCode == 1) {
			initialize();
		} else if (reqCode == 2 && resCode == RESULT_OK) {
			Toast.makeText(this, "Installing ZIP...", Toast.LENGTH_SHORT).show();
			if (unpackArchive(new File(intent.getData().getPath()), dataDir, "Core")) {
				Toast.makeText(this, "Installed!", Toast.LENGTH_SHORT).show();
			} else {
				Toast.makeText(this, "Installation failed!", Toast.LENGTH_SHORT).show();
			}
		}
	}
	
	private void initialize() {
		runOnUiThread(()-> {
			findViewById(R.id.download_progress).setVisibility(View.INVISIBLE);
			findViewById(R.id.download_msg).setVisibility(View.INVISIBLE);
			TextView lblTap = (TextView) findViewById(R.id.tap_label);
			lblTap.setText("Tap to Start");
			lblTap.startAnimation(AnimationUtils.loadAnimation(this, R.anim.fade));
			findViewById(R.id.screen).setOnClickListener(e -> initializeAssets());
		});
	}
	
	private void initializeAssets() {
		if (!dataDir.exists()) {
			dataDir.mkdir();
		}
		Log.d("InitActivity", "Creating " + dataDir);
		
		TextView lblTap = findViewById(R.id.tap_label);
		lblTap.clearAnimation();
		lblTap.setVisibility(View.INVISIBLE);

		TextView progressText = (TextView) findViewById(R.id.download_msg);
		progressText.setVisibility(View.VISIBLE);
		progressText.setText("Connecting...");
		findViewById(R.id.download_progress).setVisibility(View.VISIBLE);
		
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

						if (unpackArchive(f, dataDir, uiname)) {
							f.delete();
							status.setProperty("unpack." + name, "true");
						}
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
					copyStream(
						getResources().openRawResource(R.raw.cacert),
						new FileOutputStream(certFile));
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
			getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
			
			progressText.setText("Launching Wesnoth...");
			Log.d("InitActivity", "Launch wesnoth");
			Intent launchIntent = new Intent(this, WesnothActivity.class);
			launchIntent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
			startActivity(launchIntent);
			finish();
		});
	}
	
	private void clearGameData(File dataDir) {
		new AlertDialog.Builder(this)
			.setTitle("Confirm Deletion")
			.setMessage("All gamedata will be completely deleted. Are you sure?")
			.setPositiveButton("Yes", (dialog, which) -> {
				Toast.makeText(this, "Clearing data...", Toast.LENGTH_SHORT).show();
				try {
					Files.walk(dataDir.toPath())
						.sorted(Comparator.reverseOrder())
						.map(Path::toFile)
						.forEach(File::delete);
					Toast.makeText(this, "Cleared!", Toast.LENGTH_SHORT).show();
				} catch (IOException ioe) {
					Log.e("InitActivity", "IO exception", ioe);
					Toast.makeText(this, "Failed!", Toast.LENGTH_SHORT).show();
				}
			})
			.setNegativeButton("No", null)
			.setCancelable(false)
			.show();
	}
	
	// Show a file chooser to open a file
	private void openFile() {
		Intent inttOpen = new Intent(Intent.ACTION_GET_CONTENT);
		inttOpen.addCategory(Intent.CATEGORY_OPENABLE);
		inttOpen.setType("application/zip");
		inttOpen.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, false);
		Intent inttOpen2 = Intent.createChooser(inttOpen, "Open ZIP file...");
		startActivityForResult(inttOpen2, 2);
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
	
	private void copyStream(InputStream in, OutputStream out) throws IOException {
		byte[] buffer = new byte[8192];
		int length;
		while ((length = in.read(buffer)) > 0) {
			out.write(buffer, 0, length);
		}
		out.close();
		in.close();
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

			// TODO rewrite to use copyStream function.
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
			
			Log.d("Download", "Download success from URL: " + url);
			return newModified;
			
		} catch (MalformedURLException mue) {
			Log.e("Download", "Malformed url exception", mue);
		} catch (IOException ioe) {
			Log.e("Download", "IO exception", ioe);
		} catch (SecurityException se) {
			Log.e("Download", "Security exception", se);
		}
		
		return 0;
	}

	private boolean unpackArchive(File zipfile, File destdir, String type) {
		Log.d("Unpack", "Start");

		if (zipfile == null) {
			Log.e("Unpack", "File for " + type + " is null!");
			return false;
		}

		try (ZipFile zf = new ZipFile(zipfile)) {
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
					File dir = new File(destdir, ze.getName());
					if (!dir.exists()) {
						dir.mkdir();
					}
				} else {
					copyStream(
						zf.getInputStream(ze),
						new FileOutputStream(new File(destdir, ze.getName())));
				}

				Log.d("Unpack", "Unpacking " + type + ":" + progress + "/" + max);
				progress++;
			}
			
			Log.d("Unpack", "Done unpacking " + type);
			return true;
		} catch (ZipException e) {
			Log.e("Unpack", "ZIP exception", e);
		} catch (FileNotFoundException e) {
			Log.e("Unpack", "File not found", e);
		} catch (IOException e) {
			Log.e("Unpack", "IO exception", e);
		}
		
		return false;
	}

}
