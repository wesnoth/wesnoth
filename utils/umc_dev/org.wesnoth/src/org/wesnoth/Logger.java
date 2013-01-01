/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.text.SimpleDateFormat;
import java.util.Date;

import javax.swing.JOptionPane;

import org.eclipse.core.runtime.IStatus;

import org.wesnoth.utils.GUIUtils;
import org.wesnoth.utils.WorkspaceUtils;

/**
 * A class that logs activities in a file
 */
public class Logger
{
    private static class LoggerInstance
    {
        private static Logger instance_ = new Logger( );
    }

    private Logger( )
    {
    }

    private BufferedWriter logWriter_;
    private BufferedWriter toolLaunchLogWriter_;

    /**
     * Returns the singleton instance
     * 
     * @return An {@link Logger} singleton instance
     */
    public static Logger getInstance( )
    {
        return LoggerInstance.instance_;
    }

    /**
     * Starts the logger - creates the log file in the temporary directory
     */
    public void startLogger( )
    {
        if( logWriter_ != null ) {
            return;
        }
        try {
            if( WorkspaceUtils.getTemporaryFolder( ) == null ) {
                throw new IOException( "Could not create the temporary folder." ); //$NON-NLS-1$
            }

            String logFilePath = String.format( "%s/logs/log%s.txt", //$NON-NLS-1$
                WorkspaceUtils.getTemporaryFolder( ),
                WorkspaceUtils.getCurrentDateTime( ) );
            String toolsLogFilePath = String.format( "%s/logs/tools_log%s.txt", //$NON-NLS-1$
                WorkspaceUtils.getTemporaryFolder( ),
                WorkspaceUtils.getCurrentDateTime( ) );

            new File( WorkspaceUtils.getTemporaryFolder( ) + "/logs/" ).mkdirs( ); //$NON-NLS-1$

            logWriter_ = new BufferedWriter( new FileWriter( logFilePath ) );
            log( "Logging started." ); //$NON-NLS-1$
            log( "Error codes: 1 - INFO, 2 - WARNING, 4 - ERROR" ); //$NON-NLS-1$

            toolLaunchLogWriter_ = new BufferedWriter( new FileWriter(
                toolsLogFilePath ) );
            logTool( "Logging started." ); //$NON-NLS-1$
            logTool( "Error codes: 1 - INFO, 2 - WARNING, 4 - ERROR" ); //$NON-NLS-1$

        } catch( IOException e ) {
            JOptionPane
                .showMessageDialog(
                    null,
                    "There was an error trying to open the log." + e.getMessage( ) ); //$NON-NLS-1$
            e.printStackTrace( );
        }
    }

    /**
     * Stops the logger
     */
    public void stopLogger( )
    {
        if( logWriter_ == null ) {
            return;
        }
        try {
            log( "Logging ended." ); //$NON-NLS-1$
            logTool( "Logging Ended" );
            logWriter_.close( );
            toolLaunchLogWriter_.close( );
        } catch( IOException e ) {
            e.printStackTrace( );
        }
    }

    /**
     * Prints a message to the error log (severity: info)
     * 
     * @param message
     *        The message to log
     */
    public void log( String message )
    {
        log( message, IStatus.INFO );
    }

    /**
     * Logs a warning message
     * 
     * @param message
     *        The message to log
     */
    public void logWarn( String message )
    {
        log( message, IStatus.WARNING );
    }

    /**
     * Logs an error message
     * 
     * @param message
     *        The message to log
     */
    public void logError( String message )
    {
        log( message, IStatus.ERROR );
    }

    /**
     * Logs the specified exception, providing the stacktrace to the console
     * 
     * @param e
     *        The exception to log
     */
    public void logException( Exception e )
    {
        logExceptionToWriter( logWriter_, e );
    }

    /**
     * Logs the specified exception, providing the stacktrace to the console
     * 
     * @param e
     *        The exception to log
     */
    public void logToolException( Exception e )
    {
        logExceptionToWriter( toolLaunchLogWriter_, e );
    }

    private void logExceptionToWriter( BufferedWriter writer, Exception e )
    {
        if( e == null ) {
            return;
        }

        // put the stack trace in a string
        StringWriter sw = new StringWriter( );
        PrintWriter pw = new PrintWriter( sw );
        e.printStackTrace( pw );

        logToWriter( writer, e.getLocalizedMessage( ), IStatus.ERROR );
        logToWriter( writer, sw.toString( ), IStatus.ERROR );
    }

    /**
     * Logs the message (severity: info) showing also a messagebox to the user
     * 
     * @param message
     *        The message to log
     * @param guiMessage
     *        The message to show to the user
     */
    public void log( String message, String guiMessage )
    {
        log( message, IStatus.INFO );
        GUIUtils.showInfoMessageBox( guiMessage );
    }

    /**
     * Prints a message to the error log with the specified severity
     * 
     * @param message
     *        the message to log
     * @param severity
     *        the severity level from IStatus enum
     */
    public void log( String message, int severity )
    {
        logToWriter( logWriter_, message, severity );
    }

    /**
     * Prints a message to the tool launch log (severity: info)
     * 
     * @param message
     *        The message to log
     */
    public void logTool( String message )
    {
        logToWriter( toolLaunchLogWriter_, message, IStatus.INFO );
    }

    private void logToWriter( BufferedWriter writer, String message,
        int severity )
    {
        if( writer != null ) {
            try {
                writer
                    .write( String
                        .format(
                            "%s | %d | %s\n", //$NON-NLS-1$
                            new SimpleDateFormat( "yyyy-MM-dd HH:mm:ss" ).format( new Date( ) ), //$NON-NLS-1$
                            severity, message ) );
                writer.flush( );
            } catch( IOException e ) {
                e.printStackTrace( );
            }
        }

        // don't print to console the tools if there was no error/warning
        if( writer != toolLaunchLogWriter_
            || ( writer == toolLaunchLogWriter_ && severity != IStatus.INFO ) ) {
            System.out.println( message );
        }
    }
}
