/*******************************************************************************
 * Copyright (c) 2011 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.installs;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.eclipse.core.resources.IResource;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.swt.SWT;
import org.wesnoth.Constants;
import org.wesnoth.Logger;
import org.wesnoth.preferences.Preferences;
import org.wesnoth.projects.ProjectUtils;
import org.wesnoth.utils.GUIUtils;
import org.wesnoth.utils.WorkspaceUtils;

public class WesnothInstallsUtils
{
    /**
     * A map which stores the installs settings
     */
    private static Map< String, WesnothInstall > installs_ =
        new HashMap< String, WesnothInstall >();

    /**
     * Returns a list of the current wesnoth installations available
     * in the preferences store
     * @return A list with Wesnoth Installs
     */
    public static List<WesnothInstall> getInstalls()
    {
        List<WesnothInstall> installsList = new ArrayList<WesnothInstall>();
        // unpack installs
        String[] installs = Preferences.getString( Constants.P_INST_INSTALL_LIST ).split( ";" ); //$NON-NLS-1$
        for ( String str : installs ){
            if ( str.isEmpty() )
                continue;

            String[] tokens = str.split( ":" ); //$NON-NLS-1$

            if ( tokens.length != 2 ) {
                Logger.getInstance().logError( "invalid install [" + str + "] in installs list." ); //$NON-NLS-1$
                continue;
            }

            installsList.add( new WesnothInstall( tokens[0], tokens[1] ) );
        }

        return installsList;
    }

    /**
     * Sets the specified Installs list in the preferences store
     * @param installsList The list to replace / set the installs list
     */
    public static void setInstalls( Collection<WesnothInstall> installsList )
    {
        // pack back the installs
        StringBuilder installs = new StringBuilder();
        for ( WesnothInstall install : installsList ) {
            if ( installs.length() > 0 )
                installs.append( ";" ); //$NON-NLS-1$

            installs.append( install.getName( ) );
            installs.append( ":" ); //$NON-NLS-1$
            installs.append( install.getVersion( ) );
        }
        Preferences.getPreferences().setValue( Constants.P_INST_INSTALL_LIST, installs.toString() );
    }

    /**
     * Returns the install name for the specified resource
     * @param resourcePath The path to the resource
     * @return The install name for the resource
     */
    public static String getInstallNameForResource( String resourcePath )
    {
        return getInstallNameForResource( ResourcesPlugin.getWorkspace( ).getRoot( ).
                findMember( resourcePath ) );
    }

    /**
     * Returns the install name for the specified resource
     * @param resource The resource
     * @return The install name for the resource
     */
    public static String getInstallNameForResource( IResource resource )
    {
        if ( resource == null )
            return ""; //$NON-NLS-1$

        return ProjectUtils.getCacheForProject( resource.getProject( ) ).getInstallName( );
    }

    /**
     * Sets the install name for the specified resource
     * @param resource The resource to set the install to
     * @param newInstallName The new install name
     */
    public static void setInstallNameForResource( IResource resource, String newInstallName )
    {
        if ( resource == null )
            return;

        ProjectUtils.getCacheForProject( resource.getProject( ) ).setInstallName( newInstallName );
    }

    /**
     * Checks whether the Wesnoth Installation is properly setup
     * for the specified resource. If it is not, it will guide the user
     * through selecting a proper install (if any).
     *
     * @param resource True if the installation is valid, false
     * otherwise
     * @return Boolean flag whether the setup was successfull or failed.
     */
    public static boolean setupInstallForResource( IResource resource )
    {
        String installName = getInstallNameForResource( resource );
        // check if Paths are set for the install.
        if ( ! WorkspaceUtils.checkPathsAreSet( installName, false ) ) {

            // no defaut - nothing to do.
            if ( Preferences.getDefaultInstallName( ).isEmpty( ) )
                return false;

            if ( GUIUtils.showMessageBox(
                    "The existing set install for the project " +
                    resource.getProject( ).getName( ) +
                    " doesn't exist anymore or isn't fully configured. " +
                    "Do you want to try fallback to the default?",
                    SWT.ICON_QUESTION | SWT.YES | SWT.NO) == SWT.NO )
                return false; // no hope :(

            // fallback to default
            installName = Preferences.getDefaultInstallName( );

            if ( WorkspaceUtils.checkPathsAreSet( installName, true ) ) {
                // replace current install with the default
                setInstallNameForResource( resource, installName );
                return true;
            }

            // sorry, no happy ending
            return false;
        }

        return true;
    }
}
