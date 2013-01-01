/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.wizards.faction;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Text;

import org.wesnoth.Messages;
import org.wesnoth.wizards.WizardPageTemplate;

/**
 * Second page of the New Faction Wizard
 */
public class FactionPage1 extends WizardPageTemplate
{
    private Text   txtChoices_;
    private Text   txtExcept_;
    private Button chkRandomFaction_;

    /**
     * Create the wizard.
     */
    public FactionPage1( )
    {
        super( "factionPage1" ); //$NON-NLS-1$
        setTitle( Messages.FactionPage1_1 );
        setDescription( Messages.FactionPage1_2 );
    }

    @Override
    public void createControl( Composite parent )
    {
        super.createControl( parent );
        Composite container = new Composite( parent, SWT.NULL );

        // the page doesn't have any requirements, so it's complete by default
        setPageComplete( true );

        setControl( container );
        container.setLayout( new GridLayout( 2, false ) );
        new Label( container, SWT.NONE );

        chkRandomFaction_ = new Button( container, SWT.CHECK );
        chkRandomFaction_.setText( Messages.FactionPage1_3 );
        chkRandomFaction_.addSelectionListener( new SelectionAdapter( ) {
            @Override
            public void widgetSelected( SelectionEvent e )
            {
                if( ! ( e.getSource( ) instanceof Button ) ) {
                    return;
                }
                setExtraFactionEnableness( ( ( Button ) e.getSource( ) )
                    .getSelection( ) );
            }
        } );

        Label lblR = new Label( container, SWT.NONE );
        GridData gd_lblR = new GridData( SWT.FILL, SWT.CENTER, false, false, 1,
            1 );
        gd_lblR.widthHint = 77;
        lblR.setLayoutData( gd_lblR );
        lblR.setText( Messages.FactionPage1_4 );

        txtChoices_ = new Text( container, SWT.BORDER );
        txtChoices_.setEnabled( false );
        txtChoices_.setLayoutData( new GridData( SWT.FILL, SWT.CENTER, true,
            false, 1, 1 ) );

        Label lblExcept = new Label( container, SWT.NONE );
        lblExcept.setLayoutData( new GridData( SWT.FILL, SWT.CENTER, false,
            false, 1, 1 ) );
        lblExcept.setText( Messages.FactionPage1_5 );

        txtExcept_ = new Text( container, SWT.BORDER );
        txtExcept_.setEnabled( false );
        txtExcept_.setLayoutData( new GridData( SWT.FILL, SWT.CENTER, true,
            false, 1, 1 ) );
    }

    private void setExtraFactionEnableness( boolean state )
    {
        txtChoices_.setEnabled( state );
        txtExcept_.setEnabled( state );
    }

    /**
     * @return True if the faction is random
     */
    public boolean getIsRandomFaction( )
    {
        return chkRandomFaction_.getSelection( );
    }

    /**
     * @return The choices of the faction
     */
    public String getChoices( )
    {
        return getIsRandomFaction( ) ? txtChoices_.getText( ): ""; //$NON-NLS-1$
    }

    /**
     * @return Except
     */
    public String getExcept( )
    {
        return getIsRandomFaction( ) ? txtExcept_.getText( ): ""; //$NON-NLS-1$
    }
}
