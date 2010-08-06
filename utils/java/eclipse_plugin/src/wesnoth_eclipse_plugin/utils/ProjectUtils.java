/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.utils;

import java.util.HashMap;
import java.util.Map;
import java.util.Properties;

import org.eclipse.core.resources.IProject;

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
}