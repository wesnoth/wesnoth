/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.utils;

import java.io.BufferedReader;
import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;

import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IFolder;
import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.IStatus;
import org.eclipse.core.runtime.NullProgressMonitor;
import org.eclipse.swt.SWT;

import wesnoth_eclipse_plugin.Logger;

public class ResourceUtils
{
	/**
	 * Copies a file from source to target
	 * @param source
	 * @param target
	 * @throws IOException
	 */
	public static boolean copyTo(File source, File target)
	{
		if (source == null || target == null)
			return false;

		try{
			InputStream in = new FileInputStream(source);
			OutputStream out = new FileOutputStream(target);

			// Transfer bytes from in to out
			byte[] buf = new byte[1024];
			int len;
			while ((len = in.read(buf)) > 0)
			{
				out.write(buf, 0, len);
			}
			in.close();
			out.close();

			return true;
		}
		catch(IOException e)
		{
			Logger.getInstance().logException(e);
			return false;
		}
	}

	public static String getFileContents(File file)
	{
		if (!file.exists() || !file.isFile())
			return "";

		String contentsString = "";
		BufferedReader reader = null;
		try
		{
			String line = "";
			reader = new BufferedReader(new InputStreamReader(new FileInputStream(file)));
			while ((line = reader.readLine()) != null)
			{
				contentsString += (line + "\n");
			}
		} catch (IOException e)
		{
			Logger.getInstance().logException(e);
		} finally
		{
			try
			{
				if (reader!= null)
					reader.close();
			} catch (Exception e)
			{
				Logger.getInstance().logException(e);
			}
		}
		return contentsString;
	}

	/**
	 * Creates the desired resource
	 *
	 * @param resource the resource to be created (IFile/IFolder)
	 * @param project the project where to be created the resource
	 * @param resourceName the name of the resource
	 * @param input the contents of the resource or null if no contents needed
	 */
	public static void createResource(IResource resource, IProject project,
						String resourceName, InputStream input)
	{
		try
		{
			if (!project.isOpen())
			{
				project.open(new NullProgressMonitor());
			}

			if (resource.exists())
				return;

			if (resource instanceof IFile)
			{
				((IFile) resource).create(input, true, new NullProgressMonitor());
			} else if (resource instanceof IFolder)
			{
				((IFolder) resource).create(true, true, new NullProgressMonitor());
			}

		} catch (CoreException e)
		{
			Logger.getInstance().log("Error creating the resource" + resourceName, IStatus.ERROR);
			GUIUtils.showMessageBox("Error creating the resource" + resourceName, SWT.ICON_ERROR);
			Logger.getInstance().logException(e);
		}
	}

	/**
	 * Creates a folder in the specified project with the specified details
	 * @param project the project in which the folder will be created
	 * @param folderName the name of the folder
	 */
	public static void createFolder(IProject project, String folderName)
	{
		IFolder folder = project.getFolder(folderName);
		createResource(folder, project, folderName, null);
	}

	/**
	 * Creates a file in the specified project with the specified details
	 * @param project the project in which the file will be created
	 * @param fileName the filename of the file
	 * @param fileContentsString the text which will be contained in the file
	 * @param overwrite true to overwrite the file if it already exists
	 */
	public static void createFile(IProject project, String fileName, String fileContentsString,
			boolean overwrite)
	{
		IFile file = project.getFile(fileName);
		if (fileContentsString == null)
		{
			fileContentsString = "";
			Logger.getInstance().log("file contents are null", IStatus.WARNING);
		}

		if (file.exists() && overwrite)
		{
			try
			{
				file.delete(true, null);
			} catch (CoreException e)
			{
				Logger.getInstance().logException(e);
			}
		}

		ByteArrayInputStream inputStream = new ByteArrayInputStream(fileContentsString.getBytes());
		createResource(file, project, fileName, inputStream);
	}

	public static void createWesnothFile(String path)
	{
		File wesnothFile = new File(path);
		try
		{
			if (!(wesnothFile.exists()))
			{
				wesnothFile.createNewFile();
				FileWriter writer = new FileWriter(wesnothFile);
				writer.write("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n");
				writer.write("<!DOCTYPE properties SYSTEM \"http://java.sun.com/dtd/properties.dtd\">\n");
				writer.write("<properties>\n</properties>\n");
				writer.close();
			}
		}
		catch (Exception e) {
			Logger.getInstance().logException(e);
		}
	}
}