/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.product;

import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Menu;
import org.eclipse.swt.widgets.MenuItem;
import org.eclipse.ui.IPerspectiveDescriptor;
import org.eclipse.ui.IPerspectiveListener;
import org.eclipse.ui.IStartup;
import org.eclipse.ui.IWorkbenchPage;
import org.eclipse.ui.IWorkbenchWindow;
import org.eclipse.ui.PlatformUI;

import org.wesnoth.Messages;

/**
 * A class that wipes irrelevant action for the UMC IDE
 */
public class ActionWiper implements IStartup, IPerspectiveListener
{
    private static final String[] ACTIONS_2_WIPE = new String[] {
        "org.eclipse.search.searchActionSet", //$NON-NLS-1$
        "org.eclipse.debug.ui.breakpointActionSet", //$NON-NLS-1$
        "org.eclipse.debug.ui.debugActionSet", //$NON-NLS-1$
        "org.eclipse.debug.ui.launchActionSet", //$NON-NLS-1$
        "org.eclipse.debug.ui.profileActionSet", //$NON-NLS-1$
        "org.eclipse.ui.externaltools.ExternalToolsSet" //$NON-NLS-1$
                                                 // "org.eclipse.ui.edit.text.actionSet.presentation",
                                                     // "org.eclipse.ui.edit.text.actionSet.openExternalFile",
                                                     // "org.eclipse.ui.edit.text.actionSet.annotationNavigation",
                                                     // "org.eclipse.ui.edit.text.actionSet.navigation",
                                                     // "org.eclipse.ui.edit.text.actionSet.convertLineDelimitersTo",
                                                     // "org.eclipse.update.ui.softwareUpdates"
                                                 };

    @Override
    public void earlyStartup( )
    {
        IWorkbenchWindow[] windows = PlatformUI.getWorkbench( )
            .getWorkbenchWindows( );
        for( int i = 0; i < windows.length; i++ ) {
            IWorkbenchPage page = windows[i].getActivePage( );
            if( page != null ) {
                wipeActions( page );
            }
            windows[i].addPerspectiveListener( this );
        }
    }

    private void wipeActions( final IWorkbenchPage page )
    {
        Display.getDefault( ).syncExec( new Runnable( ) {
            @Override
            public void run( )
            {
                // remove the run menu
                Menu menu = page.getWorkbenchWindow( ).getShell( ).getMenuBar( );
                for( MenuItem item: menu.getItems( ) ) {
                    if( item.getText( ).equals( Messages.ActionWiper_6 ) ) {
                        item.dispose( );
                    }
                }

                for( int i = 0; i < ACTIONS_2_WIPE.length; i++ ) {
                    page.hideActionSet( ACTIONS_2_WIPE[i] );
                }
            }
        } );
    }

    @Override
    public void perspectiveActivated( IWorkbenchPage page,
        IPerspectiveDescriptor perspective )
    {
        wipeActions( page );
    }

    @Override
    public void perspectiveChanged( IWorkbenchPage page,
        IPerspectiveDescriptor perspective, String changeId )
    {
    }
}
