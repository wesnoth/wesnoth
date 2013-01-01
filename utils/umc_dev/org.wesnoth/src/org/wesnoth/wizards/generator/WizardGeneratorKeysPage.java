/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.wizards.generator;

import java.util.List;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Combo;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Text;

import org.wesnoth.Messages;
import org.wesnoth.utils.StringUtils;
import org.wesnoth.wizards.WizardPageTemplate;
import org.wesnoth.wml.WMLKey;
import org.wesnoth.wml.WMLKeyValue;

/**
 * A page that generates keys for a tag
 */
public class WizardGeneratorKeysPage extends WizardPageTemplate
{
    private List< WMLKey > keys_;
    private int            startIndex_, endIndex_;
    private Composite      container_;
    private int            indent_;

    /**
     * @param tagName
     *        The tag the keys are belonging to
     * @param keys
     *        The current keys list
     * @param startIndex
     *        The start index in the keys list
     * @param endIndex
     *        The end index in the keys list
     * @param indent
     *        The indent used when creating the WML output
     */
    public WizardGeneratorKeysPage( String tagName, List< WMLKey > keys,
        int startIndex, int endIndex, int indent )
    {
        super( "wizardPageKey" + startIndex );
        setTitle( tagName + Messages.WizardGeneratorPageKey_1 );

        indent_ = indent;

        startIndex_ = startIndex;
        endIndex_ = endIndex;
        keys_ = keys;
    }

    @Override
    public void createControl( Composite parent )
    {
        super.createControl( parent );
        container_ = new Composite( parent, SWT.NULL );
        setControl( container_ );
        container_.setLayout( new GridLayout( 2, false ) );

        for( int i = startIndex_; i <= endIndex_; i++ ) {
            WMLKey key = keys_.get( i );

            if( key.is_Forbidden( ) ) {
                continue;
            }

            Label label = new Label( container_, SWT.NONE );
            label.setLayoutData( new GridData( SWT.FILL, SWT.CENTER, false,
                false, 1, 1 ) );
            // add star to required items
            label.setText( key.getName( ) + ( key.is_Enum( ) ? "*": "" ) + ":" ); //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$

            // if the is an enum create a combobox instead of textbox
            if( key.is_Enum( ) ) {
                Combo combo = new Combo( container_, SWT.READ_ONLY );
                combo.setLayoutData( new GridData( SWT.FILL, SWT.CENTER, true,
                    false, 1, 1 ) );
                combo.setData( "name", key.getName( ) ); //$NON-NLS-1$

                for( WMLKeyValue value: key.getValues( ) ) {
                    combo.add( value.toString( ) );
                }

                combo.select( 0 );
            }
            else {
                Text textBox = new Text( container_, SWT.BORDER );
                textBox.setLayoutData( new GridData( SWT.FILL, SWT.CENTER,
                    true, false, 1, 1 ) );

                textBox.setData( "name", key.getName( ) ); //$NON-NLS-1$
                textBox.setData( "valType", key.getValues( ) ); //$NON-NLS-1$
                textBox.setData( "card", key.get_Cardinality( ) ); //$NON-NLS-1$
                textBox.setData( "trans", key.is_Translatable( ) ); //$NON-NLS-1$
                if( key.is_Required( ) ) {
                    textBox.setData( "comp", false ); // is textbox complete //$NON-NLS-1$
                }

                textBox.addModifyListener( new ModifyListener( ) {
                    @Override
                    public void modifyText( ModifyEvent e )
                    {
                        if( ! ( e.getSource( ) instanceof Text ) ) {
                            return;
                        }

                        Text txt = ( ( Text ) e.getSource( ) );
                        if( txt.getData( "comp" ) == null ) {
                            return;
                        }

                        if( ( txt.getText( ).isEmpty( ) && ( txt.getData(
                            "card" ).toString( ).equals( "1" ) ) ) || // cardinality //$NON-NLS-1$ //$NON-NLS-2$
                            ! txt.getText( ).matches(
                                txt.getData( "valType" ).toString( ) ) // regex //$NON-NLS-1$
                        ) {
                            txt.setData( "comp", false ); //$NON-NLS-1$
                        }
                        else {
                            txt.setData( "comp", true ); //$NON-NLS-1$
                        }
                        updatePageIsComplete( );
                    }
                } );
            }
        }
        updatePageIsComplete( );
    }

    private void updatePageIsComplete( )
    {
        setPageComplete( false );

        for( Control child: container_.getChildren( ) ) {
            if( ! ( child instanceof Text ) ) {
                continue;
            }

            if( child.getData( "comp" ) == null ) {
                continue;
            }

            if( child.getData( "comp" ).toString( ).equals( "false" ) ) //$NON-NLS-1$ //$NON-NLS-2$
            {
                setErrorMessage( child.getData( "name" ) + //$NON-NLS-1$
                    Messages.WizardGeneratorPageKey_22
                    + child.getData( "valType" ) ); //$NON-NLS-1$
                return;
            }
        }

        setPageComplete( true );
        setErrorMessage( null );
    }

    /**
     * @return the generated WML Output
     */
    public String getContent( )
    {
        StringBuilder result = new StringBuilder( );
        for( Control child: container_.getChildren( ) ) {
            if( ! ( child instanceof Text || child instanceof Combo ) ) {
                continue;
            }
            String text = ""; //$NON-NLS-1$
            if( child instanceof Text ) {
                text = ( child.getData( "trans" ).toString( ).equals( "true" ) == true ? "_": "" ) + //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$ //$NON-NLS-4$
                    "\"" + ( ( Text ) child ).getText( ) + "\""; //$NON-NLS-1$ //$NON-NLS-2$
            }
            else {
                text = ( ( Combo ) child ).getText( );
            }
            result.append( StringUtils.multiples( "\t", indent_ ) + //$NON-NLS-1$
                child.getData( "name" ) + "=" + text + "\n" ); //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$
        }
        return result.toString( );
    }
}
