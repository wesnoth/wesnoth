/*******************************************************************************
 * Copyright (c) 2010 - 2011 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.projects;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IProjectDescription;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.runtime.Path;
import org.wesnoth.Constants;
import org.wesnoth.Logger;
import org.wesnoth.Messages;
import org.wesnoth.preferences.Preferences;
import org.wesnoth.preferences.Preferences.Paths;
import org.wesnoth.templates.ReplaceableParameter;
import org.wesnoth.templates.TemplateProvider;
import org.wesnoth.utils.ResourceUtils;
import org.wesnoth.utils.StringUtils;

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
	 * Gets the properties map for this project.
     * @return A map with properties of the project
	 */
	public static Map<String, String> getPropertiesForProject(IProject project)
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
			cache.loadCache( );
			projectCache_.put(project,cache);
		}
		return cache;
	}

	/**
	 * Creates a new wesnoth project with the specified name
	 * and on the specified location on disk
	 * @param name The name of the new project
	 * @param location The location of the new project
     * @param installName The name of the install this project belongs to
	 * @return A project handle
	 * @throws CoreException
	 */
	public static IProject createWesnothProject( String name, String location,
	        String installName, IProgressMonitor monitor )
	{
	    IProject newProject = ResourcesPlugin.getWorkspace().getRoot().getProject( name );

        IProjectDescription description =
            ResourcesPlugin.getWorkspace().newProjectDescription( name );
        description.setLocation( new Path( location ) );

	    createWesnothProject( newProject, description, installName, true, monitor );

	    return newProject;
	}

	/**
	 * Creates a project that has associated the wesnoth nature using
	 * the specified handle. If the project is created there will be
	 * no modifications done by this method.
	 * @param handle the handle to the project
	 * @param description the default description used when the project is created
     * @param installName The name of the install this project belongs to
     * @param open True to open the project after it was created
	 * @param monitor the monitor will do a 30 worked amount in the method
	 * @throws CoreException
	 */
	public static int createWesnothProject( IProject handle, IProjectDescription description,
				String installName, boolean open, IProgressMonitor monitor)
	{
		if ( handle == null || handle.exists() )
			return -1;

		try{
		    String projectPath = null;

		    if (handle.getLocation() == null && description != null)
		        projectPath = description.getLocationURI().toString();
		    else
		        projectPath = handle.getLocation().toOSString();

		    monitor.subTask(Messages.ProjectUtils_0);

	        // cleanup existing files
	        ResourceUtils.removeFile(projectPath + "/.wesnoth"); //$NON-NLS-1$
	        ResourceUtils.removeFile(projectPath + "/.project"); //$NON-NLS-1$
	        ResourceUtils.removeFile(projectPath + "/.build.xml"); //$NON-NLS-1$
		    monitor.worked(5);

		    monitor.subTask(String.format(Messages.ProjectUtils_4, handle.getName()));
		    // create the project

		    if (description == null)
		        handle.create(monitor);
		    else
		        handle.create(description, monitor);

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

		    // add the build.xml file, but only if the project is not located
		    // into the data/campaigns or user's /addons directory
		    Paths paths = Preferences.getPaths( installName );
		    String normalizedPath = StringUtils.normalizePath( projectPath );
		    if ( ! normalizedPath.contains( StringUtils.normalizePath( paths.getCampaignDir( ) ) ) &&
		         ! normalizedPath.contains( StringUtils.normalizePath( paths.getAddonsDir( ) ) ) )
		    {
		        ArrayList<ReplaceableParameter> param = new ArrayList<ReplaceableParameter>();
		        param.add(new ReplaceableParameter("$$project_name", handle.getName())); //$NON-NLS-1$
		        param.add(new ReplaceableParameter("$$project_dir_name", handle.getName())); //$NON-NLS-1$
		        ResourceUtils.createFile(handle, "build.xml", //$NON-NLS-1$
		                TemplateProvider.getInstance().getProcessedTemplate("build_xml", param), true); //$NON-NLS-1$
		    }
		    monitor.worked(10);
        }
        catch (CoreException e) {
            Logger.getInstance().logException(e);
            return 1;
        }
		return 0;
	}
}