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
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Text;

import org.wesnoth.Messages;
import org.wesnoth.wizards.WizardPageTemplate;

/**
 * Page 1 for the New Campaign Wizard
 */
public class CampaignPage1 extends WizardPageTemplate
{
    private Text   txtCampaignName_;
    private Text   txtVersion_;
    private Text   txtTranslationDir_;
    private Text   txtAuthor_;
    private Text   txtEmail_;
    private Text   txtDescription_;
    private Text   txtPassphrase_;
    private Text   txtIcon_;
    private Button chkMultiCampaign_;
    private Button chkGeneratePBL_;

    /**
     * Creates a new {@link CampaignPage1}
     */
    public CampaignPage1( )
    {
        super( "campaignPage1" ); //$NON-NLS-1$
        setTitle( Messages.CampaignPage1_1 );
        setDescription( Messages.CampaignPage1_2 );
        setPageComplete( false );
    }

    @Override
    public void createControl( Composite parent )
    {
        super.createControl( parent );
        Composite container = new Composite( parent, SWT.NULL );

        setControl( container );
        ModifyListener updatePageCompleteListener = new ModifyListener( ) {
            @Override
            public void modifyText( ModifyEvent e )
            {
                updateIsPageComplete( );
            }
        };
        container.setLayout( new GridLayout( 3, false ) );

        Label _lblCampaignName = new Label( container, SWT.NONE );
        _lblCampaignName.setText( Messages.CampaignPage1_3 );

        txtCampaignName_ = new Text( container, SWT.BORDER );
        GridData gd_txtCampaignName_ = new GridData( SWT.FILL, SWT.CENTER,
            false, false, 1, 1 );
        gd_txtCampaignName_.heightHint = 15;
        txtCampaignName_.setLayoutData( gd_txtCampaignName_ );
        txtCampaignName_.addModifyListener( updatePageCompleteListener );
        new Label( container, SWT.NONE );

        Label lblVersion = new Label( container, SWT.NONE );
        lblVersion.setText( Messages.CampaignPage1_4 );

        txtVersion_ = new Text( container, SWT.BORDER );
        txtVersion_.setLayoutData( new GridData( SWT.FILL, SWT.CENTER, false,
            false, 1, 1 ) );
        txtVersion_.addModifyListener( updatePageCompleteListener );

        Label lblFormat = new Label( container, SWT.NONE );
        lblFormat.setToolTipText( Messages.CampaignPage1_5 );
        lblFormat.setText( Messages.CampaignPage1_6 );

        Label lblTranslationsDir = new Label( container, SWT.NONE );
        lblTranslationsDir.setText( Messages.CampaignPage1_7 );

        txtTranslationDir_ = new Text( container, SWT.BORDER );
        txtTranslationDir_.setLayoutData( new GridData( SWT.FILL, SWT.CENTER,
            false, false, 1, 1 ) );

        chkMultiCampaign_ = new Button( container, SWT.CHECK );
        GridData gd_chkMultiCampaign_ = new GridData( SWT.LEFT, SWT.CENTER,
            false, false, 2, 1 );
        gd_chkMultiCampaign_.widthHint = 236;
        chkMultiCampaign_.setLayoutData( gd_chkMultiCampaign_ );
        chkMultiCampaign_.setText( Messages.CampaignPage1_10 );
        new Label( container, SWT.NONE );

        chkGeneratePBL_ = new Button( container, SWT.CHECK );
        chkGeneratePBL_.setLayoutData( new GridData( SWT.LEFT, SWT.CENTER,
            false, false, 3, 1 ) );
        chkGeneratePBL_.setText( Messages.CampaignPage1_11 );

        Label lblAuthor = new Label( container, SWT.NONE );
        lblAuthor.setText( Messages.CampaignPage1_12 );

        txtAuthor_ = new Text( container, SWT.BORDER );
        txtAuthor_.setLayoutData( new GridData( SWT.FILL, SWT.CENTER, false,
            false, 1, 1 ) );
        new Label( container, SWT.NONE );

        Label lblDescription = new Label( container, SWT.NONE );
        lblDescription.setText( Messages.CampaignPage1_13 );

        txtEmail_ = new Text( container, SWT.BORDER );
        txtEmail_.setLayoutData( new GridData( SWT.FILL, SWT.CENTER, false,
            false, 1, 1 ) );
        new Label( container, SWT.NONE );

        Label lblDescription_1 = new Label( container, SWT.NONE );
        lblDescription_1.setText( Messages.CampaignPage1_14 );

        txtDescription_ = new Text( container, SWT.BORDER );
        txtDescription_.setLayoutData( new GridData( SWT.FILL, SWT.CENTER,
            false, false, 1, 1 ) );
        new Label( container, SWT.NONE );

        Label lblIcon = new Label( container, SWT.NONE );
        lblIcon.setText( Messages.CampaignPage1_15 );

        txtPassphrase_ = new Text( container, SWT.BORDER );
        txtPassphrase_.setLayoutData( new GridData( SWT.FILL, SWT.CENTER,
            false, false, 1, 1 ) );
        new Label( container, SWT.NONE );

        Label lblIcon_1 = new Label( container, SWT.NONE );
        lblIcon_1.setText( Messages.CampaignPage1_16 );

        txtIcon_ = new Text( container, SWT.BORDER );
        GridData gd_txtIcon_ = new GridData( SWT.FILL, SWT.CENTER, false,
            false, 1, 1 );
        gd_txtIcon_.widthHint = 163;
        txtIcon_.setLayoutData( gd_txtIcon_ );

        Label lblRelativeToThe_1 = new Label( container, SWT.NONE );
        GridData gd_lblRelativeToThe_1 = new GridData( SWT.FILL, SWT.CENTER,
            false, false, 1, 1 );
        gd_lblRelativeToThe_1.widthHint = 285;
        lblRelativeToThe_1.setLayoutData( gd_lblRelativeToThe_1 );
        lblRelativeToThe_1.setToolTipText( Messages.CampaignPage1_17 );
        lblRelativeToThe_1.setText( Messages.CampaignPage1_18 );

        updateIsPageComplete( );
    }

    /**
     * Checks the mandatory fields and updates the isPageComplete status
     */
    public void updateIsPageComplete( )
    {
        setPageComplete( false );
        if( txtCampaignName_.getText( ).isEmpty( ) ) {
            setErrorMessage( Messages.CampaignPage1_19 );
            return;
        }

        // match the pattern x.y.z
        if( txtVersion_.getText( ).isEmpty( )
            || ! ( txtVersion_.getText( )
                .matches( "[\\d]+\\.[\\d]+\\.\\d[\\w\\W\\d\\D\\s\\S]*" ) ) ) //$NON-NLS-1$
        {
            setErrorMessage( Messages.CampaignPage1_21 );
            return;
        }

        setErrorMessage( null );
        setPageComplete( true );
    }

    /**
     * @return the Campaign Name
     */
    public String getCampaignName( )
    {
        return txtCampaignName_.getText( );
    }

    /**
     * @return the author
     */
    public String getAuthor( )
    {
        return txtAuthor_.getText( );
    }

    /**
     * @return the version
     */
    public String getVersion( )
    {
        return txtVersion_.getText( );
    }

    /**
     * @return the description
     */
    public String getCampaignDescription( )
    {
        return txtDescription_.getText( );
    }

    /**
     * @return the Icon
     */
    public String getIconPath( )
    {
        return txtIcon_.getText( );
    }

    /**
     * @return the email
     */
    public String getEmail( )
    {
        return txtEmail_.getText( );
    }

    /**
     * @return the passphrase
     */
    public String getPassphrase( )
    {
        return txtPassphrase_.getText( );
    }

    /**
     * @return the translation directory
     */
    public String getTranslationDir( )
    {
        return txtTranslationDir_.getText( );
    }

    /**
     * @return true if the campaign is multiplayer
     */
    public boolean isMultiplayer( )
    {
        return chkMultiCampaign_.getSelection( );
    }

    /**
     * 
     * @return true to create the pbl file
     */
    public boolean needsPBLFile( )
    {
        return chkGeneratePBL_.getSelection( );
    }
}
