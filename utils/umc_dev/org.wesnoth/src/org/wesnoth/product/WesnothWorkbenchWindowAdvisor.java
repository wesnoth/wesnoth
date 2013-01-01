/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.product;

import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.NullProgressMonitor;
import org.eclipse.ui.application.ActionBarAdvisor;
import org.eclipse.ui.application.IActionBarConfigurer;
import org.eclipse.ui.application.IWorkbenchWindowConfigurer;
import org.eclipse.ui.application.WorkbenchWindowAdvisor;

import org.wesnoth.Logger;

/**
 * Workbench advisor for the UMC IDE
 */
public class WesnothWorkbenchWindowAdvisor extends WorkbenchWindowAdvisor
{
    /**
     * Creates a new {@link WesnothWorkbenchAdvisor}
     * 
     * @param configurer
     *        The configurer used to configure the workbench
     */
    public WesnothWorkbenchWindowAdvisor( IWorkbenchWindowConfigurer configurer )
    {
        super( configurer );
    }

    @Override
    public ActionBarAdvisor createActionBarAdvisor(
        IActionBarConfigurer configurer )
    {
        return new WesnothActionBarAdvisor( configurer );
    }

    @Override
    public void preWindowOpen( )
    {
        super.preWindowOpen( );
        IWorkbenchWindowConfigurer configurer = getWindowConfigurer( );
        configurer.setShowMenuBar( true );
        configurer.setShowProgressIndicator( true );
        configurer.setShowStatusLine( true );
        configurer.setShowPerspectiveBar( true );
        configurer.setShowFastViewBars( true );
        configurer.setShowCoolBar( true );
    }

    @Override
    public void postWindowCreate( )
    {
        getWindowConfigurer( ).getWindow( ).getActivePage( )
            .hideActionSet( "org.eclipse.ui.run" ); //$NON-NLS-1$
    }

    @Override
    public boolean preWindowShellClose( )
    {
        try {
            ResourcesPlugin.getWorkspace( ).save( true,
                new NullProgressMonitor( ) );
        } catch( CoreException e ) {
            Logger.getInstance( ).logException( e );
        }
        return true;
    }
}
