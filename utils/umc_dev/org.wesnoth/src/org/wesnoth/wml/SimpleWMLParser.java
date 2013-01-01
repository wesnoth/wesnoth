/*******************************************************************************
 * Copyright (c) 2011 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.wml;

import com.google.common.base.Preconditions;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.eclipse.core.resources.IFile;
import org.eclipse.emf.common.util.TreeIterator;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.xtext.nodemodel.ICompositeNode;
import org.eclipse.xtext.nodemodel.util.NodeModelUtils;
import org.eclipse.xtext.parser.IParseResult;
import org.eclipse.xtext.parser.IParser;

import org.wesnoth.Logger;
import org.wesnoth.preprocessor.Define;
import org.wesnoth.projects.ProjectCache;
import org.wesnoth.utils.ResourceUtils;
import org.wesnoth.utils.WMLUtils;
import org.wesnoth.wml.WMLVariable.Scope;

/**
 * A simple WML Parser that parses a xtext WML Config file resource
 */
public class SimpleWMLParser
{
    private WMLConfig             config_;
    private WMLRoot               root_;
    private ProjectCache          projectCache_;
    private int                   dependencyIndex_;
    private String                currentFileLocation_;

    private Map< String, Define > defines_;

    /**
     * Creates a new parser for the specified file
     * 
     * @param file
     *        The file which to parse
     */
    public SimpleWMLParser( IFile file )
    {
        this( file,
            new WMLConfig( file.getProjectRelativePath( ).toString( ) ),
            null );
    }

    /**
     * Creates a new parser and fills the specified config file
     * 
     * @param file
     *        The file which to parse
     * @param config
     *        The config to fill
     * @param projCache
     *        The project cache (can be null) on which to reflect
     *        the parsed data
     */
    public SimpleWMLParser( IFile file, WMLConfig config, ProjectCache projCache )
    {
        config_ = Preconditions.checkNotNull( config );
        root_ = WMLUtils.getWMLRoot( file );
        projectCache_ = projCache;

        dependencyIndex_ = ResourceUtils.getDependencyIndex( file );

        currentFileLocation_ = file.getLocation( ).toOSString( );

        defines_ = new HashMap< String, Define >( );
    }

    /**
     * Creates a new parser that can be called on files outside the workspace
     * 
     * @param file
     *        The file to parse
     * @param parser
     *        The parser to use when parsing the file
     * @throws FileNotFoundException
     */
    public SimpleWMLParser( File file, IParser parser )
        throws FileNotFoundException
    {
        projectCache_ = null;

        IParseResult result = parser.parse( new FileReader( file ) );
        root_ = ( WMLRoot ) result.getRootASTElement( );

        config_ = new WMLConfig( file.getAbsolutePath( ) );

        currentFileLocation_ = file.getAbsolutePath( );

        defines_ = new HashMap< String, Define >( );
    }

    /**
     * Parses the config. The resulted config will be available in
     * {@link #getParsedConfig()}
     */
    public void parse( )
    {
        if( root_ == null ) {
            return;
        }

        WMLTag currentTag = null;
        String currentTagName = "";
        String textdomain = "";

        // clear previous parsed info
        config_.getWMLTags( ).clear( );
        config_.getEvents( ).clear( );
        defines_.clear( );


        TreeIterator< EObject > itor = root_.eAllContents( );
        // nothing to parse!
        if( ! itor.hasNext( ) ) {
            return;
        }

        while( itor.hasNext( ) ) {
            EObject object = itor.next( );

            if( object instanceof WMLTag ) {
                currentTag = ( WMLTag ) object;
                currentTagName = currentTag.getName( );

                if( currentTagName != null ) {
                    if( currentTagName.equals( "scenario" ) ) {
                        config_.IsScenario = true;
                    }
                    else if( currentTagName.equals( "campaign" ) ) {
                        config_.IsCampaign = true;
                    }
                    else if( currentTagName.equals( "preproc_define" ) ) {
                        addNewDefine( currentTag );
                    }
                }
            }
            else if( object instanceof WMLKey ) {
                if( currentTag != null && currentTagName != null ) {
                    WMLKey key = ( WMLKey ) object;
                    String keyName = key.getName( );

                    if( keyName.equals( "id" ) ) {
                        if( currentTagName.equals( "scenario" ) ) {
                            config_.ScenarioId = WMLUtils.getKeyValue( key
                                .getValues( ) );
                        }
                        else if( currentTagName.equals( "campaign" ) ) {
                            config_.CampaignId = WMLUtils.getKeyValue( key
                                .getValues( ) );
                        }
                    }
                    else if( keyName.equals( "name" ) ) {
                        if( currentTagName.equals( "set_variable" )
                            || currentTagName.equals( "set_variables" ) ) {
                            handleSetVariable( object );
                        }
                        else if( currentTagName.equals( "clear_variable" )
                            || currentTagName.equals( "clear_variables" ) ) {
                            handleUnsetVariable( object );
                        }
                        else if( currentTagName.equals( "event" ) ) {
                            String eventName = key.getValue( );

                            if( eventName.charAt( 0 ) == '"' ) {
                                eventName = eventName.substring( 1 );
                            }

                            if( eventName.charAt( eventName.length( ) - 1 ) == '"' ) {
                                eventName = eventName.substring( 0,
                                    eventName.length( ) - 1 );
                            }

                            config_.getEvents( ).add( eventName );
                        }
                    }
                }
            }
            else if( object instanceof WMLMacroCall ) {
                WMLMacroCall macroCall = ( WMLMacroCall ) object;
                String macroCallName = macroCall.getName( );
                if( macroCallName.equals( "VARIABLE" ) ) {
                    handleSetVariable( object );
                }
                else if( macroCallName.equals( "CLEAR_VARIABLE" ) ) {
                    handleUnsetVariable( object );
                }
            }
            else if( object instanceof WMLLuaCode ) {
                SimpleLuaParser luaParser = new SimpleLuaParser(
                    currentFileLocation_,
                    NodeModelUtils.getNode( object ).getStartLine( ),
                    ( ( WMLLuaCode ) object ).getValue( ) );
                luaParser.parse( );

                config_.getWMLTags( ).putAll( luaParser.getTags( ) );
            }
            else if( object instanceof WMLMacroDefine ) {
                addNewDefine( ( WMLMacroDefine ) object, textdomain );
            }
            else if( object instanceof WMLTextdomain ) {
                textdomain = ( ( WMLTextdomain ) object ).getName( );
                if( textdomain == null ) {
                    textdomain = "";
                }
            }
        }
    }

    /**
     * Creates a new define from a macro define
     * 
     * @param object
     */
    private void addNewDefine( WMLMacroDefine object, String textdomain )
    {
        ICompositeNode node = NodeModelUtils.getNode( object );
        if( node == null ) {
            return;
        }

        try {
            String defineHeader = object.getName( ).replaceAll( "\\r|\\n|\\t", "" );
            int firstSpaceIndex = defineHeader.indexOf( ' ' ) + 1;
            int defineNameEndIndex = defineHeader.indexOf( ' ', firstSpaceIndex + 1 );

            String name = defineHeader.substring( firstSpaceIndex, defineNameEndIndex );

            int linenum = node.getTotalStartLine( );
            String location = currentFileLocation_;

            String[] splittedArgs = defineHeader.substring( defineNameEndIndex + 1 ).split( " " );
            List< String > args = new ArrayList< String >( );
            for( String arg: splittedArgs ) {
                args.add( arg );
            }

            StringBuffer value = new StringBuffer( );
            for( WMLValuedExpression expression: object.getExpressions( ) ) {
                ICompositeNode expressionNode = NodeModelUtils.getNode( expression );
                if( expressionNode != null ) {
                    value.append( expressionNode.getText( ) );
                }
            }

            Define define = new Define( name, value.toString( ), textdomain, linenum, location, args );
            defines_.put( name, define );
        } catch( Exception e ) {
            // some formatting exceptions. We don't care 'bout em.
        }
    }

    /**
     * Creates a new define from the specified definition tag
     * and adds it to the list of defines
     * 
     * @param currentTag
     *        The tag that contains the define definition
     */
    private void addNewDefine( WMLTag currentTag )
    {
        String defineName = "";
        String location = "";
        int linenum = 0;
        String textdomain = "";
        String value = "";
        List< String > args = new ArrayList< String >( );

        // parse general define information
        for( WMLKey key: currentTag.getWMLKeys( ) ) {
            String keyName = key.getName( );
            if( keyName.equals( "name" ) ) {
                defineName = getUnquotedString( key.getValue( ) );
            }
            else if( keyName.equals( "value" ) ) {
                value = getUnquotedString( key.getValue( ) );
            }
            else if( keyName.equals( "textdomain" ) ) {
                textdomain = getUnquotedString( key.getValue( ) );
            }
            else if( keyName.equals( "linenum" ) ) {
                linenum = Integer.valueOf( getUnquotedString( key.getValue( ) ) );
            }
            else if( keyName.equals( "location" ) ) {
                location = getUnquotedString( key.getValue( ) );
            }
        }

        // parse define arguments
        for( WMLTag arg: currentTag.getWMLTags( ) ) {
            for( WMLKey key: arg.getWMLKeys( ) ) {
                if( key.getName( ).equals( "name" ) ) {
                    args.add( getUnquotedString( key.getValue( ) ) );
                }
            }
        }

        defines_.put( defineName, new Define( defineName, value, textdomain,
            linenum, location, args ) );
    }

    private static String getUnquotedString( String str )
    {
        int startIndex = 0;
        int endIndex = str.length( );

        if( str.charAt( 0 ) == '"' ) {
            startIndex = 1;
        }
        if( str.charAt( endIndex - 1 ) == '"' ) {
            endIndex--;
        }

        return str.substring( startIndex, endIndex );
    }

    private String getVariableNameByContext( EObject context )
    {
        String variableName = null;

        if( context instanceof WMLKey ) {
            variableName = WMLUtils.getKeyValue( ( ( WMLKey ) context )
                .getValues( ) );
        }
        else if( context instanceof WMLMacroCall ) {
            WMLMacroCall macro = ( WMLMacroCall ) context;
            if( macro.getParameters( ).size( ) > 0 ) {
                variableName = WMLUtils.toString( macro.getParameters( )

                    .get( 0 ) );
            }
        }

        return variableName;
    }

    private void handleSetVariable( EObject context )
    {
        if( projectCache_ == null ) {
            return;
        }

        String variableName = getVariableNameByContext( context );

        if( variableName == null ) {
            Logger.getInstance( ).logWarn(
                "setVariable: couldn't get variable name from context:"
                    + context );
        }

        WMLVariable variable = projectCache_.getVariables( ).get( variableName );
        if( variable == null ) {
            variable = new WMLVariable( variableName );
            projectCache_.getVariables( ).put( variableName, variable );
        }

        int nodeOffset = NodeModelUtils.getNode( context ).getTotalOffset( );
        for( Scope scope: variable.getScopes( ) ) {
            if( scope.contains( dependencyIndex_, nodeOffset ) ) {
                return; // nothing to do
            }
        }

        // couldn't find any scope. Add a new one then.
        variable.getScopes( ).add( new Scope( dependencyIndex_, nodeOffset ) );
    }

    private void handleUnsetVariable( EObject context )
    {
        if( projectCache_ == null ) {
            return;
        }

        String variableName = getVariableNameByContext( context );
        if( variableName == null ) {
            Logger.getInstance( ).logWarn(
                "unsetVariable: couldn't get variable name from context:"
                    + context );
        }

        WMLVariable variable = projectCache_.getVariables( ).get( variableName );
        if( variable == null ) {
            return;
        }

        int nodeOffset = NodeModelUtils.getNode( context ).getTotalOffset( );

        // get the first containing scope, and modify its end index/offset

        for( Scope scope: variable.getScopes( ) ) {
            if( scope.contains( dependencyIndex_, nodeOffset ) ) {

                scope.EndIndex = dependencyIndex_;
                scope.EndOffset = nodeOffset;

                return;
            }
        }
    }

    /**
     * Returns the parsed WMLConfig
     * 
     * @return Returns the parsed WMLConfig
     */
    public WMLConfig getParsedConfig( )
    {
        return config_;
    }

    /**
     * Returns the parsed Defines
     * 
     * @return Returns the parsed Defines
     */
    public Map< String, Define > getDefines( )
    {
        return defines_;
    }
}
