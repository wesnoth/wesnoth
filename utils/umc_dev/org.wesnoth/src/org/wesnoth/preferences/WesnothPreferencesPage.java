/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.preferences;

import java.io.File;

import org.osgi.service.prefs.BackingStoreException;

import org.eclipse.core.runtime.Platform;
import org.eclipse.core.runtime.preferences.IEclipsePreferences;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.events.SelectionListener;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Label;

import org.wesnoth.Logger;
import org.wesnoth.Messages;
import org.wesnoth.WesnothPlugin;
import org.wesnoth.utils.GUIUtils;
import org.wesnoth.utils.ResourceUtils;
import org.wesnoth.utils.WorkspaceUtils;

/**
 * The default Wesnoth Preferences Page
 */
public class WesnothPreferencesPage extends AbstractPreferencePage
{
    /**
     * Creates a grid-style {@link WesnothPreferencesPage}
     */
    public WesnothPreferencesPage( )
    {
        super( GRID );

        setPreferenceStore( WesnothPlugin.getDefault( ).getPreferenceStore( ) );
        setDescription( Messages.WesnothPreferencesPage_0 );
    }

    @Override
    public void createFieldEditors( )
    {
    }

    @Override
    protected Control createContents( Composite parent )
    {
        Composite composite = new Composite( parent, SWT.NONE );
        composite.setLayout( new GridLayout( 2, false ) );

        Label lblPlugin = new Label( composite, SWT.NONE );
        lblPlugin.setText( "Reset all plugin's preferences: " );

        Button buttonPlugin = new Button( composite, SWT.NONE );
        buttonPlugin.setText( "Reset" );
        buttonPlugin.addSelectionListener( new SelectionListener( ) {

            @Override
            public void widgetSelected( SelectionEvent e )
            {
                if( GUIUtils
                    .showMessageBox(
                        "Are you sure you want to clear the plugin preferences?",
                        SWT.YES | SWT.NO ) == SWT.NO ) {
                    return;
                }

                // clear the preferences
                IEclipsePreferences root = Platform.getPreferencesService( )
                    .getRootNode( );
                try {
                    for( String rootName: root.childrenNames( ) ) {

                        org.osgi.service.prefs.Preferences childNode = root
                            .node( rootName );
                        for( String childName: childNode.childrenNames( ) ) {

                            org.osgi.service.prefs.Preferences node = childNode
                                .node( childName );

                            if( childName.startsWith( "org.wesnoth" ) ) {
                                try {
                                    node.clear( );
                                    node.flush( );
                                    node.sync( );
                                } catch( BackingStoreException e1 ) {
                                    Logger.getInstance( ).logException( e1 );
                                }
                            }

                        }
                    }

                    Preferences.initializeToDefault( );
                } catch( BackingStoreException e1 ) {
                    e1.printStackTrace( );
                }

                // clear the plugin's dirs
                File pluginDir = WesnothPlugin.getDefault( ).getStateLocation( )
                    .toFile( ).getParentFile( );
                ResourceUtils.deleteDirectory( pluginDir.getAbsolutePath( )
                    + "/org.wesnoth" );
                ResourceUtils.deleteDirectory( pluginDir.getAbsolutePath( )
                    + "/org.wesnoth.ui" );

                // clear the temporary files
                File[] files = new File( WorkspaceUtils.getTemporaryFolder( ) )
                    .listFiles( );

                for( File file: files ) {
                    // don't remove the logs
                    if( file.isDirectory( ) && file.getName( ).equals( "logs" ) ) {
                        continue;
                    }

                    ResourceUtils.deleteDirectory( file );
                }
            }

            @Override
            public void widgetDefaultSelected( SelectionEvent e )
            {
            }
        } );

        return super.createContents( parent );
    }
}
