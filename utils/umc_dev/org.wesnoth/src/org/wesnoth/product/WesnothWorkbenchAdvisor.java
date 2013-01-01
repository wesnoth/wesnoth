/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.product;

import org.eclipse.core.runtime.IAdaptable;
import org.eclipse.ui.application.IWorkbenchWindowConfigurer;
import org.eclipse.ui.application.WorkbenchWindowAdvisor;
import org.eclipse.ui.ide.IDE;

import org.wesnoth.utils.StringUtils;
import org.wesnoth.utils.WorkspaceUtils;
import org.wesnoth.views.WesnothProjectsExplorer;


/**
 * A class that creates the workbench in the product
 */
public class WesnothWorkbenchAdvisor extends WorkbenchAdvisorHack
{
    @Override
    public WorkbenchWindowAdvisor createWorkbenchWindowAdvisor(
        IWorkbenchWindowConfigurer configurer )
    {
        return new WesnothWorkbenchWindowAdvisor( configurer );
    }

    @Override
    public String getInitialWindowPerspectiveId( )
    {
        return WMLPerspective.ID_WMLPERSPECTIVE;
    }

    @Override
    public IAdaptable getDefaultPageInput( )
    {
        return new WesnothProjectsExplorer( );
    }

    @Override
    public void preStartup( )
    {
        IDE.registerAdapters( );
    }

    @Override
    public void postStartup( )
    {
        // if we are testing, don't setup workspace, because we are
        // setting the paths via the test suite
        boolean isTesting = ! StringUtils.isNullOrEmpty( System
            .getProperty( "isTesting" ) );

        if( ! isTesting
            && WorkspaceUtils.checkPathsAreSet( null, false ) == false ) {
            WorkspaceUtils.setupWorkspace( true );
        }
    }
}
