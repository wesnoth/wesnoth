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
import org.wesnoth.projects.ProjectCache;
import org.wesnoth.projects.ProjectUtils;
import org.wesnoth.utils.GUIUtils;
import org.wesnoth.utils.WorkspaceUtils;
import org.wesnoth.wml.WMLConfig;

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
        IProject project = container.getProject( );
        ProjectCache cache = ProjectUtils.getCacheForProject( project );

        String campaignId = "none"; //$NON-NLS-1$
        for( WMLConfig config: cache.getWMLConfigs( ).values( ) ) {
            if( ! config.IsCampaign ) {
                continue;
            }

            campaignId = ( config.CampaignId == null ? "invalid": config.CampaignId ); //$NON-NLS-1$
        }

        StringBuffer scenarios = new StringBuffer( );
        for( WMLConfig config: cache.getWMLConfigs( ).values( ) ) {
            if( ! config.IsScenario ) {
                continue;
            }

            if( scenarios.length( ) != 0 ) {
                scenarios.append( ", " ); //$NON-NLS-1$
            }

            scenarios.append( config.ScenarioId == null ? "invalid": config.ScenarioId ); //$NON-NLS-1$
        }

        int mapsCount = 0;
        File mapsFolder = new File( container.getLocation( ).toOSString( ) + "/maps" ); //$NON-NLS-1$
        if( mapsFolder.exists( ) ) {
            mapsCount = mapsFolder.listFiles( ).length;
        }

        int unitsCount = 0;
        File unitsFolder = new File( container.getLocation( ).toOSString( ) + "/units" ); //$NON-NLS-1$
        if( unitsFolder.exists( ) ) {
            unitsCount = unitsFolder.listFiles( ).length;
        }

        return String.format( Messages.WesnothProjectReport_4, campaignId,
            scenarios.toString( ), mapsCount, unitsCount );
    }
}
