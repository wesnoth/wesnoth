/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.action;

import java.io.File;

import org.eclipse.core.resources.IContainer;
import org.eclipse.core.resources.IProject;
import org.eclipse.jface.action.IAction;

import org.wesnoth.Messages;
import org.wesnoth.utils.GUIUtils;
import org.wesnoth.utils.WorkspaceUtils;

/**
 * Shows a simple project report.
 */
public class WesnothProjectReport extends ObjectActionDelegate
{
    @Override
    public void run( IAction action )
    {
        IProject project = WorkspaceUtils.getSelectedProject( );
        if( project == null ) {
            GUIUtils.showWarnMessageBox( Messages.WesnothProjectReport_0 );
            return;
        }

        GUIUtils.showInfoMessageBox( getReportForContainer( project ) );
    }

    /**
     * Gets the report for specified container (sceanarios, maps, units)
     * 
     * @param container
     * @return
     */
    private String getReportForContainer( IContainer container )
    {
        int[] statistics = new int[3];

        File scenariosFolder = new File( container.getLocation( ).toOSString( )
            + "/scenarios" ); //$NON-NLS-1$
        if( scenariosFolder.exists( ) ) {
            statistics[0] = scenariosFolder.listFiles( ).length;
        }

        File mapsFolder = new File( container.getLocation( ).toOSString( )
            + "/maps" ); //$NON-NLS-1$
        if( mapsFolder.exists( ) ) {
            statistics[1] = mapsFolder.listFiles( ).length;
        }

        File unitsFolder = new File( container.getLocation( ).toOSString( )
            + "/units" ); //$NON-NLS-1$
        if( unitsFolder.exists( ) ) {
            statistics[2] = unitsFolder.listFiles( ).length;
        }

        return String.format( Messages.WesnothProjectReport_4, statistics[0],
            statistics[1], statistics[2] );
    }
}
