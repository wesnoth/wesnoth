/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.utils;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IProjectDescription;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.jface.dialogs.DialogSettings;
import org.wesnoth.Constants;
import org.wesnoth.Logger;
import org.wesnoth.Messages;
import org.wesnoth.templates.ReplaceableParameter;
import org.wesnoth.templates.TemplateProvider;


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
	public static DialogSettings getPropertiesForProject(IProject project)
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
	 * Saves the current properties of the specified project on the filesystem.
	 * If the file/properties don't exist they will be created
	 * @param project
	 */
	public static void saveCacheForProject(IProject project)
	{
		getCacheForProject(project).saveCache();
	}

	/**
	 * Creates a project that has associated the wesnoth nature using
	 * the specified handle. If the project is created there will be
	 * no modifications done by this method.
	 * @param handle the handle to the project
	 * @param description the default description used when the project is created
	 * @param the monitor will do a 30 worked amount in the method
	 * @throws CoreException
	 */
	public static int createWesnothProject(IProject handle, IProjectDescription description,
				boolean open, boolean createBuildXML, IProgressMonitor monitor) throws CoreException
	{
		if (handle.exists())
			return -1;
		String projectPath = null;

		if (handle.getLocation() == null && description != null)
			projectPath = description.getLocationURI().toString();
		else if (handle.getLocation() != null)
			projectPath = handle.getLocation().toOSString();

		monitor.subTask(Messages.ProjectUtils_0);
		if (projectPath != null)
		{
			// cleanup existing files
			ResourceUtils.removeFile(projectPath + "/.wesnoth"); //$NON-NLS-1$
			ResourceUtils.removeFile(projectPath + "/.project"); //$NON-NLS-1$
			ResourceUtils.removeFile(projectPath + "/.build.xml"); //$NON-NLS-1$
		}
		monitor.worked(5);

		monitor.subTask(String.format(Messages.ProjectUtils_4, handle.getName()));
		// create the project
		try{
			if (description == null)
				handle.create(monitor);
			else
			{
				handle.create(description, monitor);
			}
		}
		catch (CoreException e) { // project already exists
			Logger.getInstance().logException(e);
		}

		if (open)
			handle.open(monitor);
		monitor.worked(10);

		monitor.subTask(Messages.ProjectUtils_6);
		// add wesnoth nature
		IProjectDescription tmpDescription = handle.getDescription();
		tmpDescription.setNatureIds(new String[] { Constants.NATURE_WESNOTH /*,
				Constants.NATURE_XTEXT */ });
		handle.setDescription(tmpDescription, monitor);
		monitor.worked(5);

		// add the build.xml file
		if (createBuildXML)
		{
			ArrayList<ReplaceableParameter> param = new ArrayList<ReplaceableParameter>();
			param.add(new ReplaceableParameter("$$project_name", handle.getName())); //$NON-NLS-1$
			param.add(new ReplaceableParameter("$$project_dir_name", handle.getName())); //$NON-NLS-1$
			ResourceUtils.createFile(handle, "build.xml", //$NON-NLS-1$
					TemplateProvider.getInstance().getProcessedTemplate("build_xml", param), true); //$NON-NLS-1$
		}
		monitor.worked(10);
		return 0;
	}
}