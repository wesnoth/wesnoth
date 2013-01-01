/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.utils;

import java.io.File;

import org.eclipse.core.resources.IResource;
import org.eclipse.core.runtime.IPath;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.FileDialog;

import org.wesnoth.Logger;
import org.wesnoth.Messages;
import org.wesnoth.WesnothPlugin;

/**
 * Utilities class that handles with Wesnoth Map Files (*.map)
 */
public class MapUtils
{
    /**
     * Import a map file into the currently selected directory
     */
    public static void importMap( )
    {
        if( WorkspaceUtils.getSelectedFolder( ) == null ) {
            Logger.getInstance( ).log( "no directory selected (importMap)", //$NON-NLS-1$
                Messages.MapUtils_1 );
            return;
        }

        FileDialog mapDialog = new FileDialog( WesnothPlugin.getShell( ),
            SWT.OPEN );
        mapDialog.setText( Messages.MapUtils_2 );
        mapDialog.setFilterExtensions( new String[] { "*.map" } ); //$NON-NLS-1$
        String file = mapDialog.open( );

        if( file == null ) {
            return;
        }

        try {
            File source = new File( file );
            File target = new File( WorkspaceUtils.getSelectedFolder( )
                .getLocation( ).toOSString( )
                + IPath.SEPARATOR + source.getName( ) );

            if( target.exists( ) ) {
                if( GUIUtils.showMessageBox( Messages.MapUtils_4,
                    SWT.ICON_QUESTION | SWT.YES | SWT.NO ) == SWT.NO ) {
                    return;
                }
            }

            ResourceUtils.copyTo( source, target );
            WorkspaceUtils.getSelectedFolder( ).refreshLocal(
                IResource.DEPTH_INFINITE, null );
        } catch( Exception e ) {
            Logger.getInstance( ).logException( e );
        }
    }
}
