/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.navigation;

import org.eclipse.core.resources.IFile;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.jface.text.Region;
import org.eclipse.xtext.nodemodel.ICompositeNode;
import org.eclipse.xtext.nodemodel.util.NodeModelUtils;
import org.eclipse.xtext.resource.XtextResource;
import org.eclipse.xtext.ui.editor.hyperlinking.HyperlinkHelper;
import org.eclipse.xtext.ui.editor.hyperlinking.IHyperlinkAcceptor;

import org.wesnoth.Logger;
import org.wesnoth.installs.WesnothInstallsUtils;
import org.wesnoth.preferences.Preferences;
import org.wesnoth.preferences.Preferences.Paths;
import org.wesnoth.preprocessor.Define;
import org.wesnoth.projects.ProjectUtils;
import org.wesnoth.ui.editor.WMLEditor;
import org.wesnoth.utils.WMLUtils;
import org.wesnoth.wml.WMLKey;
import org.wesnoth.wml.WMLMacroCall;

/**
 * Helper that creates hyperlinks at the offset if needed
 */
public class WMLHyperlinkHelper extends HyperlinkHelper
{
    @Override
    public void createHyperlinksByOffset( XtextResource resource, int offset,
        IHyperlinkAcceptor acceptor )
    {
        super.createHyperlinksByOffset( resource, offset, acceptor );

        EObject object = WMLUtils.resolveElementAt( resource, offset );

        if( object == null ) {
            return;
        }

        IFile file = WMLEditor.getActiveEditorFile( );
        if( file == null ) {
            Logger.getInstance( ).logError(
                "FATAL! file is null (and it shouldn't) " );
            return;
        }

        Paths paths = Preferences.getPaths( WesnothInstallsUtils
            .getInstallNameForResource( file ) );

        ICompositeNode node = NodeModelUtils.getNode( object );

        createMapHyperlink( paths, object, acceptor, node );
        createMacroHyperlink( paths, file, object, acceptor, node );
    }

    /**
     * Creates a hyperlink for opening the macro definition
     * 
     * @param paths
     *        The paths variable for the current install
     * @param file
     *        The current edited file
     * @param object
     *        The current object
     * @param acceptor
     *        The hyperlink acceptor
     * @param node
     *        The node model representation of the macro
     */
    private void createMacroHyperlink( Paths paths, IFile file, EObject object,
        IHyperlinkAcceptor acceptor, ICompositeNode node )
    {
        if( object instanceof WMLMacroCall == false ) {
            return;
        }

        WMLMacroCall macro = ( WMLMacroCall ) object;

        // get the define for the macro
        Define define = ProjectUtils.getCacheForProject( file.getProject( ) )
            .getDefines( ).get( macro.getName( ) );
        if( define == null || define.getLocation( ).length( ) <= 2 ) {
            return;
        }

        String filePath = define.getLocation( );

        if( filePath.charAt( 0 ) == '~' ) {
            // expand the '~' character to user data dir
            filePath = filePath.replaceFirst( "~", paths.getUserDataDir( ) );
        }
        else if( filePath.startsWith( "core/" ) ) {
            // expand the data/core path
            filePath = filePath.replaceFirst( "core/", paths.getCoreDir( ) );
        }

        FileLocationOpenerHyperlink macroTarget = new FileLocationOpenerHyperlink( );
        macroTarget.setHyperlinkRegion( new Region( node.getOffset( ), node
            .getLength( ) ) );
        macroTarget.setFilePath( filePath );
        macroTarget.setLinenumber( define.getLineNum( ) );
        acceptor.accept( macroTarget );
    }

    /**
     * Creates a hyperlink for opening the map ( if applying )
     * 
     * @param paths
     *        The paths variable for the current install
     * @param object
     *        The current object
     * @param acceptor
     *        The hyperlink acceptor
     * @param node
     *        The node model representation of the macro
     */
    private void createMapHyperlink( Paths paths, EObject object,
        IHyperlinkAcceptor acceptor, ICompositeNode node )
    {
        EObject container = object.eContainer( );

        if( ! ( container instanceof WMLKey )
            || ! ( ( WMLKey ) container ).getName( ).equals( "map_data" ) ) {
            return;
        }

        String mapLocation = node.getText( );

        // too few characters
        if( mapLocation.length( ) <= 2 ) {
            return;
        }

        // trim the " and the { (if any exist)
        int indexStart = 0;
        int indexEnd = 0;

        if( mapLocation.charAt( 0 ) == '"' ) {
            indexStart = 1;

            if( mapLocation.charAt( 1 ) == '{' ) {
                indexStart = 2;
            }
        }

        if( mapLocation.charAt( mapLocation.length( ) - 1 ) == '"' ) {
            indexEnd = mapLocation.length( ) - 1;

            if( mapLocation.charAt( mapLocation.length( ) - 2 ) == '}' ) {
                indexEnd = mapLocation.length( ) - 2;
            }
        }

        mapLocation = mapLocation.substring( indexStart, indexEnd );

        if( mapLocation.charAt( 0 ) == '~' ) {
            // expand the '~' character to user data dir
            mapLocation = mapLocation
                .replaceFirst( "~", paths.getUserDataDir( ) );
        }
        else if( mapLocation.startsWith( "campaigns/" ) ) {
            // expand the campaigns path
            mapLocation = mapLocation.replaceFirst( "campaigns/",
                paths.getCampaignDir( ) );
        }

        MapOpenerHyperlink hyperlink = new MapOpenerHyperlink( );
        hyperlink.setHyperlinkRegion( new Region( node.getOffset( ), node
            .getLength( ) ) );
        hyperlink.setLocation( mapLocation );
        acceptor.accept( hyperlink );
    }
}
