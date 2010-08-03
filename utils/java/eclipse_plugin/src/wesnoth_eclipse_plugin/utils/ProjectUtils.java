/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.utils;

import java.io.File;
import java.io.StringReader;
import java.util.HashMap;
import java.util.Map;
import java.util.Properties;

import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;

import org.eclipse.core.resources.IContainer;
import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IFolder;
import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.runtime.Path;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

import wesnoth_eclipse_plugin.Logger;

public class ProjectUtils
{
	/**
	 * A map which stores the caches for each project.
	 * The properties are stored in '.wesnoth' file
	 */
	private static Map<IProject, ProjectCache> projectCache_ =
		new HashMap<IProject, ProjectCache>();

	public static Map<IProject, ProjectCache> getProjectCaches()
	{
		return projectCache_;
	}

	/**
	 * Gets the properties store for specified project.
	 * If the store doesn't exist it will be created.
	 * This will return null if it has been an exception
	 * This method ensures it will get the latest up-to-date '.wesnoth' file
	 * @param project the project
	 */
	public static Properties getPropertiesForProject(IProject project)
	{
		return getCacheForProject(project).getProperties();
	}

	/**
	 * Gets the cache for the specified project
	 * @param project
	 * @return
	 */
	public static ProjectCache getCacheForProject(IProject project)
	{
		ProjectCache cache = projectCache_.get(project);

		if (cache == null)
		{
			cache = new ProjectCache(project);
			projectCache_.put(project,cache);
		}
		return cache;
	}

	/**
	 * Sets the properties store for the specified project and saves the file
	 * If the '.wesnoth' file doesn't exist it will be created
	 */
	public static void setPropertiesForProject(IProject project, Properties props)
	{
		projectCache_.put(project, new ProjectCache(project, props));
	}

	/**
	 * Saves the current properties of the specified project on the filesystem.
	 * If the file/properties don't exist they will be created
	 * @param project
	 */
	public static void saveCacheForProject(IProject project)
	{
		getCacheForProject(project).saveCache();
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
		WMLSaxHandler handler = getParsedWMLFromResource(
					PreprocessorUtils.getPreprocessedFilePath(
						getMainConfigLocation(resource), false, true).toString());
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
		WMLSaxHandler handler = getParsedWMLFromResource(
				PreprocessorUtils.getPreprocessedFilePath(file, false, true).toString());
		if (handler == null)
			return null;
		return handler.ScenarioId;
	}

	/**
	 * Returns the WMLSaxHandler for the parsed specified resource
	 * @param resourcePath The resourcepath to parse
	 * @return
	 */
	public static WMLSaxHandler getParsedWMLFromResource(String resourcePath)
	{
		ExternalToolInvoker parser = WMLTools.runWMLParser2(resourcePath);
		try{
			parser.waitForTool();
			SAXParser saxparser;
			saxparser = SAXParserFactory.newInstance().newSAXParser();

			WMLSaxHandler handler = new WMLSaxHandler();
			saxparser.parse(new InputSource(new StringReader(parser.getOutputContent())), handler);
			return handler;
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