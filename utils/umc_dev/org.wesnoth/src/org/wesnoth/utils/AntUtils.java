/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.utils;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.PrintStream;
import java.util.Iterator;
import java.util.Map;
import java.util.Map.Entry;

import org.apache.tools.ant.BuildLogger;
import org.apache.tools.ant.DefaultLogger;
import org.apache.tools.ant.Project;
import org.apache.tools.ant.ProjectHelper;

import org.wesnoth.Logger;


/**
 * An util class that handles with ant files
 */
public class AntUtils
{
    /**
     * Runs the specified ant file, and returns the output of the runned file
     * 
     * @param antFile
     * @param properties
     *        the hasmap with userproperties to be added to the ant file
     * @param recordOutput
     *        true if the output of the runned file should be recorded and
     *        returned
     * @return null if the build didn't success
     */
    public static String runAnt( String antFile,
        Map< String, String > properties, boolean recordOutput )
    {
        Project project = new Project( );
        ByteArrayOutputStream out = null;

        try {
            out = new ByteArrayOutputStream( );
            if( recordOutput ) {
                project.addBuildListener( AntUtils.createLogger( out ) );
            }

            project.init( );
            File buildFile = new File( antFile );
            ProjectHelper.configureProject( project, buildFile );

            Iterator< Entry< String, String >> iterator = properties.entrySet( )
                .iterator( );
            while( iterator.hasNext( ) ) {
                Entry< String, String > key = iterator.next( );
                project.setUserProperty( key.getKey( ), key.getValue( ) );
            }
            project.executeTarget( project.getDefaultTarget( ) );

            return out.toString( );
        } catch( Exception e ) {
            Logger.getInstance( ).logException( e );
            return null;
        }
    }

    /**
     * Creates the default build logger for sending build events to the ant log.
     */
    private static BuildLogger createLogger( ByteArrayOutputStream out )
    {
        DefaultLogger logger = new DefaultLogger( );

        logger.setMessageOutputLevel( Project.MSG_INFO );
        logger.setOutputPrintStream( new PrintStream( out ) );
        logger.setErrorPrintStream( new PrintStream( out ) );
        logger.setEmacsMode( false );

        return logger;
    }
}
