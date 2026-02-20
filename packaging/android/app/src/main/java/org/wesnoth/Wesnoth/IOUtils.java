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
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import android.content.Context;
import android.net.Uri;
import android.util.Log;
import android.webkit.MimeTypeMap;
import androidx.documentfile.provider.DocumentFile;

public class IOUtils {
	public static void copyStreamNoClose(InputStream in, OutputStream out) throws IOException {
		byte[] buffer = new byte[8192];
		int length;
		while ((length = in.read(buffer)) > 0) {
			out.write(buffer, 0, length);
		}
	}

	public static void copyStream(InputStream in, OutputStream out) throws IOException {
		copyStreamNoClose(in, out);
		out.close();
		in.close();
	}
	
	public static void copyRecursive(Context context, File sourceFile, Uri targetUri) {
		DocumentFile source = DocumentFile.fromFile(sourceFile);
		DocumentFile targetDir = DocumentFile.fromTreeUri(context, targetUri);
		if (source == null || targetDir == null) return;
		
		copyRecursive(context, source, targetDir);
	}
	
	public static void copyRecursive(Context context, DocumentFile source, File targetFile) {
		DocumentFile targetDir = DocumentFile.fromFile(targetFile);
		if (source == null || targetDir == null) return;
		
		copyRecursive(context, source, targetDir);
	}

	private static void copyRecursive(Context context, DocumentFile source, DocumentFile targetParent) {
		final String sourceName = source.getName();

		if (source.isDirectory()) {
			DocumentFile newDir = targetParent.findFile(sourceName);
			if (newDir == null || !newDir.isDirectory()) {
				newDir = targetParent.createDirectory(sourceName);
				if (newDir == null) {
					return;
				}
			}

			for (DocumentFile child : source.listFiles()) {
				copyRecursive(context, child, newDir);
			}
		} else {
			try {
				DocumentFile existingFile = targetParent.findFile(sourceName);
				if (existingFile != null && existingFile.isFile()) {
					existingFile.delete();
				}

				DocumentFile newFile = targetParent.createFile(getMimeType(sourceName), sourceName);
				if (newFile == null) {
					return;
				}

				InputStream in = context.getContentResolver().openInputStream(source.getUri());
				OutputStream out = context.getContentResolver().openOutputStream(newFile.getUri());

				if (in == null || out == null) {
					return;
				}

				copyStream(in, out);
			} catch (IOException ioe) {
				Log.e("Import/Export copy", "IO error", ioe);
			}
		}
	}
	
	private static String getMimeType(String filename) {
		String ext = filename.substring(filename.lastIndexOf('.') + 1).toLowerCase();
		return MimeTypeMap.getSingleton().getMimeTypeFromExtension(ext);
	}
}
