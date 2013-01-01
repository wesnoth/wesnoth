/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.wizards.scenario;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Label;

import org.wesnoth.Messages;
import org.wesnoth.wizards.WizardPageTemplate;

/**
 * The third page of the New Scenario Page
 */
public class ScenarioPage2 extends WizardPageTemplate
{
    Button chkIsMultiplayerScenario_;
    Button chkAllowNewGame_;

    /**
     * Create the wizard.
     */
    public ScenarioPage2( )
    {
        super( "scenarioPage2" ); //$NON-NLS-1$
        setTitle( Messages.ScenarioPage2_1 );
        setDescription( Messages.ScenarioPage2_2 );
    }

    /**
     * Create contents of the wizard.
     * 
     * @param parent
     */
    @Override
    public void createControl( Composite parent )
    {
        super.createControl( parent );
        Composite container = new Composite( parent, SWT.NULL );

        setControl( container );
        container.setLayout( new GridLayout( 1, false ) );

        chkIsMultiplayerScenario_ = new Button( container, SWT.CHECK );
        chkIsMultiplayerScenario_.setText( Messages.ScenarioPage2_3 );
        new Label( container, SWT.NONE );
        chkIsMultiplayerScenario_
            .addSelectionListener( new SelectionAdapter( ) {
                @Override
                public void widgetSelected( SelectionEvent e )
                {
                    if( ! ( e.getSource( ) instanceof Button ) ) {
                        return;
                    }
                    setMPSettings( ( ( Button ) e.getSource( ) )
                        .getSelection( ) );
                }
            } );

        chkAllowNewGame_ = new Button( container, SWT.CHECK );
        chkAllowNewGame_.setSelection( true );
        chkAllowNewGame_.setEnabled( false );
        chkAllowNewGame_.setText( Messages.ScenarioPage2_4 );
    }

    private void setMPSettings( boolean status )
    {
        chkAllowNewGame_.setEnabled( status );
    }

    /**
     * @return True if a new game is allowed, false otherwise
     */
    public String getAllowNewGame( )
    {
        return isMultiplayerScenario( ) ? String.valueOf( chkAllowNewGame_
            .getSelection( ) ): ""; //$NON-NLS-1$
    }

    /**
     * @return True if this is a multiplayer scenario, false
     *         otherwise
     */
    public boolean isMultiplayerScenario( )
    {
        return chkIsMultiplayerScenario_.getSelection( );
    }
}
