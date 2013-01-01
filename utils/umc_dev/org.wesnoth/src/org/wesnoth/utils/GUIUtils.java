/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.utils;

import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.MessageBox;
import org.eclipse.ui.IWorkbenchWindow;
import org.eclipse.ui.console.ConsolePlugin;
import org.eclipse.ui.console.IConsole;
import org.eclipse.ui.console.IConsoleManager;
import org.eclipse.ui.console.MessageConsole;

import org.wesnoth.Logger;

/**
 * Utilities class to handle the GUI. It can show message boxes in a
 * multithreading-safe manner.
 */
public class GUIUtils
{
    /**
     * Shows an information message box with the specified message (thread-safe)
     * 
     * @param message
     *        the message to show
     * @return The return code of the message box.
     */
    public static int showInfoMessageBox( final String message )
    {
        return showMessageBox( WorkspaceUtils.getWorkbenchWindow( ), message,
            SWT.ICON_INFORMATION );
    }

    /**
     * Shows an information message box with the specified message (thread-safe)
     * 
     * @param message
     *        the message to show
     * @return The return code of the message box.
     */
    public static int showWarnMessageBox( final String message )
    {
        return showMessageBox( WorkspaceUtils.getWorkbenchWindow( ), message,
            SWT.ICON_WARNING );
    }

    /**
     * Shows an error message box with the specified message (thread-safe)
     * 
     * @param message
     *        the message to show
     * @return The return code of the message box.
     */
    public static int showErrorMessageBox( final String message )
    {
        return showMessageBox( WorkspaceUtils.getWorkbenchWindow( ), message,
            SWT.ICON_ERROR );
    }

    /**
     * Shows a message box with the specified message and style(thread-safe)
     * 
     * @param message
     *        the message to show
     * @param style
     *        the style of the messageBox
     * @return The return code of the message box.
     */
    public static int showMessageBox( final String message, final int style )
    {
        return showMessageBox( WorkspaceUtils.getWorkbenchWindow( ), message,
            style );
    }

    /**
     * Shows a message box with the specified message (thread-safe)
     * 
     * @param window
     *        the window where to show the message box
     * @param message
     *        the message to show
     * @return The return code of the message box.
     */
    public static int showMessageBox( final IWorkbenchWindow window,
        final String message )
    {
        return showMessageBox( window, message, SWT.ICON_INFORMATION );
    }

    /**
     * Shows a message box with the specified message (thread-safe)
     * 
     * @param window
     *        the window where to show the message box
     * @param message
     *        the message to show
     * @param style
     *        The style of the message box.
     * @return The return code of the message box.
     */
    public static int showMessageBox( final IWorkbenchWindow window,
        final String message, final int style )
    {
        if( window == null || window.getShell( ) == null || message == null ) {
            return - 1;
        }

        RunnableWithResult< Integer > runnable = new RunnableWithResult< Integer >( ) {
            @Override
            public void run( )
            {
                MessageBox box = new MessageBox( window.getShell( ), style );
                box.setMessage( message );
                setResult( box.open( ) );
            }
        };

        try {
            window.getShell( ).getDisplay( ).syncExec( runnable );
            return runnable.getResult( );
        } catch( Exception e ) {
            Logger.getInstance( ).logException( e );
            return - 1;
        }
    }

    /**
     * Creates and returns a console with the specified parameters
     * 
     * @param consoleTitle
     *        The title of the console
     * @param imageDescriptor
     *        The image Descriptor
     * @param activate
     *        True to activate the console
     * @return A {@link MessageConsole} instance.
     */
    public static MessageConsole createConsole( String consoleTitle,
        ImageDescriptor imageDescriptor, boolean activate )
    {
        MessageConsole console = null;
        IConsoleManager conMan = ConsolePlugin.getDefault( )
            .getConsoleManager( );
        IConsole[] existing = conMan.getConsoles( );
        for( int i = 0; i < existing.length; i++ ) {
            if( consoleTitle.equals( existing[i].getName( ) ) ) {
                console = ( MessageConsole ) existing[i];
                console.clearConsole( );
                break;
            }
        }

        if( console == null ) // console not found
        {
            console = new MessageConsole( consoleTitle, imageDescriptor );
            conMan.addConsoles( new IConsole[] { console } );
        }
        if( activate ) {
            console.activate( );
        }
        return console;
    }
}
