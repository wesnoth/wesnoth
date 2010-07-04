/**
 * @author Timotei Dolean
 */
package wesnoth_eclipse_plugin.utils;

import java.io.BufferedReader;
import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
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
import org.eclipse.core.runtime.Status;
import org.eclipse.jface.dialogs.ErrorDialog;

import wesnoth_eclipse_plugin.Activator;
import wesnoth_eclipse_plugin.Logger;

public class ResourceUtils
{
	/**
	 * Copies a file from source to target
	 * @param source
	 * @param target
	 * @throws Exception
	 */
	public static void copyTo(File source, File target) throws Exception
	{
		if (source == null || target == null)
			return;

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
	}

	public static String getFileContents(File file)
	{
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
		} catch (Exception e)
		{
			e.printStackTrace();
			return null;
		} finally
		{
			try
			{
				reader.close();
			} catch (Exception e)
			{
				e.printStackTrace();
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
	public static void createResource(IResource resource, IProject project, String resourceName, InputStream input)
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
			Logger.print("Error creating the resource" + resourceName, IStatus.ERROR);
			ErrorDialog dlgDialog = new ErrorDialog(Activator.getShell(), "Error creating the file", "There was an error creating the resource: " + resourceName,
					new Status(IStatus.ERROR, "wesnoth_plugin", "error"), 0);
			dlgDialog.open();
			e.printStackTrace();
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
	 */
	public static void createFile(IProject project, String fileName, String fileContentsString)
	{
		IFile file = project.getFile(fileName);
		if (fileContentsString == null)
		{
			fileContentsString = "";
			Logger.print("file contents are null", 2);
		}

		ByteArrayInputStream inputStream = new ByteArrayInputStream(fileContentsString.getBytes());
		createResource(file, project, fileName, inputStream);
	}
}