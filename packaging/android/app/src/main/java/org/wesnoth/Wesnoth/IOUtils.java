package org.wesnoth.Wesnoth;

import java.io.File;
import java.io.FileInputStream;
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
	
	public static void copyFolderToTree(Context context, File sourceDir, Uri treeUri) {
		DocumentFile targetRoot = DocumentFile.fromTreeUri(context, treeUri);
		if (targetRoot == null) return;
		
		copyRecursive(context, sourceDir, targetRoot);
	}

	private static void copyRecursive(Context context, File source, DocumentFile targetParent) {
		if (source.isDirectory()) {
			DocumentFile newDir = targetParent.findFile(source.getName());
			if (newDir == null || !newDir.isDirectory()) {
				newDir = targetParent.createDirectory(source.getName());
			}

			for (File child : source.listFiles()) {
				copyRecursive(context, child, newDir);
			}
		} else {
			try {
				DocumentFile existingFile = targetParent.findFile(source.getName());
				if (existingFile != null && existingFile.isFile()) {
					existingFile.delete();
				}

				DocumentFile newFile = targetParent.createFile(getMimeType(source.getName()), source.getName());
				copyStream(
					new FileInputStream(source),
					context.getContentResolver().openOutputStream(newFile.getUri()));
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
