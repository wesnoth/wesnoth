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
import java.util.List;

import org.eclipse.core.resources.IResource;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.Combo;

import org.wesnoth.Logger;
import org.wesnoth.Messages;
import org.wesnoth.preferences.Preferences;
import org.wesnoth.projects.ProjectUtils;
import org.wesnoth.utils.GUIUtils;
import org.wesnoth.utils.StringUtils;
import org.wesnoth.utils.WorkspaceUtils;

/**
 * Utilities class that handles Wesnoth installs
 */
public class WesnothInstallsUtils
{
    /**
     * Returns a list of the current wesnoth installations available
     * in the preferences store
     * 
     * @return A list with Wesnoth Installs
     */
    public static List< WesnothInstall > getInstalls( )
    {
        List< WesnothInstall > installsList = new ArrayList< WesnothInstall >( );
        // unpack installs
        String[] installs = Preferences.getString(
            Preferences.INST_INSTALL_LIST ).split( ";" ); //$NON-NLS-1$
        for( String str: installs ) {
            if( str.isEmpty( ) ) {
                continue;
            }

            String[] tokens = str.split( ":" ); //$NON-NLS-1$

            if( tokens.length != 2 ) {
                Logger.getInstance( ).logError(
                    "invalid install [" + str + "] in installs list." ); //$NON-NLS-1$ //$NON-NLS-2$
                continue;
            }

            installsList.add( new WesnothInstall( tokens[0], tokens[1] ) );
        }

        return installsList;
    }

    /**
     * Sets the specified Installs list in the preferences store
     * 
     * @param installsList
     *        The list to replace / set the installs list
     */
    public static void setInstalls( Collection< WesnothInstall > installsList )
    {
        // pack back the installs
        StringBuilder installs = new StringBuilder( );
        for( WesnothInstall install: installsList ) {
            if( installs.length( ) > 0 ) {
                installs.append( ";" ); //$NON-NLS-1$
            }

            installs.append( install.getName( ) );
            installs.append( ":" ); //$NON-NLS-1$
            installs.append( install.getVersion( ) );
        }
        Preferences.getPreferences( ).setValue( Preferences.INST_INSTALL_LIST,
            installs.toString( ) );
    }

    /**
     * Returns the install name for the specified resource
     * 
     * @param resourcePath
     *        The path to the resource
     * @return The install name for the resource
     */
    public static String getInstallNameForResource( String resourcePath )
    {
        if( StringUtils.isNullOrEmpty( resourcePath ) ) {
            return "";
        }

        return getInstallNameForResource( ResourcesPlugin.getWorkspace( )
            .getRoot( ).findMember( resourcePath ) );
    }

    /**
     * Returns the install name for the specified resource
     * 
     * @param resource
     *        The resource
     * @return The install name for the resource
     */
    public static String getInstallNameForResource( IResource resource )
    {
        if( resource == null || ! resource.exists( ) ) {
            return ""; //$NON-NLS-1$
        }

        return ProjectUtils.getCacheForProject( resource.getProject( ) )
            .getInstallName( );
    }

    /**
     * Sets the install name for the specified resource
     * 
     * @param resource
     *        The resource to set the install to
     * @param newInstallName
     *        The new install name
     */
    public static void setInstallNameForResource( IResource resource,
        String newInstallName )
    {
        if( resource == null ) {
            return;
        }

        ProjectUtils.getCacheForProject( resource.getProject( ) )
            .setInstallName( newInstallName );
    }

    /**
     * Fills the specified combo box with all the current installs
     * and selects the default or the first ( if no default exists )
     * 
     * @param comboBox
     *        The combobox to fill
     */
    public static void fillComboWithInstalls( Combo comboBox )
    {
        comboBox.removeAll( );
        comboBox.clearSelection( );

        // fill the installs
        String defaultInstallName = Preferences.getDefaultInstallName( );
        for( WesnothInstall install: WesnothInstallsUtils.getInstalls( ) ) {
            comboBox.add( install.getName( ) );

            // select the default
            if( install.getName( ).equals( defaultInstallName ) ) {
                comboBox.select( comboBox.getItemCount( ) - 1 );
            }
        }

        // select the first if there is no other selected
        if( comboBox.getSelectionIndex( ) == - 1
            && comboBox.getItemCount( ) > 0 ) {
            comboBox.select( 0 );
        }
    }

    /**
     * Checks whether the Wesnoth Installation is properly setup
     * for the specified resource. If it is not, it will guide the user
     * through selecting a proper install (if any).
     * 
     * @param resource
     *        True if the installation is valid, false
     *        otherwise
     * @return Boolean flag whether the setup was successfull or failed.
     */
    public static boolean setupInstallForResource( IResource resource )
    {
        String installName = getInstallNameForResource( resource );
        // check if Paths are set for the install.
        if( ! WorkspaceUtils.checkPathsAreSet( installName, false ) ) {

            // no defaut - nothing to do.
            if( Preferences.getDefaultInstallName( ).isEmpty( ) ) {
                return false;
            }

            if( GUIUtils
                .showMessageBox( String.format(
                    Messages.WesnothInstallsUtils_1, resource
                        .getProject( ).getName( ) ),
                    SWT.ICON_QUESTION | SWT.YES | SWT.NO ) == SWT.NO ) {
                return false; // no hope :(
            }

            // fallback to default
            installName = Preferences.getDefaultInstallName( );

            if( WorkspaceUtils.checkPathsAreSet( installName, true ) ) {
                // replace current install with the default
                setInstallNameForResource( resource, installName );

                // re-create the core library link
                ProjectUtils.createCoreLibraryFolder( resource.getProject( ),
                    IResource.NONE );

                return true;
            }

            // sorry, no happy ending
            return false;
        }

        return true;
    }
}
