/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.product;

import org.eclipse.equinox.app.IApplication;
import org.eclipse.equinox.app.IApplicationContext;
import org.eclipse.swt.widgets.Display;
import org.eclipse.ui.IWorkbench;
import org.eclipse.ui.PlatformUI;

import org.wesnoth.Logger;

/**
 * The main running point of the UMC IDE
 */
public class WesnothApplication implements IApplication
{
    @Override
    public Object start( IApplicationContext context )
    {
        Display display = PlatformUI.createDisplay( );
        Logger.getInstance( ).startLogger( );
        try {
            int returnCode = PlatformUI.createAndRunWorkbench( display,
                new WesnothWorkbenchAdvisor( ) );
            if( returnCode == PlatformUI.RETURN_RESTART ) {
                return IApplication.EXIT_RESTART;
            }
            else {
                return IApplication.EXIT_OK;
            }
        } catch( Exception e ) {
            Logger.getInstance( ).logException( e );
            return IApplication.EXIT_OK;
        } finally {
            display.dispose( );
        }
    }

    @Override
    public void stop( )
    {
        if( ! PlatformUI.isWorkbenchRunning( ) ) {
            return;
        }

        final IWorkbench workbench = PlatformUI.getWorkbench( );
        final Display display = workbench.getDisplay( );
        display.syncExec( new Runnable( ) {
            @Override
            public void run( )
            {
                if( ! display.isDisposed( ) ) {
                    workbench.close( );
                }
            }
        } );
    }
}
