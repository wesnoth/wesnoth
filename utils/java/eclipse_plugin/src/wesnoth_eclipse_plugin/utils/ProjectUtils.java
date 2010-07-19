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
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map.Entry;

import org.eclipse.core.resources.IContainer;
import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IFolder;
import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.runtime.Path;

import wesnoth_eclipse_plugin.Logger;

public class ProjectUtils
{
	private static HashMap<IProject, HashMap<String, List<String>>> projectPreferences_ =
		new HashMap<IProject, HashMap<String, List<String>>>();

	public static HashMap<String, List<String>> getPreferencesForProject(IProject project)
	{
		return projectPreferences_.get(project);
	}

	public static void setPreferencesForProject(IProject project, HashMap<String,
			List<String>> prefs)
	{
		projectPreferences_.put(project, prefs);
	}

	public static HashMap<String, String> getSettingsForProject(IProject project)
	{
		HashMap<String, List<String>> pref = getPreferencesForProject(project);
		if (pref == null)
			return null;
		List<String> settingsList = pref.get("settings");
		HashMap<String, String> settings = new HashMap<String, String>();
		for (String string : settingsList)
		{
			settings.put(string.split("=")[0], string.split("=")[1]);
		}
		return settings;
	}

	public static void setSettingsForProject(IProject project,HashMap<String, String> settings)
	{
		List<String> settingsList = new ArrayList<String>();
		for (Entry<String,String> key : settings.entrySet())
		{
			settingsList.add(key.getKey() + "=" + key.getValue());
		}
		HashMap<String, List<String>> prefs = getPreferencesForProject(project);
		prefs.put("settings",settingsList);
		setPreferencesForProject(project, prefs);
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
	 * Returns "_main.cfg" location relative to user's directory
	 * from the specified resource or null if it isn't any
	 * @param resource The resource where to search for '_main.cfg'
	 * @return
	 */
	public static String getMainConfigLocation(IResource resource)
	{
		if (resource == null)
			return null;

		IResource targetResource = null;
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
					targetResource = resource;
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
		return WorkspaceUtils.getPathRelativeToUserDir(targetResource);
	}

	public static String getCampaignID(IResource resource)
	{
		return getConfigKeyValue(getMainConfigLocation(resource),"id");
	}
	public static String getScenarioID(String fileName)
	{
		return getConfigKeyValue(fileName,"id");
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