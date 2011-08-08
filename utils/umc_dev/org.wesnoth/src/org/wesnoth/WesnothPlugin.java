/*******************************************************************************
 * Copyright (c) 2010 - 2011 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth;

import java.util.Map.Entry;

import org.osgi.framework.BundleContext;

import org.eclipse.core.resources.IProject;
import org.eclipse.jface.dialogs.TrayDialog;
import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.plugin.AbstractUIPlugin;

import org.wesnoth.preprocessor.PreprocessorUtils;
import org.wesnoth.projects.ProjectCache;
import org.wesnoth.projects.ProjectUtils;
import org.wesnoth.utils.WorkspaceUtils;

/**
 * The activator class controls the plug-in life cycle
 */
public class WesnothPlugin extends AbstractUIPlugin
{
    // The shared instance
    private static WesnothPlugin plugin;

    /**
     * The constructor
     */
    public WesnothPlugin( )
    {
    }

    @Override
    public void start( BundleContext context ) throws Exception
    {
        super.start( context );
        plugin = this;
        Logger.getInstance( ).startLogger( );

        if( PlatformUI.isWorkbenchRunning( ) ) {
            if( WorkspaceUtils.checkPathsAreSet( null, false ) == false ) {
                Display.getDefault( ).asyncExec( new Runnable( ) {
                    @Override
                    public void run( )
                    {
                        WorkspaceUtils.setupWorkspace( true );
                    }
                } );
            }
        }

        TrayDialog.setDialogHelpAvailable( true );
    }

    @Override
    public void stop( BundleContext context ) throws Exception
    {
        // save the caches of the projects on disk
        for( Entry< IProject, ProjectCache > cache: ProjectUtils
            .getProjectCaches( ).entrySet( ) ) {
            cache.getValue( ).saveCache( );
        }
        PreprocessorUtils.getInstance( ).saveTimestamps( );

        Logger.getInstance( ).stopLogger( );

        plugin = null;
        super.stop( context );
    }

    /**
     * Returns the shared instance
     * 
     * @return the shared instance
     */
    public static WesnothPlugin getDefault( )
    {
        return plugin;
    }

    /**
     * Returns an image descriptor for the image file at the given plug-in
     * relative path
     * 
     * @param path
     *        the path
     * @return the image descriptor
     */
    public static ImageDescriptor getImageDescriptor( String path )
    {
        return imageDescriptorFromPlugin( Constants.PLUGIN_ID, path );
    }

    /**
     * Returns the plugin's shell
     * 
     * @return
     */
    public static Shell getShell( )
    {
        return plugin.getWorkbench( ).getDisplay( ).getActiveShell( );
    }
}
