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
import java.util.concurrent.atomic.AtomicInteger;
import java.util.zip.ZipEntry;
import java.util.zip.ZipException;
import java.util.zip.ZipFile;
import java.util.zip.ZipInputStream;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
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

	private final static LinkedHashMap<String, String> packages = new LinkedHashMap<String, String>();
	private final static String VERSION_ID = "1.19.14";
	private final static String ARCHIVE_URL =
		"https://sourceforge.net/projects/wesnoth/files/wesnoth/wesnoth-%s/android-data/%s/download";

	private File dataDir;

	private String toSizeString(long bytes) {
		return String.format("%4.2f MB", (bytes * 1.0f) / (1e6));
	}

	@Override
	protected void onCreate(Bundle savedState) {
		packages.put("Core Data", "master.zip");
		packages.put("Music", "music.zip");
		packages.put("Patch", "patch.zip");

		super.onCreate(savedState);
		setContentView(R.layout.activity_init);

		// Keep the screen on while this activity runs
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

		initMainDataDir();

		initSettingsMenu();

		doPowerCheckAndStart();
	}

	public void onActivityResult(int reqCode, int resCode, Intent intent) {
		// Start the asset download only after the user has returned from
		// battery save settings, if they already went there via the
		// AlertDialog in onCreate().
		if (reqCode == 1) {
			initialize();
		} else if (reqCode == 2 && resCode == RESULT_OK) {
			initializeAssetsFromZip(intent.getData());
		}
	}

	// Create the settings menu ui
	private void initSettingsMenu() {
		ImageButton btnSettings = findViewById(R.id.settings_btn);
		btnSettings.setOnClickListener(e -> {
			PopupMenu settingsMenu = new PopupMenu(InitActivity.this, btnSettings);
			settingsMenu.getMenuInflater().inflate(R.menu.main_menu, settingsMenu.getMenu());
			settingsMenu.setOnMenuItemClickListener(menuItem -> {
				if (menuItem.getItemId() == R.id.mnuClear) {
					// TODO do the deleting in another thread
					clearGameData(dataDir);
					return true;
				} else if (menuItem.getItemId() == R.id.mnuLocalInstall) {
					showZIPHelpDialog();
					return true;
				}
				return false;
			});
			settingsMenu.show();
		});
	}

	// Note: wrap in runOnUiThread(()-> {...}) if called from another thread
	private void showLaunchScreen() {
		findViewById(R.id.download_progress).setVisibility(View.INVISIBLE);
		findViewById(R.id.download_msg).setVisibility(View.INVISIBLE);
		TextView lblTap = findViewById(R.id.tap_label);
		lblTap.setText("Tap to Start");
		lblTap.startAnimation(AnimationUtils.loadAnimation(this, R.anim.fade));
	}

	// Note: wrap in runOnUiThread(()-> {...}) if called from another thread
	private void showProgressScreen() {
		TextView lblTap = findViewById(R.id.tap_label);
		lblTap.clearAnimation();
		lblTap.setVisibility(View.INVISIBLE);
		findViewById(R.id.download_msg).setVisibility(View.VISIBLE);
		findViewById(R.id.download_progress).setVisibility(View.VISIBLE);
	}

	// Check if battery saver is on, and prompt to turn off
	// then start the main task
	private void doPowerCheckAndStart() {
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

	// Initialize gamedata directory
	private void initMainDataDir() {
		dataDir = new File(getExternalFilesDir(null), "gamedata");
		if (!dataDir.exists()) {
			dataDir.mkdir();
		}
		Log.d("InitActivity", "Creating " + dataDir);
	}

	private void initialize() {
		runOnUiThread(()-> {
			showLaunchScreen();
			findViewById(R.id.screen).setOnClickListener(e -> initializeAssets());
		});
	}

	private void initializeAssets() {
		findViewById(R.id.screen).setOnClickListener(null);
		showProgressScreen();
		TextView progressText = findViewById(R.id.download_msg);
		progressText.setText("Connecting...");

		Executors.newSingleThreadExecutor().execute(() -> {
			//TODO Update mechanism when patch is available.

			Properties status = initStatusFile(dataDir);
			boolean isManual = Boolean.parseBoolean(status.getProperty("manual_install", "false"));

			if (!isManual) {
				for (Map.Entry<String, String> entry : InitActivity.packages.entrySet()) {
					String name = entry.getValue();
					String uiname = entry.getKey();

					File packageFile = new File(dataDir, name);
					long lastModified = Long.parseLong(status.getProperty("modified." + name, "0"));

					// Download file
					if (status.getProperty("unpack." + name, "false").equalsIgnoreCase("false")) {
						final String downloadAddr = String.format(ARCHIVE_URL, VERSION_ID, name);
						Log.d("InitActivity", "Starting to download " + name + " from " + downloadAddr);
						try {
							lastModified = downloadFile(
								downloadAddr,
								packageFile,
								uiname,
								lastModified);

							status.setProperty("modified." + name, "" + lastModified);
						} catch (Exception e) {
							Log.e("Download", "security error", e);
						}

						// Unpack archive
						// TODO Checksum verification?
						if (packageFile.exists()) {
							Log.d("InitActivity", "Start unpack " + name);

							if (unpackArchive(packageFile, dataDir, uiname)) {
								packageFile.delete();
								status.setProperty("unpack." + name, "true");
							}
						}
					}
				}
			}

			extractNetworkCertificate();

			storeStatus(status);

			runOnUiThread(() -> progressText.setText("Unpacking finished..."));

			Log.d("InitActivity", "Stop unpack");

			// Launch Wesnoth
			runOnUiThread(() -> launchWesnoth());
		});
	}

	private void launchWesnoth() {
		getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		TextView progressText = findViewById(R.id.download_msg);
		progressText.setText("Launching Wesnoth...");
		Log.d("InitActivity", "Launch wesnoth");
		Intent launchIntent = new Intent(this, WesnothActivity.class);
		launchIntent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
		startActivity(launchIntent);
		finish();
	}

	private Properties initStatusFile(File dataDir) {
		Properties status = new Properties();
		File statusFile = new File(dataDir, "status.properties");
		try (FileInputStream statusStream = new FileInputStream(statusFile)) {
			if (statusFile.exists()) {
				status.load(statusStream);
			} else {
				statusFile.createNewFile();
			}
		} catch (IOException ioe) {
			Log.e("InitActivity", "IO exception", ioe);
		}
		return status;
	}

	private void storeStatus(Properties status) {
		File statusFile = new File(dataDir, "status.properties");
		try (FileOutputStream statusStream = new FileOutputStream(statusFile)) {
			status.store(statusStream, "Wesnoth Assets Status");
		} catch (IOException ioe) {
			Log.e("InitActivity", "IO exception", ioe);
		}
	}

	// Extract certificate file from apk raw resource
	private void extractNetworkCertificate() {
		// TODO update mechanism for this file
		File certDir = new File(dataDir, "certificates");
		if (!certDir.exists()) {
			certDir.mkdir();
		}
		File certFile = new File(certDir, "cacert.pem");
		if (certFile.exists()) {
			return;
		}

		try (FileOutputStream certStream = new FileOutputStream(certFile)) {
			certFile.createNewFile();
			copyStream(getResources().openRawResource(R.raw.cacert), certStream);
		} catch (Exception e) {
			Log.e("InitActivity", "Exception", e);
		}
	}

	private void initializeAssetsFromZip(Uri uri) {
		Executors.newSingleThreadExecutor().execute(() -> {
			runOnUiThread(() -> showProgressScreen());

			Properties status = initStatusFile(dataDir);

			if (unpackArchive(uri, dataDir, "Core")) {
				status.setProperty("manual_install", "true");
				storeStatus(status);
				runOnUiThread(()-> Toast.makeText(this, "Installed!", Toast.LENGTH_SHORT).show());
			} else {
				runOnUiThread(()-> Toast.makeText(this, "Installation failed!", Toast.LENGTH_SHORT).show());
			}

			runOnUiThread(() -> recreate());
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
					recreate();
				} catch (IOException ioe) {
					Log.e("InitActivity", "IO exception", ioe);
					Toast.makeText(this, "Failed!", Toast.LENGTH_SHORT).show();
				}
			})
			.setNegativeButton("No", null)
			.setCancelable(false)
			.show();
	}

	private void showZIPHelpDialog() {
		new AlertDialog.Builder(this)
			.setTitle("Manual Install Guide")
			.setMessage("The ZIP file should contain data, fonts, images, sounds and translations folders from a Wesnoth PC installation. This also disables automatic updates until you clear the game data.")
			.setPositiveButton("Proceed", (dialog, which) -> openDataFile())
			.setNegativeButton("Cancel", null)
			.setCancelable(false)
			.show();
	}

	// Show a file chooser to open a file
	private void openDataFile() {
		Intent inttOpen = new Intent(Intent.ACTION_GET_CONTENT);
		inttOpen.addCategory(Intent.CATEGORY_OPENABLE);
		inttOpen.setType("application/zip");
		inttOpen.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, false);
		Intent inttOpen2 = Intent.createChooser(inttOpen, "Open ZIP file...");
		startActivityForResult(inttOpen2, 2);
	}

	private void updateDownloadProgress(int progress, int max, String type) {
		TextView progressText = (TextView) findViewById(R.id.download_msg);
		ProgressBar progressBar = (ProgressBar) findViewById(R.id.download_progress);
		progressBar.setProgress(progress);
		progressText.setText("Downloading " + type + " ... (" + toSizeString(progress) + "/" + toSizeString(max) + ")");
	}

	private void updateUnpackProgress(int progress, int max, String type) {
		TextView progressText = (TextView) findViewById(R.id.download_msg);
		ProgressBar progressBar = (ProgressBar) findViewById(R.id.download_progress);
		progressBar.setProgress(progress);
		// progress starts from 0 but asset counting starts from 1.
		// also, when installing from zip the total number of files is
		// not available, so don't show it in that case.
		if (max > 0) {
			progressText.setText("Unpacking " + type + " assets... (" + (progress+1) + "/" + max + ")");
		} else {
			progressText.setText("Unpacking " + type + " assets... (" + (progress+1) + ")");
		}
	}

	private void copyStreamNoClose(InputStream in, OutputStream out) throws IOException {
		byte[] buffer = new byte[8192];
		int length;
		while ((length = in.read(buffer)) > 0) {
			out.write(buffer, 0, length);
		}
	}

	private void copyStream(InputStream in, OutputStream out) throws IOException {
		copyStreamNoClose(in, out);
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

			final int response = conn.getResponseCode();
			if (response != HttpURLConnection.HTTP_OK) {
				Log.e("Download", "Server returned response: " + response);
				return newModified;
			}

			final int max = (int) conn.getContentLengthLong();
			final AtomicInteger progress = new AtomicInteger(0);
			final AtomicInteger length = new AtomicInteger(0);
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
				progressBar.setMax(max);
				progressBar.setProgress(0);
			});

			length.set(in.read(buffer));
			while (length.get() > 0) {
				out.write(buffer, 0, length.get());
				runOnUiThread(() -> updateDownloadProgress(progress.addAndGet(length.get()), max, type));
				length.set(in.read(buffer));
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

	private boolean unpackArchive(Uri uri, File destdir, String type) {
		Log.d("Unpack", "Start");

		InputStream zipstream = null;
		try {
			zipstream = getContentResolver().openInputStream(uri);
		} catch (FileNotFoundException fe) {
			Log.e("Unpack", "File not found exception", fe);
			return false;
		}

		try (ZipInputStream zf = new ZipInputStream(getContentResolver().openInputStream(uri))) {
			AtomicInteger progress = new AtomicInteger(1);

			runOnUiThread(() -> ((ProgressBar) findViewById(R.id.download_progress)).setIndeterminate(true));

			ZipEntry ze;
			while ((ze = zf.getNextEntry()) != null) {
				runOnUiThread(() -> updateUnpackProgress(progress.get(), 0, type));

				if (ze.isDirectory()) {
					File dir = new File(destdir, ze.getName());
					if (!dir.exists()) {
						dir.mkdir();
					}
				} else {
					FileOutputStream out = new FileOutputStream(new File(destdir, ze.getName()));
					copyStreamNoClose(zf, out);
					out.close();
				}

				Log.d("Unpack", "Unpacking " + type + ":" + progress.get());
				progress.incrementAndGet();
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

	private boolean unpackArchive(File zipfile, File destdir, String type) {
		Log.d("Unpack", "Start");

		if (zipfile == null) {
			Log.e("Unpack", "File for " + type + " is null!");
			return false;
		}

		try (ZipFile zf = new ZipFile(zipfile)) {
			Enumeration<? extends ZipEntry> e = zf.entries();

			AtomicInteger progress = new AtomicInteger(1);
			final int max = zf.size();

			runOnUiThread(() -> {
				ProgressBar progressBar = (ProgressBar) findViewById(R.id.download_progress);
				progressBar.setMax((int) max);
				progressBar.setProgress(0);
			});

			while (e.hasMoreElements()) {
				ZipEntry ze = (ZipEntry) e.nextElement();

				runOnUiThread(() -> updateUnpackProgress(progress.get(), max, type));

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

				Log.d("Unpack", "Unpacking " + type + ":" + progress.get() + "/" + max);
				progress.incrementAndGet();
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
