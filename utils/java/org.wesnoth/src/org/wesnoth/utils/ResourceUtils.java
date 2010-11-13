/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.utils;

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
import java.util.List;

import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;

import org.eclipse.core.resources.IContainer;
import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IFolder;
import org.eclipse.core.resources.IMarker;
import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.IStatus;
import org.eclipse.core.runtime.NullProgressMonitor;
import org.eclipse.core.runtime.Path;
import org.eclipse.jface.dialogs.DialogSettings;
import org.eclipse.swt.SWT;
import org.wesnoth.Constants;
import org.wesnoth.Logger;
import org.wesnoth.Messages;
import org.wesnoth.templates.ReplaceableParameter;
import org.wesnoth.templates.TemplateProvider;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;

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

	/**
	 * Gets the contents as string of the specified file
	 * @param file The file
	 * @return
	 */
	public static String getFileContents(File file)
	{
		if (!file.exists() || !file.isFile())
			return ""; //$NON-NLS-1$

		String contentsString = ""; //$NON-NLS-1$
		BufferedReader reader = null;
		try
		{
			String line = ""; //$NON-NLS-1$
			reader = new BufferedReader(new InputStreamReader(new FileInputStream(file)));
			while ((line = reader.readLine()) != null)
			{
				contentsString += (line + "\n"); //$NON-NLS-1$
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
			Logger.getInstance().log(Messages.ResourceUtils_4 + resourceName, IStatus.ERROR);
			GUIUtils.showMessageBox(Messages.ResourceUtils_5 + resourceName, SWT.ICON_ERROR);
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
			fileContentsString = ""; //$NON-NLS-1$
			Logger.getInstance().log(Messages.ResourceUtils_7, IStatus.WARNING);
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

	/**
	 * Creates the '.wesnoth' file with the specified path
	 * only if it doesn't exist already
	 * @param path The path of '.wesnoth' file
	 * @param force If true the file will be re-created if exists
	 */
	public static void createWesnothFile(String path, boolean force)
	{
		File wesnothFile = new File(path);
		try
		{
			if (force == true ||
				(force == false && wesnothFile.exists() == false))
			{
				createNewFile(wesnothFile.getAbsolutePath());
				new DialogSettings("project").save(path); //$NON-NLS-1$
			}
		}
		catch (Exception e) {
			Logger.getInstance().logException(e);
		}
	}

	/**
	 * Creates the 'build.xml' with the specified path
	 * @param path The full path to the 'build.xml' file
	 * @param params The parameters list to replace in the template of 'build.xml'
	 */
	public static void createBuildXMLFile(String path,
			List<ReplaceableParameter> params)
	{
		try{
			File antFile = new File(path);
			createNewFile(antFile.getAbsolutePath());
			FileWriter writer = new FileWriter(antFile);
			writer.write(
					TemplateProvider.getInstance().getProcessedTemplate("build_xml", params)); //$NON-NLS-1$
			writer.close();
		}
		catch (Exception e) {
			Logger.getInstance().logException(e);
		}
	}

	/**
	 * Creates a new empty file in the target.
	 * Subsequent non-existent directories in the path will be created
	 * @param target
	 * @return
	 */
	public static boolean createNewFile(String target)
	{
		createDirectory(new File(target).getParent());
		try
		{
			return new File(target).createNewFile();
		}
		catch (IOException e)
		{
			return false;
		}
	}

	/**
	 * Creates the specified directory.
	 * Subsequent non-existent directories will be created
	 * @param target
	 * @return
	 */
	public static boolean createDirectory(String target)
	{
		return new File(target).mkdirs();
	}

	/**
	 * Removes the specified file
	 * @param target
	 * @return
	 */
	public static boolean removeFile(String target)
	{
		if (new File(target).exists())
			return new File(target).delete();
		return true;
	}

	//TODO: create a simple java wmlparsers in order to get the right values
	public static String getConfigKeyValue(String fileName, String propertyName)
	{
		if (fileName == null || propertyName.isEmpty())
			return null;

		String value = ""; //$NON-NLS-1$
		File file = new File(fileName);
		if (!file.exists())
			return null;

		String fileContents = ResourceUtils.getFileContents(file);
		if (fileContents == null)
			return null;

		int index = fileContents.indexOf(propertyName + "="); //$NON-NLS-1$
		if (index == -1)
		{
			Logger.getInstance().log(String.format(Messages.ResourceUtils_12,
					propertyName, fileName));
			return null;
		}
		index += (propertyName.length() + 1); // jump over the property name characters

		// skipp spaces between the property name and value (if any)
		while(index < fileContents.length() && fileContents.charAt(index) == ' ')
			++index;

		while(index < fileContents.length() && fileContents.charAt(index) != '#' &&
				fileContents.charAt(index) != ' ' &&
				fileContents.charAt(index) != '\r' && fileContents.charAt(index) != '\n')
		{
			value += fileContents.charAt(index);
			++index;
		}

		return value;
	}

	/**
	 * Returns "_main.cfg" file
	 * from the specified resource or null if it isn't any
	 * It will start searching upwards starting from curren
	 * resource's directory, until it finds a '_main.cfg' but it will
	 * stop when encounters a project
	 *
	 * @param resource The resource where to search for '_main.cfg'
	 * @return
	 */
	public static IFile getMainConfigLocation(IResource resource)
	{
		if (resource == null)
			return null;

		IFile targetResource = null;
		if (resource instanceof IProject)
		{
			IProject project = (IProject)resource;
			if (project.getFile("_main.cfg").exists()) //$NON-NLS-1$
				targetResource = project.getFile("_main.cfg"); //$NON-NLS-1$
		}

		if (targetResource == null && resource instanceof IFolder)
		{
			IFolder folder = (IFolder)resource;
			if (folder.getFile(new Path("_main.cfg")).exists()) //$NON-NLS-1$
				targetResource = folder.getFile(new Path("_main.cfg")); //$NON-NLS-1$
		}

		if (targetResource == null && resource instanceof IFile)
		{
			if (resource.getName().equals("_main.cfg")) //$NON-NLS-1$
					targetResource = (IFile) resource;
			else
			{
				IProject project = resource.getProject();
				if (project.getFile("_main.cfg").exists()) //$NON-NLS-1$
					targetResource = project.getFile("_main.cfg"); //$NON-NLS-1$
				else
				{
					// this might be the case of "user addon's" project
					// we're going to the first subdirectory under the project
					IContainer container = resource.getParent();
					if (container != null)
					{
						while(container.getParent() != null &&
								container.getParent() != resource.getProject())
						{
							container = container.getParent();
						}
						IFile file = project.getFile(
								container.getProjectRelativePath().toOSString() + "/_main.cfg"); //$NON-NLS-1$
						if (file.exists())
							targetResource = file;
					}
				}
			}
		}
		if (targetResource == null)
			return null;
		return targetResource;
	}

	/**
	 * Gets the campaign id from the specified resource, or null
	 * If the resource is not a '_main.cfg' it will search for it
	 * with {@link ProjectUtils#getMainConfigLocation(IResource)}
	 * @param resource The resource where to search the id
	 * @return
	 */
	public static String getCampaignID(IResource resource)
	{
		WMLSaxHandler handler = (WMLSaxHandler) getWMLSAXHandlerFromResource(
				PreprocessorUtils.getInstance().getPreprocessedFilePath(
						getMainConfigLocation(resource), false, true).toString(),
						new WMLSaxHandler(resource.getLocation().toOSString()));
		if (handler == null)
			return null;
		return handler.getConfigFile().getCampaignId();
	}

	/**
	 * Gets the campaign id
	 * @param fileName
	 * @return
	 */
	public static String getScenarioID(IFile file)
	{
		WMLSaxHandler handler = (WMLSaxHandler) getWMLSAXHandlerFromResource(
				PreprocessorUtils.getInstance().getPreprocessedFilePath(file, false, true).toString(),
				new WMLSaxHandler(file.getLocation().toOSString()));
		if (handler == null)
			return null;
		return handler.getConfigFile().getScenarioId();
	}

	/**
	 * Returns the SaxHandler for the parsed specified wml resource
	 * @param resourcePath The resourcepath to parse
	 * @param saxHandler The SAX Handler used to handle the parsed wml
	 * @return
	 */
	public static DefaultHandler getWMLSAXHandlerFromResource(String resourcePath,
					DefaultHandler saxHandler)
	{
		ExternalToolInvoker parser = WMLTools.runWMLParser2(resourcePath);
		if (parser == null)
			return null;
		try{
			SAXParser saxparser;
			saxparser = SAXParserFactory.newInstance().newSAXParser();

			saxparser.parse(new InputSource(parser.getStdout()), saxHandler);
			return saxHandler;
		}
		catch (SAXException e) {
			Logger.getInstance().logException(e);
			Logger.getInstance().logError(Messages.ResourceUtils_21 + parser.getOutputContent());
			return null;
		}
		catch (Exception e)
		{
			Logger.getInstance().logException(e);
			return null;
		}
	}

	/**
	 * @deprecated Use the {@link WMLSaxHandler#IsCampaign}
	 * @param fileName
	 * @return
	 */
	@Deprecated
	public static boolean isCampaignFile(String fileName)
	{
		if (!fileName.endsWith(".cfg")) //$NON-NLS-1$
			return false;
		String fileContentString = ResourceUtils.getFileContents(new File(fileName));
		return (fileContentString.contains("[campaign]") && fileContentString.contains("[/campaign]")); //$NON-NLS-1$ //$NON-NLS-2$
	}

	/**
	 * @deprecated Use the {@link WMLSaxHandler#IsScenario}
	 * @param fileName
	 * @return
	 */
	@Deprecated
	public static boolean isScenarioFile(String fileName)
	{
		if (!fileName.endsWith(".cfg")) //$NON-NLS-1$
			return false;
		String fileContentString = ResourceUtils.getFileContents(new File(fileName));
		return (fileContentString.contains("[scenario]") && fileContentString.contains("[/scenario]")); //$NON-NLS-1$ //$NON-NLS-2$
	}

	/**
	 * Deletes all markers of type specified from the resource
	 * @param resource
	 * @param type
	 */
	public static void deleteMarkers(IResource resource, String type)
	{
		try
		{
			resource.deleteMarkers(type, false, IResource.DEPTH_INFINITE);
		} catch (CoreException e)
		{
			Logger.getInstance().logException(e);
		}
	}

	/**
	 * Parses the current line of the file and add the marker (if any) on the file.
	 * Current used format: "sourcefile", line x: error message
	 * @param line the line to parse
	 * @param type the created marker or null if there was none
	 * @return
	 */
	public static IMarker addMarkerForLine(String line, String type)
	{
		// error template, is the one used by GCC:
		// "sourcefile", line <linenumber>: error message
		try
		{
			final String pivot = ", line "; //$NON-NLS-1$
			int pivotIndex =  line.indexOf(pivot, 2);

			String sourceFile = line.substring(1, pivotIndex - 1);
			int lineNumber = Integer.parseInt(
					line.substring(pivotIndex + pivot.length(),
							line.indexOf(":", pivotIndex + pivot.length() + 1))); //$NON-NLS-1$
			String message = line.substring(line.indexOf(" ", pivotIndex + pivot.length() + 1)); //$NON-NLS-1$

			// Get the file
			IFile file = ResourcesPlugin.getWorkspace().getRoot().
				getFileForLocation(new Path(sourceFile));
			if (file.exists() == false)
				return null;

			IMarker marker = file.createMarker(type);
			marker.setAttribute(IMarker.SOURCE_ID, sourceFile);
			marker.setAttribute(IMarker.MESSAGE, message);
			marker.setAttribute(IMarker.LINE_NUMBER, lineNumber);

			// wmllint should be info or warning
			if (type.equals(Constants.MARKER_WMLLINT))
				marker.setAttribute(IMarker.SEVERITY, IMarker.SEVERITY_INFO);
			else
				marker.setAttribute(IMarker.SEVERITY, IMarker.SEVERITY_ERROR);
			return marker;
		}
		catch (Exception e)
		{
			Logger.getInstance().logException(e);
			return null;
		}
	}
}