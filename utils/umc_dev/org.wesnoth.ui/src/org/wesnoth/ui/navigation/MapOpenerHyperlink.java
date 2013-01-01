/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.navigation;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

import org.eclipse.swt.SWT;
import org.eclipse.xtext.ui.editor.hyperlinking.XtextHyperlink;

import org.wesnoth.Logger;
import org.wesnoth.templates.TemplateProvider;
import org.wesnoth.utils.GUIUtils;
import org.wesnoth.utils.GameUtils;

/**
 * A link that opens map files in the Wesnoth Map Editor
 */
public class MapOpenerHyperlink extends XtextHyperlink
{
    private String location_;

    /**
     * Sets the location of the map to open
     * 
     * @param location
     *        The new location
     */
    public void setLocation( String location )
    {
        location_ = location;
    }

    /**
     * @return The location of the map to open
     */
    public String getLocation( )
    {
        return location_;
    }

    @Override
    public void open( )
    {
        if( ! new File( location_ ).exists( ) ) {
            if( GUIUtils
                .showMessageBox(
                    "The map doesn't exist. Do you want to create a default one and open that?",
                    SWT.YES | SWT.NO ) == SWT.NO ) {
                return;
            }

            // go ahead, create the map
            FileWriter writer;
            try {
                writer = new FileWriter( location_ );
                writer.write( TemplateProvider.getInstance( ).getTemplate(
                    "map" ) );
                writer.close( );
            } catch( IOException e ) {
                Logger.getInstance( ).logException( e );
            }
        }

        GameUtils.startEditor( location_ );
    }
}
