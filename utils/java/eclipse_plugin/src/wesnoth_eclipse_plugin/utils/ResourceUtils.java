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
import java.util.List;

import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;

import org.eclipse.core.resources.IContainer;
import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IFolder;
import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.IStatus;
import org.eclipse.core.runtime.NullProgressMonitor;
import org.eclipse.core.runtime.Path;
import org.eclipse.swt.SWT;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;

import wesnoth_eclipse_plugin.Logger;
import wesnoth_eclipse_plugin.templates.ReplaceableParameter;
import wesnoth_eclipse_plugin.templates.TemplateProvider;

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

	/**
	 * Creates the '.wesnoth' file with the specified path
	 * only if it doesn't exist already
	 * @param path The path of '.wesnoth' file
	 */
	public static void createWesnothFile(String path)
	{
		File wesnothFile = new File(path);
		try
		{
			if (!(wesnothFile.exists()))
			{
				createNewFile(wesnothFile.getAbsolutePath());
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
					TemplateProvider.getInstance().getProcessedTemplate("build_xml", params));
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
		if (new File(target).isDirectory() == false)
			return false;
		return new File(target).mkdirs();
	}

	//TODO: create a simple java wmlparsers in order to get the right values
	public static String getConfigKeyValue(String fileName, String propertyName)
	{
		if (fileName == null || propertyName.isEmpty())
			return null;

		String value = "";
		File file = new File(fileName);
		if (!file.exists())
			return null;

		String fileContents = ResourceUtils.getFileContents(file);
		if (fileContents == null)
			return null;

		int index = fileContents.indexOf(propertyName + "=");
		if (index == -1)
		{
			Logger.getInstance().log(String.format("property %s not found in file %s",
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
			if (project.getFile("_main.cfg").exists())
				targetResource = project.getFile("_main.cfg");
		}

		if (targetResource == null && resource instanceof IFolder)
		{
			IFolder folder = (IFolder)resource;
			if (folder.getFile(new Path("_main.cfg")).exists())
				targetResource = folder.getFile(new Path("_main.cfg"));
		}

		if (targetResource == null && resource instanceof IFile)
		{
			if (resource.getName().equals("_main.cfg"))
					targetResource = (IFile) resource;
			else
			{
				IProject project = resource.getProject();
				if (project.getFile("_main.cfg").exists())
					targetResource = project.getFile("_main.cfg");
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
								container.getProjectRelativePath().toOSString() + "/_main.cfg");
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
					PreprocessorUtils.getPreprocessedFilePath(
						getMainConfigLocation(resource), false, true).toString(),
						new WMLSaxHandler());
		if (handler == null)
			return null;
		return handler.CampaignId;
	}

	/**
	 * Gets the campaign id
	 * @param fileName
	 * @return
	 */
	public static String getScenarioID(IFile file)
	{
		WMLSaxHandler handler = (WMLSaxHandler) getWMLSAXHandlerFromResource(
				PreprocessorUtils.getPreprocessedFilePath(file, false, true).toString(),
				new WMLSaxHandler());
		if (handler == null)
			return null;
		return handler.ScenarioId;
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
			Logger.getInstance().logError("Using output: " + parser.getOutputContent());
			return null;
		}
		catch (Exception e)
		{
			Logger.getInstance().logException(e);
			return null;
		}
	}

	public static boolean isCampaignFile(String fileName)
	{
		if (!fileName.endsWith(".cfg"))
			return false;
		//TODO: replace this with a better checking
		String fileContentString = ResourceUtils.getFileContents(new File(fileName));
		return (fileContentString.contains("[campaign]") && fileContentString.contains("[/campaign]"));
	}
	public static boolean isScenarioFile(String fileName)
	{
		if (!fileName.endsWith(".cfg"))
			return false;
		//TODO: replace this with a better checkings
		String fileContentString = ResourceUtils.getFileContents(new File(fileName));
		return (fileContentString.contains("[scenario]") && fileContentString.contains("[/scenario]"));
	}
}