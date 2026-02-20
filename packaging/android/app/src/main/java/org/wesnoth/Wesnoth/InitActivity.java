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

import java.io.BufferedReader;
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
import java.util.ArrayList;
import java.util.Comparator;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.List;
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
import android.widget.ImageButton;
import android.widget.PopupMenu;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;
import androidx.documentfile.provider.DocumentFile;

public class InitActivity extends Activity {
	private final static String MANIFEST_URL =
		"https://sourceforge.net/projects/wesnoth/files/wesnoth/wesnoth-%s/android-data/manifest.txt/download";

	private File dataDir;
	private Properties status = new Properties();

	private String toSizeString(long bytes) {
		return String.format("%4.2f MB", (bytes * 1.0f) / (1e6));
	}

	@Override
	protected void onCreate(Bundle savedState) {
		super.onCreate(savedState);
		setContentView(R.layout.activity_init);

		// Keep the screen on while this activity runs
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

		initMainDataDir();
		
		status = initStatusFile(new File(dataDir, "status.properties"));

		initSettingsMenu();

		doPowerCheckAndStart();
	}

	private List<PackageInfo> readManifest() {
		List<PackageInfo> packages = new ArrayList<>();
		long lastModified = Long.parseLong(status.getProperty("manifest.modified", "0"));
		
		String versionID = BuildConfig.VERSION_NAME;
		// Delete '+dev', since SF data url doesn't have it.
		if (versionID.endsWith("+dev")) {
			versionID = versionID.substring(0, versionID.length() - 4);
		}

		String downloadAddr = String.format(MANIFEST_URL, versionID);
		Log.d("Manifest", "Fetching manifest from " + downloadAddr);
		File manifestFile = new File(dataDir, "manifest.txt");
		
		try {
			lastModified = downloadFile(downloadAddr, manifestFile, lastModified, "Checking Manifest...", true);

			Properties manifest = new Properties();
			manifest.load(new FileInputStream(manifestFile));
			for (String pkgid : manifest.getProperty("packages", "").split(",\\s*")) {
				packages.add(PackageInfo.from(manifest, pkgid));
			}
			Log.d("Manifest", "Last Modified: " + lastModified);
			Log.d("Manifest", "Packages: " + packages.toString());

			status.setProperty("manifest.modified", "" + lastModified);
			Log.d("Manifest", "Fetched and loaded successfully");
		} catch (Exception e) {
			Log.e("Download", "security error", e);
		}
		
		return packages;
	}

	public void onActivityResult(int reqCode, int resCode, Intent intent) {
		// Start the asset download only after the user has returned from
		// battery save settings, if they already went there via the
		// AlertDialog in onCreate().
		if (reqCode == 1) {
			initialize();
		} else if (reqCode == 2 && resCode == RESULT_OK) {
			initializeAssetsFromZip(intent.getData());
		} else if (reqCode == 3 && resCode == RESULT_OK) {
			importUserData(intent.getData());
		} else if (reqCode == 4 && resCode == RESULT_OK) {
			exportUserData(intent.getData());
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
					showClearDataDialog(dataDir);
					return true;
				} else if (menuItem.getItemId() == R.id.mnuLocalInstall) {
					showZIPHelpDialog();
					return true;
				} else if (menuItem.getItemId() == R.id.mnuImportExport) {
					showImportExportDialog();
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
			if (!Boolean.parseBoolean(status.getProperty("manual_install", "false"))) {
				HashMap<String, String> excluded = new HashMap<String, String>();
				
				for (PackageInfo info : readManifest()) {
					String id = info.getId();
					if (excluded.containsKey(id) && excluded.get(id).equals(info.getVersion())) {
						Log.d("InitActivity", "Not downloading excluded package " + id);
						continue;
					}
					
					String url = info.getURL();
					long lastModified = Long.parseLong(status.getProperty(id + ".modified", "0"));
					int oldVersion = PackageInfo.getPatchVersion(status.getProperty(id + ".version", "0"));
					int newVersion = info.getPatchVersion();
					boolean downloadPkg = newVersion > oldVersion;
					
					// Check if dependencies for this package exist, only download if all satisfied
					for (Map.Entry<String, String> dep : info.getDependencies().entrySet()) {
						int depOldVersion = PackageInfo.getPatchVersion(status.getProperty(dep.getKey() + ".version", "0"));
						int depNewVersion = PackageInfo.getPatchVersion(dep.getValue());
						if (depNewVersion != depOldVersion) {
							Log.d("InitActivity", "Dependency " + dep.getKey() + "for " + id + " not found");
							downloadPkg = false;
						}
					}
					
					Log.d("InitActivity", id + " version: " + oldVersion + " (local), " + newVersion + " (remote)");
					
					if (downloadPkg) {
						
						File packageFile = new File(dataDir, id + ".zip");
						
						// Download package
						Log.d("InitActivity", "Starting to download " + id + " from " + url);
						
						try {
							lastModified = downloadFile(
								url, packageFile, lastModified, info.getUIName(), false);
							
							status.setProperty(id + ".modified", "" + lastModified);
						} catch (Exception e) {
							Log.e("Download", "security error", e);
						}
	
						// Unpack archive
						// TODO Checksum verification?
						if (packageFile.exists()) {
							Log.d("InitActivity", "Start unpacking " + id);
							
							if (unpackArchive(packageFile, dataDir, info.getUIName())) {
								status.setProperty(id + ".version", "" + info.getVersion());
								// this package is already supplying what it excludes,
								// so mark excluded packages as installed
								for (Map.Entry<String, String> entry : info.getExcluded().entrySet()) {
									status.setProperty(entry.getKey() + ".version", entry.getValue().toString());
								}
								excluded.putAll(info.getExcluded());
								packageFile.delete();
							}
						}
					} else {
						Log.d("InitActivity", "No new version/dependency unmet for " + id + " found in server, skipping.");
					}
				}
			} else {
				Log.d("InitActivity", "Manually installed data, automatic updates will not be performed.");
			}

			extractNetworkCertificate();

			storeStatus(status);

			Log.d("InitActivity", "Stop unpack");

			if (new File(dataDir, "data").exists()
				&& new File(dataDir, "fonts").exists()
				&& new File(dataDir, "sounds").exists()
				&& new File(dataDir, "images").exists())
			{
				// Launch Wesnoth
				runOnUiThread(() -> launchWesnoth());
			} else {
				runOnUiThread(() -> {
					new AlertDialog.Builder(this)
						.setTitle("Data missing!")
						.setMessage("Gamedata is missing, please download it to proceed (requires network).")
						.setPositiveButton("OK", (d, res) -> initialize())
						.setNegativeButton("Exit", (d, res) -> System.exit(0))
						.setCancelable(false)
						.show();
				});
			}
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

	private Properties initStatusFile(File statusFile) {
		Properties status = new Properties();
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
			IOUtils.copyStream(getResources().openRawResource(R.raw.cacert), certStream);
		} catch (Exception e) {
			Log.e("InitActivity", "Exception", e);
		}
	}

	private void initializeAssetsFromZip(Uri uri) {
		Executors.newSingleThreadExecutor().execute(() -> {
			runOnUiThread(() -> showProgressScreen());

			initMainDataDir();
			
			status = initStatusFile(new File(dataDir, "status.properties"));
			
			String msg;
			if (unpackArchive(uri, dataDir, "Custom Data")) {
				status.setProperty("manual_install", "true");
				// if we have a custom status.properties bundled inside, merge it with `status`.
				status.putAll(initStatusFile(new File(dataDir, "status.properties")));
				storeStatus(status);
				msg = "Installed!";
			} else {
				msg = "Installation failed!";
			}

			runOnUiThread(() -> {
				runOnUiThread(()-> Toast.makeText(this, msg, Toast.LENGTH_SHORT).show());
				recreate();
			});
		});
	}

	private void showClearDataDialog(File dataDir) {
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
	
	private void showImportExportDialog() {
		new AlertDialog.Builder(this)
			.setTitle("Import/Export User Data")
			.setMessage("This allows you to import/export your userdata folder, which contains your add-ons, game saves, logs and so on. Intended for advanced users and UMC creators.")
			.setPositiveButton("Import", (dialog, which) ->
				// Open directory picker to select import destination
				startActivityForResult(new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE), 3)
			)
			.setNegativeButton("Export", (dialog, which) ->
				// Open directory picker to select export destination
				startActivityForResult(new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE), 4)
			)
			.setNeutralButton("Cancel", null)
			.setCancelable(false)
			.show();
	}
	
	private void importUserData(Uri uri) {
		Toast.makeText(this, "Importing...", Toast.LENGTH_SHORT).show();
		Executors.newSingleThreadExecutor().execute(() -> {
			runOnUiThread(()-> showProgressScreen());
			DocumentFile targetDir = DocumentFile.fromTreeUri(this, uri);
			for (DocumentFile child : targetDir.listFiles()) {
				if (!child.getName().equals("gamedata")) {
					runOnUiThread(()-> updateProgress("Importing " + child.getName(), 0));
					IOUtils.copyRecursive(this, child, getExternalFilesDir(null));
				}
			}
			runOnUiThread(()-> showLaunchScreen());
			runOnUiThread(()-> Toast.makeText(this, "Imported!", Toast.LENGTH_SHORT).show());
		});
	}
	
	private void exportUserData(Uri uri) {
		Toast.makeText(this, "Exporting...", Toast.LENGTH_SHORT).show();
		Executors.newSingleThreadExecutor().execute(() -> {
			runOnUiThread(()-> showProgressScreen());
			for (File child : getExternalFilesDir(null).listFiles()) {
				if (!child.getName().equals("gamedata")) {
					runOnUiThread(()-> updateProgress("Exporting " + child.getName(), 0));
					IOUtils.copyRecursive(this, child, uri);
				}
			}
			runOnUiThread(()-> showLaunchScreen());
			runOnUiThread(()-> Toast.makeText(this, "Exported!", Toast.LENGTH_SHORT).show());
		});
	}
	
	private void updateProgress(String progressMsg, int progress) {
		TextView progressText = (TextView) findViewById(R.id.download_msg);
		ProgressBar progressBar = (ProgressBar) findViewById(R.id.download_progress);
		progressBar.setIndeterminate(progress == -1);
		progressBar.setProgress(progress);
		progressText.setText(progressMsg);
	}

	private void updateDownloadProgress(int progress, int max, String type) {
		updateProgress(
			String.format("Downloading %s ... (%s/%s)", type, toSizeString(progress), toSizeString(max)),
			progress);
	}

	private void updateUnpackProgress(int progress, int max, String type) {
		// progress starts from 0 but asset counting starts from 1.
		// also, when installing from zip the total number of files is
		// not available, so don't show max in that case.
		String unpackMsg = max > 0
			? String.format("Unpacking %s assets... (%s/%s)", type, progress+1, max)
			: String.format("Unpacking %s assets... (%s)", type, progress+1);
		updateProgress(unpackMsg, progress);
	}

	private long downloadFile(String url, File destpath, long modified, String typeOrMsg, boolean isCustomMsg) {
		long newModified = 0;
		// based on https://stackoverflow.com/a/4896527/22060628
		Log.d("Download", "URL: " + url);

		try {
			HttpURLConnection conn = (HttpURLConnection) new URL(url).openConnection();
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

			newModified = conn.getLastModified();
			// File did not change on server, don't download.
			if (newModified == modified) {
				return newModified;
			}
			
			final int max = (int) conn.getContentLengthLong();
			final AtomicInteger progress = new AtomicInteger(0);
			final AtomicInteger length = new AtomicInteger(0);

			// TODO rewrite to use copyStream function.
			byte[] buffer = new byte[8192];
			try (
				DataInputStream in = new DataInputStream(conn.getInputStream());
				OutputStream out = new FileOutputStream(destpath))
			{
				runOnUiThread(() -> {
					ProgressBar progressBar = (ProgressBar) findViewById(R.id.download_progress);
					progressBar.setMax(max);
					progressBar.setProgress(0);
				});

				length.set(in.read(buffer));
				while (length.get() > 0) {
					out.write(buffer, 0, length.get());
					runOnUiThread(() -> {
						if (isCustomMsg) {
							updateProgress(typeOrMsg, -1);
						} else {
							updateDownloadProgress(progress.addAndGet(length.get()), max, typeOrMsg);
						}
					});
					length.set(in.read(buffer));
				}
			}

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

		try (ZipInputStream zf = new ZipInputStream(zipstream)) {
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
					IOUtils.copyStreamNoClose(zf, out);
					out.close();
				}

				Log.d("Unpack", "Unpacking " + type + ": " + progress.get());
				progress.incrementAndGet();
			}
			
			boolean res = applyDeleteList();
			
			Log.d("Unpack", "Done unpacking " + type);
			
			return res;
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
					IOUtils.copyStream(
						zf.getInputStream(ze),
						new FileOutputStream(new File(destdir, ze.getName())));
				}

				Log.d("Unpack", "Unpacking " + type + ": " + progress.get() + "/" + max);
				progress.incrementAndGet();
			}

			boolean res = applyDeleteList();
			
			Log.d("Unpack", "Done unpacking " + type);
			
			return res;
		} catch (ZipException e) {
			Log.e("Unpack", "ZIP exception", e);
			return false;
		} catch (FileNotFoundException e) {
			Log.e("Unpack", "File not found", e);
			return false;
		} catch (IOException e) {
			Log.e("Unpack", "IO exception", e);
			return false;
		}
	}
	
	/**
	 * Delete any files on the deletelist file (delete.list on zip root)
	 * inside ZIP. Deletelist file will be deleted on success.
	 */
	private boolean applyDeleteList() {
		Log.d("InitActivity", "Applying deletelist");
		File deleteList = new File(dataDir, "delete.list");
		if (!deleteList.exists()) {
			 // Unpack finished sucessfully and no deletelist, so no deletion needed
			Log.d("InitActivity", "deletelist " + deleteList.getAbsolutePath() + " not found, skipping");
			return true;
		}
		
		AtomicInteger progress = new AtomicInteger(1);
		runOnUiThread(() -> updateProgress("Patching", -1));
		String line = "";
		try (BufferedReader reader = Files.newBufferedReader(deleteList.toPath())) {
			Log.d("Unpack", "Reading deletelist");
			while ((line = reader.readLine()) != null) {
				File toDelete = new File(dataDir, line);
				if (toDelete.exists()) {
					Log.d("Unpack", "Deleting " + toDelete.getAbsolutePath());
					toDelete.delete();
					runOnUiThread(() -> updateProgress("Patching... (" +  progress.incrementAndGet() + ")", -1));
				} else {
					Log.d("Unpack", "File " + toDelete.getAbsolutePath() + " doesn't exist.");
				}
			}
			
			return true;
		} catch (IOException e) {
			Log.e("Unpack", "Deleting " + line + " failed.");
		}
		
		deleteList.delete();
		return false;
	}
}
