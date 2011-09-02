/*******************************************************************************
 * Copyright (c) 2011 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.tests.pde;

import java.util.Arrays;

import junit.framework.TestCase;

import org.eclipse.core.resources.IWorkspace;
import org.eclipse.core.resources.IWorkspaceDescription;
import org.eclipse.core.resources.ResourcesPlugin;

import org.wesnoth.installs.WesnothInstall;
import org.wesnoth.installs.WesnothInstallsUtils;
import org.wesnoth.preferences.Preferences;
import org.wesnoth.preferences.Preferences.Paths;
import org.wesnoth.utils.StringUtils;

/**
 * This class is the base PDE test. It does the following:
 * 1) Turn off auto-building
 * 2) Setup the wesnoth preferences ( they need to be supplied as
 * test arguments ). The default install name is "default"
 */
public class PDETest extends TestCase
{
    @Override
    protected void setUp( ) throws Exception
    {
        super.setUp( );

        // set the default preferences.
        Paths paths = Preferences.getPaths( "default" );

        paths.setWesnothExecutablePath( getPath( "wesnothExec" ) );
        paths.setWorkingDir( getPath( "wesnothWorkingDir" ) );
        paths.setUserDir( getPath( "wesnothUserDir" ) );
        paths.setWMLToolsDir( getPath( "wesnothWMLTools" ) );
        Preferences
            .setString( Preferences.PYTHON_PATH, getPath( "pythonPath" ) );

        Preferences.setDefaultInstallName( "default" );
        WesnothInstallsUtils.setInstalls( Arrays.asList( new WesnothInstall(
            "default", "trunk" ) ) );

        // turn off automatic building
        IWorkspace workspace = ResourcesPlugin.getWorkspace( );
        IWorkspaceDescription description = workspace.getDescription( );
        if( description.isAutoBuilding( ) ) {
            description.setAutoBuilding( false );
            workspace.setDescription( description );
        }
    }

    private String getPath( String propertyName )
    {
        String result = System.getProperty( propertyName );
        if( StringUtils.isNullOrEmpty( result ) ) {
            System.out.println( "Please set the '" + propertyName
                + "' parameter ( -DparameterName=value ) before testing!." );
            assertTrue( false );
        }

        return result;
    }

    @Override
    protected void tearDown( ) throws Exception
    {
        super.tearDown( );

        // reset Preferences
        Paths paths = Preferences.getPaths( "default" );
        paths.setUserDir( "" );
        paths.setWesnothExecutablePath( "" );
        paths.setWMLToolsDir( "" );
        paths.setWorkingDir( "" );

        Preferences.setDefaultInstallName( "" );
        Preferences.setString( Preferences.PYTHON_PATH, "" );
        WesnothInstallsUtils
            .setInstalls( Arrays.asList( new WesnothInstall[0] ) );
    }
}
