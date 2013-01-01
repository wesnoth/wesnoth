/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.schema;

import java.io.File;
import java.io.Serializable;
import java.util.Arrays;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Stack;

import org.wesnoth.Logger;
import org.wesnoth.installs.WesnothInstall;
import org.wesnoth.installs.WesnothInstallsUtils;
import org.wesnoth.preferences.Preferences;
import org.wesnoth.utils.ResourceUtils;
import org.wesnoth.utils.StringUtils;
import org.wesnoth.wml.WMLExpression;
import org.wesnoth.wml.WMLTag;
import org.wesnoth.wml.WmlFactory2;

/**
 * This is a 'schema.cfg' parser.
 */
public class SchemaParser
{
    protected final static Map< String, SchemaParser > parsers_ = new HashMap< String, SchemaParser >( );

    /**
     * Returns a SchemaParser instance based on the specified install
     * 
     * @param installName
     *        The name of the install
     * @return A SchemaParser singleton instance
     */
    public static SchemaParser getInstance( String installName )
    {
        // null not allowed
        if( installName == null ) {
            installName = ""; //$NON-NLS-1$
        }

        SchemaParser parser = parsers_.get( installName );
        if( parser == null ) {
            parser = new SchemaParser( installName );
            parser.parseSchema( false );
            parsers_.put( installName, parser );
        }

        return parser;
    }

    /**
     * Reloads all currently stored schemas
     * 
     * @param force
     *        True to force reloading schemas
     */
    public static void reloadSchemas( boolean force )
    {
        List< WesnothInstall > installs = WesnothInstallsUtils.getInstalls( );
        for( WesnothInstall install: installs ) {
            getInstance( install.getName( ) ).parseSchema( force );
        }
    }

    private Map< String, String > primitives_;
    private Map< String, WMLTag > tags_;
    private boolean               parsingDone_;
    private String                installName_;

    private SchemaParser( String installName )
    {
        installName_ = installName;
        primitives_ = new HashMap< String, String >( );
        tags_ = new HashMap< String, WMLTag >( );
        parsingDone_ = false;
    }

    /**
     * Parses the schema
     * 
     * @param force
     *        True to force parsing the schema, skipping the existing cache
     */
    public void parseSchema( boolean force )
    {
        parseSchemaFile( force, Preferences.getPaths( installName_ )
            .getSchemaPath( ) );
    }

    /**
     * Parses the schema
     * 
     * @param force
     *        True to force parsing the schema, skipping the existing cache
     * @param schemaPath
     *        The path to the 'schema.cfg' file
     */
    public void parseSchemaFile( boolean force, String schemaPath )
    {
        if( parsingDone_ && ! force ) {
            Logger.getInstance( ).log(
                "schema not parsed since there is already in cache." ); //$NON-NLS-1$
            return;
        }

        parsingDone_ = false;
        if( force ) {
            primitives_.clear( );
            tags_.clear( );
        }

        Logger.getInstance( ).log(
            "parsing schema " + ( force == true ? "forced": "" ) ); //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$
        File schemaFile = new File( schemaPath );
        String res = ResourceUtils.getFileContents( schemaFile );
        String[] lines = StringUtils.getLines( res );
        Stack< String > tagStack = new Stack< String >( );

        WMLTag currentTag = null;
        for( int index = 0; index < lines.length; index++ ) {
            String line = lines[index];
            // skip comments and empty lines
            if( StringUtils.startsWith( line, "#" )
                || line.matches( "^[\t ]*$" ) ) {
                continue;
            }

            if( StringUtils.startsWith( line, "[" ) ) //$NON-NLS-1$
            {
                // closing tag
                if( line.charAt( line.indexOf( "[" ) + 1 ) == '/' ) //$NON-NLS-1$
                {
                    // propagate the 'needsexpanding' property to upper levels
                    boolean expand = false;
                    if( ! tagStack.isEmpty( )
                        && tags_.containsKey( tagStack.peek( ) ) ) {
                        expand = tags_.get( tagStack.peek( ) )
                            .is_NeedingExpansion( );
                    }

                    tagStack.pop( );

                    if( ! tagStack.isEmpty( )
                        && tags_.containsKey( tagStack.peek( ) )
                        && expand == true ) {
                        tags_.get( tagStack.peek( ) ).set_NeedingExpansion(
                            expand );
                    }
                }
                // opening tag
                else {
                    String tagName = line.substring(
                        line.indexOf( "[" ) + 1, line.indexOf( "]" ) ); //$NON-NLS-1$ //$NON-NLS-2$
                    String simpleTagName = tagName;
                    String extendedTagName = ""; //$NON-NLS-1$
                    if( tagName.split( ":" ).length > 1 ) //$NON-NLS-1$
                    {
                        simpleTagName = tagName.split( ":" )[0]; //$NON-NLS-1$
                        extendedTagName = tagName.split( ":" )[1]; //$NON-NLS-1$
                    }
                    tagStack.push( simpleTagName );

                    if( ! tagName.equals( "description" ) ) //$NON-NLS-1$
                    {
                        if( tags_.containsKey( simpleTagName ) ) {
                            // this tags was already refered in the schema
                            // before they were declared
                            currentTag = tags_.get( simpleTagName );
                            currentTag.set_InhertedTagName( extendedTagName );
                            currentTag.set_NeedingExpansion( ! extendedTagName
                                .isEmpty( ) );
                        }
                        else {
                            WMLTag tag = WmlFactory2.eINSTANCE.createWMLTag(
                                simpleTagName, extendedTagName );
                            tag.set_NeedingExpansion( ! extendedTagName
                                .isEmpty( ) );
                            currentTag = tag;
                            tags_.put( simpleTagName, tag );
                        }
                    }
                }
            }
            else {
                // top level - primitives defined
                if( tagStack.peek( ).equals( "schema" ) ) //$NON-NLS-1$
                {
                    String[] tokens = line.split( "=" ); //$NON-NLS-1$
                    if( tokens.length != 2 ) {
                        Logger.getInstance( ).logError(
                            "Error. invalid primitive on line :" + index ); //$NON-NLS-1$
                        continue;
                    }

                    primitives_.put( tokens[0].trim( ),
                        StringUtils.trimQuotes( tokens[1].trim( ) ) );
                }
                else if( tagStack.peek( ).equals( "description" ) ) //$NON-NLS-1$
                {
                    String[] tokens = line.trim( ).split( "=" ); //$NON-NLS-1$
                    StringBuilder value = new StringBuilder( );

                    // this *should* happen only in [description]
                    // multi-line string
                    if( StringUtils.countOf( tokens[1], '"' ) % 2 != 0 ) {
                        value.append( tokens[1] + "\n" ); //$NON-NLS-1$
                        ++index;
                        while( StringUtils.countOf( lines[index], '"' ) % 2 == 0
                            && ! StringUtils.startsWith( lines[index], "#" ) && //$NON-NLS-1$
                            index < lines.length ) {
                            value.append( lines[index] + "\n" ); //$NON-NLS-1$
                            ++index;
                        }
                        value.append( lines[index] );
                    }
                    else {
                        value.append( tokens[1] );
                    }

                    // get rid of the quotes
                    if( value.length( ) >= 2 ) {
                        value = new StringBuilder( value.substring( 1,
                            value.length( ) - 1 ) );
                    }

                    if( currentTag != null ) {
                        currentTag.set_Description( value.toString( ) );
                    }
                    else {
                        System.out.println( "Tag shouldn't have been null!" ); //$NON-NLS-1$
                    }
                }
                else {
                    String tmpLine = line.trim( );
                    if( line.contains( "#" ) ) {
                        tmpLine = line
                            .substring( 0, line.lastIndexOf( "#" ) ).trim( ); //$NON-NLS-1$
                    }
                    String[] tokens = tmpLine.split( "=" ); //$NON-NLS-1$

                    if( tokens.length != 2 ) {
                        Logger.getInstance( ).logError(
                            "Error. invalid attribute on line :" + index ); //$NON-NLS-1$
                        continue;
                    }

                    String[] value = tokens[1].substring( 1,
                        tokens[1].length( ) - 1 ).split( " " ); //$NON-NLS-1$
                    if( value.length != 2 ) {
                        Logger
                            .getInstance( )
                            .logError(
                                "Error. invalid attribute value on line:" + index ); //$NON-NLS-1$
                        continue;
                    }

                    if( currentTag != null ) {
                        if( tokens[0].startsWith( "_" ) ) // reference to another tag //$NON-NLS-1$
                        {
                            WMLTag targetTag = null;
                            targetTag = tags_.get( value[1] );

                            // tag wasn't created yet
                            if( targetTag == null ) {
                                targetTag = WmlFactory2.eINSTANCE
                                    .createWMLTag( value[1], "",
                                        getCardinality( value[0] ) );
                                tags_.put( value[1], targetTag );
                            }

                            currentTag.getExpressions( ).add( targetTag );
                        }
                        else {
                            if( primitives_.get( value[1] ) == null ) {
                                Logger
                                    .getInstance( )
                                    .logError(
                                        "Undefined primitive type in schema.cfg for: " + value[1] ); //$NON-NLS-1$
                            }

                            currentTag.getExpressions( ).add(
                                WmlFactory2.eINSTANCE.createWMLKey(
                                    tokens[0],
                                    primitives_.get( value[1] ),
                                    getCardinality( value[0] ),
                                    value[1].equals( "tstring" ) ) );
                        }
                    }
                    else {
                        // System.err.println("can't find entry for: " +
                        // tagStack.peek());
                    }
                }
            }
        }

        for( WMLTag tag: tags_.values( ) ) {
            sortChildren( tag );
        }

        for( WMLTag tag: tags_.values( ) ) {
            expandTag( tag );
        }

        Logger.getInstance( ).log( "parsing done" ); //$NON-NLS-1$
        parsingDone_ = true;
    }

    /**
     * Expands the tags that need to (the ones based on inheritance)
     */
    private void expandTag( WMLTag tag )
    {
        if( tag.is_NeedingExpansion( ) ) {
            tag.set_NeedingExpansion( false );

            for( WMLTag subTag: tag.getWMLTags( ) ) {
                expandTag( subTag );
            }

            WMLTag inhertedTag = tags_.get( tag.get_InhertedTagName( ) );
            if( inhertedTag != null ) {
                tag.getExpressions( ).addAll( inhertedTag.getExpressions( ) );
            }
        }
    }

    /**
     * Sorts all tag's children by using the cardinality comparator
     * 
     * @param tag
     *        The tag to whom to sort the children
     */
    private void sortChildren( WMLTag tag )
    {
        WMLExpression[] expressions = tag.getExpressions( )
            .toArray( new WMLExpression[tag.getExpressions( ).size( )] );
        Arrays.sort( expressions, new CardinalityComparator( ) );
        tag.getExpressions( ).clear( );

        for( WMLExpression expression: expressions ) {
            tag.getExpressions( ).add( expression );
        }

        for( WMLTag subTag: tag.getWMLTags( ) ) {
            sortChildren( subTag );
        }
    }

    /**
     * Gets the map with parsed tags
     * 
     * @return A new {@link Map} instance, with a (String, WMLTag) pair
     */
    public Map< String, WMLTag > getTags( )
    {
        return tags_;
    }

    /**
     * Returns the cardinality as a character of the specified value
     * required = 1
     * optional = ?
     * repeated = *
     * forbidden = -
     * 
     * @param value
     *        The value the parse
     * @return The char representation of the cardinality
     */
    public char getCardinality( String value )
    {
        if( value.equals( "required" ) ) {
            return '1';
        }
        else if( value.equals( "optional" ) ) {
            return '?';
        }
        else if( value.equals( "repeated" ) ) {
            return '*';
        }
        else if( value.equals( "forbidden" ) ) {
            return '-';
        }
        return 'a';
    }

    /**
     * A WML Expression comparator that sorts just after required cardinality.
     * That is, after the sort the required wmlexpressions will be first
     */
    public static class CardinalityComparator implements
        Comparator< WMLExpression >, Serializable
    {
        private static final long serialVersionUID = 6103884038547449868L;

        @Override
        public int compare( WMLExpression o1, WMLExpression o2 )
        {
            char o1_card = o1.get_Cardinality( );
            char o2_card = o2.get_Cardinality( );

            if( o1_card == '1' && o2_card != '1' ) {
                return - 1;
            }
            else if( o2_card == '1' && o1_card != '1' ) {
                return 1;
            }

            return 0;
        }
    }
}
