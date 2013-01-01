/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.wizards.campaign;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.events.SelectionListener;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Text;

import org.wesnoth.Messages;
import org.wesnoth.utils.StringUtils;
import org.wesnoth.wizards.WizardPageTemplate;

/**
 * Page 2 of the New Campaign Wizard
 */
public class CampaignPage2 extends WizardPageTemplate
{
    private Text txtAbbrev_;
    private Text txtDefine_;
    private Text txtDifficulties_;
    private Text txtFirstScenario_;
    private Text txtID_;

    /**
     * Create a new {@link CampaignPage2}
     */
    public CampaignPage2( )
    {
        super( "campaignPage2" ); //$NON-NLS-1$
        setTitle( Messages.CampaignPage2_1 );
        setDescription( Messages.CampaignPage2_2 );
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
        ModifyListener modifyListener = new ModifyListener( ) {
            @Override
            public void modifyText( ModifyEvent e )
            {
                updatePageIsComplete( );
            }
        };

        SelectionListener selectionListener = new SelectionListener( ) {
            @Override
            public void widgetSelected( SelectionEvent e )
            {
                if( ! ( e.getSource( ) instanceof Button ) ) {
                    return;
                }
                String dif = ( ( Button ) e.getSource( ) ).getText( );

                if( ! txtDifficulties_.getText( ).contains( dif ) ) {
                    txtDifficulties_.append( "," + dif ); //$NON-NLS-1$
                }
                else {
                    txtDifficulties_.setText( txtDifficulties_.getText( )
                        .replace( dif, "" ) ); //$NON-NLS-1$
                }

                txtDifficulties_.setText( StringUtils
                    .removeIncorrectCharacters(
                        txtDifficulties_.getText( ), ',', true, true ) );
            }

            @Override
            public void widgetDefaultSelected( SelectionEvent e )
            {
            }
        };

        container.setLayout( new GridLayout( 5, false ) );

        Label lblId = new Label( container, SWT.NONE );
        lblId.setText( Messages.CampaignPage2_5 );

        txtID_ = new Text( container, SWT.BORDER );
        txtID_.setLayoutData( new GridData( SWT.FILL, SWT.CENTER, true, false,
            1, 1 ) );
        txtID_.addModifyListener( modifyListener );
        new Label( container, SWT.NONE );
        new Label( container, SWT.NONE );
        new Label( container, SWT.NONE );

        Label lblAbbreviation = new Label( container, SWT.NONE );
        lblAbbreviation.setText( Messages.CampaignPage2_6 );

        txtAbbrev_ = new Text( container, SWT.BORDER );
        GridData gd_txtAbbrev_ = new GridData( SWT.FILL, SWT.CENTER, false,
            false, 1, 1 );
        gd_txtAbbrev_.widthHint = 278;
        txtAbbrev_.setLayoutData( gd_txtAbbrev_ );
        txtAbbrev_.addModifyListener( modifyListener );
        new Label( container, SWT.NONE );
        new Label( container, SWT.NONE );
        new Label( container, SWT.NONE );

        Label lblDefine = new Label( container, SWT.NONE );
        lblDefine.setText( Messages.CampaignPage2_7 );

        txtDefine_ = new Text( container, SWT.BORDER );
        GridData gd_txtDefine_ = new GridData( SWT.FILL, SWT.CENTER, false,
            false, 1, 1 );
        gd_txtDefine_.widthHint = 207;
        txtDefine_.setLayoutData( gd_txtDefine_ );
        txtDefine_.addModifyListener( modifyListener );
        new Label( container, SWT.NONE );
        new Label( container, SWT.NONE );
        new Label( container, SWT.NONE );

        Label lblDifficulties = new Label( container, SWT.NONE );
        lblDifficulties.setText( Messages.CampaignPage2_8 );

        txtDifficulties_ = new Text( container, SWT.BORDER );
        GridData gd_txtDifficulties_ = new GridData( SWT.FILL, SWT.CENTER,
            false, false, 1, 1 );
        gd_txtDifficulties_.widthHint = 208;
        txtDifficulties_.setLayoutData( gd_txtDifficulties_ );

        Button chkDiffEasy_ = new Button( container, SWT.CHECK );
        chkDiffEasy_.setText( "EASY" ); //$NON-NLS-1$
        chkDiffEasy_.addSelectionListener( selectionListener );

        Button chkDiffNormal_ = new Button( container, SWT.CHECK );
        chkDiffNormal_.setText( "NORMAL" ); //$NON-NLS-1$
        chkDiffNormal_.addSelectionListener( selectionListener );

        Button chkDiffHard_ = new Button( container, SWT.CHECK );
        chkDiffHard_.setText( "HARD" ); //$NON-NLS-1$
        chkDiffHard_.addSelectionListener( selectionListener );

        Label lblFirstScenario = new Label( container, SWT.NONE );
        lblFirstScenario.setText( Messages.CampaignPage2_12 );

        txtFirstScenario_ = new Text( container, SWT.BORDER );
        GridData gd_txtFirstScenario_ = new GridData( SWT.FILL, SWT.CENTER,
            false, false, 1, 1 );
        gd_txtFirstScenario_.widthHint = 206;
        txtFirstScenario_.setLayoutData( gd_txtFirstScenario_ );
        new Label( container, SWT.NONE );
        new Label( container, SWT.NONE );
        new Label( container, SWT.NONE );

        updatePageIsComplete( );
    }

    /**
     * Checks the mandatory fields and updates the isPageComplete status
     */
    public void updatePageIsComplete( )
    {
        setPageComplete( false );

        if( txtID_.getText( ).isEmpty( ) ) {
            setErrorMessage( Messages.CampaignPage2_13 );
            return;
        }

        if( txtAbbrev_.getText( ).isEmpty( ) ) {
            setErrorMessage( Messages.CampaignPage2_14 );
            return;
        }

        if( txtDefine_.getText( ).isEmpty( ) ) {
            setErrorMessage( Messages.CampaignPage2_15 );
            return;
        }

        setErrorMessage( null );
        setPageComplete( true );
    }

    /**
     * @return the abbreviation of the campaign
     */
    public String getAbbrev( )
    {
        return txtAbbrev_.getText( );
    }

    /**
     * @return the define of the campaign
     */
    public String getDefine( )
    {
        return txtDefine_.getText( );
    }

    /**
     * @return the difficulties of the campaign
     */
    public String getDifficulties( )
    {
        return txtDifficulties_.getText( );
    }

    /**
     * @return the first scenario of the campaign
     */
    public String getFirstScenario( )
    {
        return txtFirstScenario_.getText( );
    }

    /**
     * @return the campaign id
     */
    public String getCampaignId( )
    {
        return txtID_.getText( );
    }
}
