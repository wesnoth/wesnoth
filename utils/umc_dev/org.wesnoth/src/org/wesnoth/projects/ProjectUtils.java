/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
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

import org.eclipse.core.resources.IFolder;
import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IProjectDescription;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.resources.IWorkspaceRoot;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.runtime.IStatus;
import org.eclipse.core.runtime.NullProgressMonitor;
import org.eclipse.core.runtime.Path;

import org.wesnoth.Logger;
import org.wesnoth.Messages;
import org.wesnoth.builder.WesnothProjectNature;
import org.wesnoth.preferences.Preferences;
import org.wesnoth.preferences.Preferences.Paths;
import org.wesnoth.templates.ReplaceableParameter;
import org.wesnoth.templates.TemplateProvider;
import org.wesnoth.utils.ResourceUtils;
import org.wesnoth.utils.StringUtils;
import org.wesnoth.views.WesnothProjectsExplorer;

/**
 * Utilities class handling with Projects
 */
public class ProjectUtils
{
    /**
     * A map which stores the caches for each project.
     * The properties are stored in '.wesnoth' file
     */
    private static Map< IProject, ProjectCache > projectCache_ = new HashMap< IProject, ProjectCache >( );

    /**
     * Returns a map with projects and their caches
     * 
     * @return A {@link Map} instance.
     */
    public static Map< IProject, ProjectCache > getProjectCaches( )
    {
        return projectCache_;
    }

    /**
     * Gets the properties map for this project.
     * 
     * @param project
     *        The project to get the propertiesfor
     * 
     * @return A map with properties of the project
     */
    public static Map< String, String > getPropertiesForProject(
        IProject project )
    {
        return getCacheForProject( project ).getProperties( );
    }

    /**
     * Gets the cache for the specified project. If the cache doesn't
     * exist, a new one will be created.
     * 
     * @param project
     * @return A {@link ProjectCache} instance. It will never be null.
     */
    public static ProjectCache getCacheForProject( IProject project )
    {
        ProjectCache cache = projectCache_.get( project );

        if( cache == null ) {
            cache = new ProjectCache( project );
            cache.loadCache( );
            projectCache_.put( project, cache );
        }
        return cache;
    }

    /**
     * Creates a folder that link to the project's wesnoth installation's
     * 'data/core' directory
     * 
     * @param updateFlags
     *        bit-wise or of update flag constants
     *        (IResource.ALLOW_MISSING_LOCAL, IResource.REPLACE,
     *        IResource.BACKGROUND_REFRESH, and IResource.HIDDEN)
     * @param project
     *        The project to create the link for
     * @return True if the creation was successfully, false otherwise
     */
    public static boolean createCoreLibraryFolder( IProject project,
        int updateFlags )
    {
        ProjectCache cache = getCacheForProject( project );
        Paths paths = Preferences.getPaths( cache.getInstallName( ) );
        IFolder coreLibrary = project
            .getFolder( WesnothProjectsExplorer.CORE_LIBRARY_NAME );

        if( ResourcesPlugin.getWorkspace( ).validateLinkLocation( coreLibrary,
            paths.getCoreDirPath( ) ).getCode( ) != IStatus.ERROR ) {
            try {
                if( coreLibrary.exists( ) ) {
                    // the new Core Library will point to the same location.
                    // Skip it then.
                    if( coreLibrary.getLocation( ).equals(
                        paths.getCoreDirPath( ) ) ) {
                        return true;
                    }

                    coreLibrary.delete( true, new NullProgressMonitor( ) );
                }

                coreLibrary
                    .createLink( paths.getCoreDirPath( ), updateFlags,
                        new NullProgressMonitor( ) );

                coreLibrary.setDerived( true, new NullProgressMonitor( ) );
            } catch( CoreException e ) {
                Logger.getInstance( ).logException( e );
                return false;
            }
        }
        else {
            Logger.getInstance( ).log(
                "Couldn't create link on:" + paths.getCoreDir( )
                    + "; project: " + project.getName( ),
                "Cannot create the Wesnoth Core Library folder for project "
                    + project.getName( ) + "!" );
            return false;
        }

        return true;
    }

    /**
     * Creates a new wesnoth project with the specified name
     * and on the specified location on disk
     * 
     * @param name
     *        The name of the new project
     * @param location
     *        The location of the new project
     * @param installName
     *        The name of the install this project belongs to
     * @param monitor
     *        The monitor to monitor the progress
     * @return A project handle
     * @throws CoreException
     */
    public static IProject createWesnothProject( String name, String location,
        String installName, IProgressMonitor monitor )
    {
        return createWesnothProject( name, location, installName, false,
            monitor );
    }

    /**
     * Creates a new wesnoth project with the specified name
     * and on the specified location on disk
     * 
     * @param name
     *        The name of the new project
     * @param location
     *        The location of the new project
     * @param installName
     *        The name of the install this project belongs to
     * @param skipCreateBuildXML
     *        True to skip creating a build xml even if needed
     * @param monitor
     *        The monitor to monitor the progress
     * @return A project handle
     * @throws CoreException
     */
    public static IProject createWesnothProject( String name, String location,
        String installName, boolean skipCreateBuildXML,
        IProgressMonitor monitor )
    {
        IWorkspaceRoot root = ResourcesPlugin.getWorkspace( ).getRoot( );
        IProject newProject = root.getProject( name );
        IProjectDescription description = null;

        // set the path only if the project's location is outside the workspace
        // root
        if( location != null
            && ! location.equals( root.getLocation( ).toOSString( ) ) ) {
            description = ResourcesPlugin.getWorkspace( )
                .newProjectDescription( name );
            description.setLocation( new Path( location ) );
        }

        createWesnothProject( newProject, description, installName,
            skipCreateBuildXML, monitor );

        return newProject;
    }

    /**
     * Creates a project that has associated the wesnoth nature using
     * the specified handle. If the project is created there will be
     * no modifications done by this method.
     * 
     * @param handle
     *        the handle to the project
     * @param description
     *        the default description used when the project is created
     * @param installName
     *        The name of the install this project belongs to
     * @param skipCreateBuildXML
     *        True to skip creating a build xml even if needed
     * @param monitor
     *        the monitor will do a 30 worked amount in the method
     * @return
     *         0 - project was created successfully.
     *         -1 - project already exists or the handle is null
     *         1 - there was an error.
     * 
     * @throws CoreException
     */
    public static int createWesnothProject( IProject handle,
        IProjectDescription description, String installName,
        boolean skipCreateBuildXML, IProgressMonitor monitor )
    {
        if( handle == null || handle.exists( ) ) {
            return - 1;
        }

        try {
            String projectPath = null;

            if( handle.getLocation( ) == null && description != null ) {
                projectPath = description.getLocationURI( ).getPath( )
                    .substring( 1 );
            }
            else if( handle.getLocation( ) != null ) {
                projectPath = handle.getLocation( ).toOSString( );
            }
            else {
                // project is in workspace
                projectPath = ResourcesPlugin.getWorkspace( ).getRoot( )
                    .getLocation( ).toOSString( )
                    + "/" + handle.getProject( ).getName( );
            }

            monitor.subTask( Messages.ProjectUtils_0 );

            // cleanup existing files
            ResourceUtils.removeFile( projectPath + "/.wesnoth" ); //$NON-NLS-1$
            ResourceUtils.removeFile( projectPath + "/.project" ); //$NON-NLS-1$
            ResourceUtils.removeFile( projectPath + "/.build.xml" ); //$NON-NLS-1$
            monitor.worked( 5 );

            monitor.subTask( String.format( Messages.ProjectUtils_4,
                handle.getName( ) ) );

            // create the project
            if( description == null ) {
                handle.create( monitor );
            }
            else {
                handle.create( description, monitor );
            }

            handle.open( monitor );
            monitor.worked( 10 );

            monitor.subTask( Messages.ProjectUtils_6 );
            // add wesnoth nature
            IProjectDescription tmpDescription = handle.getDescription( );
            tmpDescription
                .setNatureIds( new String[] { WesnothProjectNature.ID_NATURE } );
            handle.setDescription( tmpDescription, monitor );
            monitor.worked( 5 );

            if( ! skipCreateBuildXML ) {
                // add the build.xml file, but only if the project is not
                // located
                // into the data/campaigns or user's /addons directory
                Paths paths = Preferences.getPaths( installName );
                String normalizedPath = StringUtils.normalizePath( projectPath );
                if( ! normalizedPath.contains( StringUtils.normalizePath( paths
                    .getCampaignDir( ) ) )
                    && ! normalizedPath.contains( StringUtils
                        .normalizePath( paths.getAddonsDir( ) ) ) ) {

                    ArrayList< ReplaceableParameter > param = new ArrayList< ReplaceableParameter >( );
                    param.add( new ReplaceableParameter(
                        "$$project_name", handle.getName( ) ) ); //$NON-NLS-1$
                    param.add( new ReplaceableParameter(
                        "$$project_dir_name", handle.getName( ) ) ); //$NON-NLS-1$
                    ResourceUtils.createFile( handle,
                        "build.xml", //$NON-NLS-1$
                        TemplateProvider.getInstance( ).getProcessedTemplate(
                            "build_xml", param ), true ); //$NON-NLS-1$
                }
            }
            monitor.worked( 10 );

            // save the install name
            ProjectCache cache = new ProjectCache( handle );
            cache.setInstallName( installName );
            cache.loadCache( );
            projectCache_.put( handle, cache );

            // create the Core library link
            createCoreLibraryFolder( handle, IResource.NONE );
            monitor.worked( 10 );

            handle.refreshLocal( IResource.DEPTH_ONE, monitor );
        } catch( CoreException e ) {
            Logger.getInstance( ).logException( e );
            return 1;
        }
        return 0;
    }
}
