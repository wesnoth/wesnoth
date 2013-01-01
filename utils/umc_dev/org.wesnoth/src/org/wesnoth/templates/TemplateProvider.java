/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.templates;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.wesnoth.Logger;
import org.wesnoth.WesnothPlugin;
import org.wesnoth.utils.Pair;
import org.wesnoth.utils.ResourceUtils;
import org.wesnoth.utils.StringUtils;

/**
 * A simple template provider class
 */
public class TemplateProvider
{
    /**
     * The filename of the file that contains the templates list
     */
    public static final String TEMPLATES_FILENAME = "templatesIndex.txt"; //$NON-NLS-1$

    /**
     * Holds the instance of the TemplateProvider.
     * This is based on the Initialization on demand holder idiom
     */
    private static class TemplateProviderInstance
    {
        private static final TemplateProvider instance_ = new TemplateProvider( );
    }

    private final Map< String, String >        templates_ = new HashMap< String, String >( );

    private final Map< String, List< String >> cacs_      = new HashMap< String, List< String >>( );

    private TemplateProvider( )
    {
        loadTemplates( );
        loadCACs( );
    }

    /**
     * Returns the singleton instance.
     * 
     * @return Returns the {@link TemplateProvider} singleton instance.
     */
    public static TemplateProvider getInstance( )
    {
        return TemplateProviderInstance.instance_;
    }

    /**
     * Loads the Content Assist Config files from the file system
     */
    public void loadCACs( )
    {
        cacs_.clear( );

        try {
            File varsFile = new File( WesnothPlugin.PLUGIN_FULL_PATH
                + "/templates/cac/variables.txt" );
            cacs_.put( "variables", Arrays.asList( StringUtils
                .getLines( ResourceUtils.getFileContents( varsFile, true,
                    true ) ) ) );

            File eventsFile = new File( WesnothPlugin.PLUGIN_FULL_PATH
                + "/templates/cac/events.txt" );
            cacs_.put( "events", Arrays.asList( StringUtils
                .getLines( ResourceUtils.getFileContents( eventsFile, true,
                    true ) ) ) );
        } catch( Exception e ) {
            Logger.getInstance( ).logException( e );
        }
    }

    /**
     * Loads the templates from the file system
     */
    public void loadTemplates( )
    {
        templates_.clear( );
        try {
            Logger.getInstance( ).log( "reading templates from: " + //$NON-NLS-1$
                WesnothPlugin.PLUGIN_FULL_PATH + TEMPLATES_FILENAME );

            BufferedReader reader = new BufferedReader( new FileReader(
                WesnothPlugin.PLUGIN_FULL_PATH + TEMPLATES_FILENAME ) );
            BufferedReader tmpReader;
            String line, tmpLine;
            StringBuilder content = new StringBuilder( );

            // read the main "templatesIndex.txt" file
            while( ( line = reader.readLine( ) ) != null ) {
                // comment
                if( line.startsWith( "#" ) || line.isEmpty( ) ) {
                    continue;
                }

                // 0 - template name | 1 - template file
                String[] tokensStrings = line.split( " " ); //$NON-NLS-1$

                if( tokensStrings.length != 2 ) {
                    Logger.getInstance( ).logWarn(
                        "TemplateIndex line " + line
                            + "is not properly formatted" );
                    continue;
                }

                content.setLength( 0 );

                if( new File( WesnothPlugin.PLUGIN_FULL_PATH + tokensStrings[1] )
                    .exists( ) ) {
                    tmpReader = new BufferedReader( new FileReader(
                        WesnothPlugin.PLUGIN_FULL_PATH + tokensStrings[1] ) );
                    while( ( tmpLine = tmpReader.readLine( ) ) != null ) {
                        content.append( tmpLine + '\n' );
                    }
                    templates_.put( tokensStrings[0], content.toString( ) );
                    tmpReader.close( );
                }
            }
            reader.close( );
        } catch( IOException e ) {
            Logger.getInstance( ).logException( e );
        }
    }

    /**
     * Gets a string of the processed specified template
     * 
     * @param templateName
     *        The name of the template to process
     * @param parameters
     *        The parameters to replace into the template
     * @return A {@link String} that represents the processed template.
     */
    public String getProcessedTemplate( String templateName,
        List< ReplaceableParameter > parameters )
    {
        String tmpTemplate = TemplateProvider.getInstance( ).getTemplate(
            templateName );
        if( tmpTemplate == null || parameters == null ) {
            return null;
        }

        StringBuilder result = new StringBuilder( );
        String[] template = StringUtils.getLines( tmpTemplate );
        boolean skipLine;

        for( int i = 0; i < template.length; ++i ) {
            skipLine = false;
            for( ReplaceableParameter param: parameters ) {
                if( template[i].contains( param.paramName ) ) {
                    template[i] = StringUtils.replaceWithIndent( template[i],
                        param.paramName, param.paramValue );

                    if( ! templateName.equals( "build_xml" ) && //$NON-NLS-1$
                        ( param.paramValue == null || param.paramValue
                            .isEmpty( ) ) ) {
                        // we don't have any value supplied -
                        // let's comment that line (if it's not already
                        // commented)
                        // or remove it if it's empty
                        if( ! ( StringUtils.startsWith( template[i], "#" ) ) ) //$NON-NLS-1$
                        {
                            template[i] = "#" + template[i]; //$NON-NLS-1$
                            Pattern pattern = Pattern.compile( "#[\t ]*" ); //$NON-NLS-1$
                            Matcher matcher = pattern.matcher( template[i] );
                            if( matcher.matches( ) ) {
                                skipLine = true;
                            }
                        }
                    }
                }
            }

            if( skipLine == false ) {
                result.append( template[i] + "\n" ); //$NON-NLS-1$
            }
        }
        return result.toString( );
    }

    /**
     * Returns the template with the specified name or empty string if none
     * 
     * @param name
     * @return A string that represents the queried template name.
     */
    public String getTemplate( String name )
    {
        String result = templates_.get( name );
        if( result == null ) {
            return ""; //$NON-NLS-1$
        }
        return result;
    }

    /**
     * Gets the Content Assist Config list for the specified type
     * 
     * @param type
     *        The type of the CAC
     * @return A list of String values. The returned list is read-only
     */
    public List< String > getCAC( String type )
    {
        List< String > result = cacs_.get( type );

        if( result == null ) {
            return new ArrayList< String >( );
        }

        return result;
    }

    /**
     * Gets the lists of the specified structure template. The first return
     * value is a list of <String, String> that consist of <Filename, Template
     * used for file contents> and the second return value is a list of String
     * with directories names
     * 
     * @param structureTemplate
     *        the template
     * @return A pair with the files and directories.
     */
    public Pair< List< Pair< String, String >>, List< String >> getFilesDirectories(
        String structureTemplate )
    {
        List< Pair< String, String >> files = new ArrayList< Pair< String, String >>( );
        List< String > dirs = new ArrayList< String >( );

        for( String line: StringUtils.getLines( structureTemplate ) ) {
            if( StringUtils.startsWith( line, "#" ) ) {
                continue;
            }

            if( line.contains( ":" ) ) // file with template //$NON-NLS-1$
            {
                String[] tmpLine = line.split( ":" ); //$NON-NLS-1$

                // oops. error
                if( tmpLine.length != 2 ) {
                    Logger
                        .getInstance( )
                        .logError(
                            String
                                .format(
                                    "error parsing 'structure template' (%s) on line %s", //$NON-NLS-1$
                                    structureTemplate, line ) );
                    continue;
                }

                files.add( new Pair< String, String >( tmpLine[0].trim( ),
                    tmpLine[1].trim( ) ) );
            }
            else {
                dirs.add( line.trim( ) );
            }
        }

        return new Pair< List< Pair< String, String >>, List< String >>( files,
            dirs );
    }
}
