/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.wizards.generator;

import java.util.HashMap;
import java.util.Map;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.widgets.Combo;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Text;

import org.wesnoth.Logger;
import org.wesnoth.Messages;
import org.wesnoth.templates.TemplateProvider;
import org.wesnoth.utils.StringUtils;
import org.wesnoth.wizards.WizardPageTemplate;

/**
 * The second page of the Wizard Launcher
 */
public class WizardLauncherPage1 extends WizardPageTemplate
{
    private Map< String, String > list_;
    private Text                  txtOtherTag_;
    private Combo                 cmbWizardName_;
    private Group                 grpCustomTag_;

    /**
     * Creates a new {@link WizardLauncherPage1}
     */
    public WizardLauncherPage1( )
    {
        super( "wizardLauncherPage1" ); //$NON-NLS-1$
        setTitle( Messages.WizardLauncherPage1_1 );
        setDescription( Messages.WizardLauncherPage1_2 );

        list_ = new HashMap< String, String >( );

        String[] templates = StringUtils.getLines( TemplateProvider
            .getInstance( ).getTemplate( "wizards" ) ); //$NON-NLS-1$

        for( String line: templates ) {
            if( StringUtils.startsWith( line, "#" ) ) {
                continue;
            }
            String[] tokens = line.split( ":" ); //$NON-NLS-1$
            if( tokens.length != 2 ) {
                Logger.getInstance( ).logError(
                    "Error in template 'wizards' on line:" + line ); //$NON-NLS-1$
                continue;
            }
            list_.put( tokens[0], tokens[1] );
        }
        list_.put( Messages.WizardLauncherPage1_7, "_" ); //$NON-NLS-1$
    }

    @Override
    public void createControl( Composite parent )
    {
        super.createControl( parent );
        Composite container = new Composite( parent, SWT.NULL );

        setControl( container );
        container.setLayout( null );

        Label label = new Label( container, SWT.NONE );
        label.setBounds( 5, 25, 141, 15 );
        label.setText( "    " ); //$NON-NLS-1$

        Label label_1 = new Label( container, SWT.NONE );
        label_1.setBounds( 151, 25, 146, 15 );
        label_1.setText( " " ); //$NON-NLS-1$
        Label label_2 = new Label( container, SWT.NONE );
        label_2.setBounds( 146, 85, 0, 15 );

        Label label_3 = new Label( container, SWT.NONE );
        label_3.setText( Messages.WizardLauncherPage1_11 );
        label_3.setBounds( 142, 63, 267, 15 );

        cmbWizardName_ = new Combo( container, SWT.READ_ONLY );
        cmbWizardName_.setBounds( 142, 83, 267, 23 );
        cmbWizardName_.setItems( list_.keySet( ).toArray(
            new String[list_.size( )] ) );
        if( cmbWizardName_.getItemCount( ) > 1 ) {
            cmbWizardName_.select( 1 );
        }
        cmbWizardName_.addSelectionListener( new SelectionAdapter( ) {
            @Override
            public void widgetSelected( SelectionEvent e )
            {
                if( ! ( e.getSource( ) instanceof Combo ) ) {
                    return;
                }
                grpCustomTag_.setVisible( ( ( Combo ) e.getSource( ) )
                    .getText( ).equals( Messages.WizardLauncherPage1_12 ) );
            }
        } );

        grpCustomTag_ = new Group( container, SWT.NONE );
        grpCustomTag_.setText( Messages.WizardLauncherPage1_13 );
        grpCustomTag_.setBounds( 142, 131, 267, 76 );
        grpCustomTag_.setVisible( false );

        txtOtherTag_ = new Text( grpCustomTag_, SWT.BORDER );
        txtOtherTag_.setBounds( 12, 37, 245, 21 );
    }

    /**
     * @return The current tag name
     */
    public String getTagName( )
    {
        return cmbWizardName_.getText( ).equals( "Other" ) == true ? txtOtherTag_
            .getText( ): list_.get( cmbWizardName_.getText( ) );
    }

    /**
     * @return the tag description
     */
    public String getTagDescription( )
    {
        return cmbWizardName_.getText( );
    }
}
