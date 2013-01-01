/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.preferences;

import java.util.HashMap;
import java.util.Map;
import java.util.Map.Entry;

import org.eclipse.jface.preference.BooleanFieldEditor;
import org.eclipse.jface.preference.StringFieldEditor;
import org.eclipse.xtext.ui.editor.preferences.fields.LabelFieldEditor;

import org.wesnoth.Messages;
import org.wesnoth.WesnothPlugin;
import org.wesnoth.jface.RegexStringFieldEditor;

/**
 * Handles Addon Manager preferences
 */
public class AddonManagerPreferencePage extends AbstractPreferencePage
{
    /**
     * The id of this preference page
     */
    public final static String                ID_ADDON_PREFERENCE_PAGE = "org.wesnoth.preferences.AddonPage"; //$NON-NLS-1$

    /**
     * Holds the ports for each addon server as a (key, value) pair,
     * where:
     * - key = port
     * - value = wesnoth server version
     */
    public final static Map< String, String > ADDON_SERVER_PORTS       = new HashMap< String, String >( );

    static {
        ADDON_SERVER_PORTS.put( "15002", "1.9.x" ); //$NON-NLS-1$ //$NON-NLS-2$
        ADDON_SERVER_PORTS.put( "15001", "1.8.x" ); //$NON-NLS-1$ //$NON-NLS-2$
        ADDON_SERVER_PORTS.put( "15003", "1.6.x" ); //$NON-NLS-1$ //$NON-NLS-2$
        ADDON_SERVER_PORTS.put( "15005", "1.4.x" ); //$NON-NLS-1$ //$NON-NLS-2$
        ADDON_SERVER_PORTS.put( "15004", "trunk" ); //$NON-NLS-1$ //$NON-NLS-2$
    }

    /**
     * Creates a new Grid-type preference page
     */
    public AddonManagerPreferencePage( )
    {
        super( GRID );

        setPreferenceStore( WesnothPlugin.getDefault( ).getPreferenceStore( ) );
        setDescription( Messages.AddonManagerPreferencePage_10 );
    }

    @Override
    protected void createFieldEditors( )
    {
        addField(
            new StringFieldEditor( Preferences.ADDON_MANAGER_PASSWORD,
                Messages.AddonManagerPreferencePage_11,
                getFieldEditorParent( ) ),
            Messages.AddonManagerPreferencePage_12 );
        addField( new BooleanFieldEditor( Preferences.ADDON_MANAGER_VERBOSE,
            Messages.AddonManagerPreferencePage_13, 1,
            getFieldEditorParent( ) ) );

        addField(
            new RegexStringFieldEditor( Preferences.ADDON_MANAGER_ADDRESS,
                Messages.AddonManagerPreferencePage_14, "[^:]*",
                Messages.AddonManagerPreferencePage_16,
                getFieldEditorParent( ) ),
            Messages.AddonManagerPreferencePage_17 );

        StringBuilder ports = new StringBuilder( );
        StringBuilder portsRegex = new StringBuilder( );
        portsRegex.append( "(" ); //$NON-NLS-1$
        for( Entry< String, String > item: ADDON_SERVER_PORTS.entrySet( ) ) {
            portsRegex.append( item.getKey( ) + "|" ); //$NON-NLS-1$
            ports.append( String.format(
                "\t%s - %s\n", item.getKey( ), item.getValue( ) ) ); //$NON-NLS-1$
        }
        portsRegex.deleteCharAt( portsRegex.length( ) - 1 );
        portsRegex.append( ")*" ); //$NON-NLS-1$

        // System.out.println(portsRegex.toString());
        addField(
            new RegexStringFieldEditor( Preferences.ADDON_MANAGER_PORT,
                Messages.AddonManagerPreferencePage_22,
                portsRegex.toString( ),
                Messages.AddonManagerPreferencePage_23,
                getFieldEditorParent( ) ),
            Messages.AddonManagerPreferencePage_24 );
        addField( new LabelFieldEditor( Messages.AddonManagerPreferencePage_25
            + ports.toString( ), getFieldEditorParent( ) ) );
    }
}
