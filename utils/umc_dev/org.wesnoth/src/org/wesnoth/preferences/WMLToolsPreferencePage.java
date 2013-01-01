/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.preferences;

import org.eclipse.jface.preference.BooleanFieldEditor;
import org.eclipse.jface.preference.ScaleFieldEditor;
import org.eclipse.xtext.ui.editor.preferences.fields.LabelFieldEditor;

import org.wesnoth.Messages;
import org.wesnoth.WesnothPlugin;

/**
 * Preference page that handles the WML Tools
 */
public class WMLToolsPreferencePage extends AbstractPreferencePage
{
    /**
     * Creates a new grid-style {@link WMLToolsPreferencePage}
     */
    public WMLToolsPreferencePage( )
    {
        super( GRID );

        setPreferenceStore( WesnothPlugin.getDefault( ).getPreferenceStore( ) );
        setDescription( Messages.WMLToolsPreferencePage_0 );
    }

    @Override
    protected void createFieldEditors( )
    {
        addField( new LabelFieldEditor( Messages.WMLToolsPreferencePage_1,
            getFieldEditorParent( ) ) );
        addField( new BooleanFieldEditor( Preferences.WMLINDENT_DRYRUN,
            Messages.WMLToolsPreferencePage_2, 1, getFieldEditorParent( ) ) );
        addField( new BooleanFieldEditor( Preferences.WMLINDENT_VERBOSE,
            Messages.WMLToolsPreferencePage_3, 1, getFieldEditorParent( ) ) );
        addField( new LabelFieldEditor( "", getFieldEditorParent( ) ) ); //$NON-NLS-1$

        addField( new LabelFieldEditor( Messages.WMLToolsPreferencePage_5,
            getFieldEditorParent( ) ) );
        addField( new ScaleFieldEditor( Preferences.WMLSCOPE_VERBOSE_LEVEL,
            Messages.WMLToolsPreferencePage_6, getFieldEditorParent( ), 0,
            2, 1, 1 ) );
        addField( new BooleanFieldEditor( Preferences.WMLSCOPE_COLLISIONS,
            Messages.WMLToolsPreferencePage_7, 1, getFieldEditorParent( ) ) );
        addField( new LabelFieldEditor( "", getFieldEditorParent( ) ) ); //$NON-NLS-1$

        addField( new LabelFieldEditor( Messages.WMLToolsPreferencePage_9,
            getFieldEditorParent( ) ) );
        addField( new BooleanFieldEditor( Preferences.WMLLINT_DRYRUN,
            Messages.WMLToolsPreferencePage_10, 1, getFieldEditorParent( ) ) );
        addField( new BooleanFieldEditor( Preferences.WMLLINT_SPELL_CHECK,
            Messages.WMLToolsPreferencePage_11, 1, getFieldEditorParent( ) ) );
        addField( new ScaleFieldEditor( Preferences.WMLLINT_VERBOSE_LEVEL,
            Messages.WMLToolsPreferencePage_12, getFieldEditorParent( ), 0,
            3, 1, 1 ) );
        addField( new LabelFieldEditor( "", getFieldEditorParent( ) ) ); //$NON-NLS-1$
    }
}
