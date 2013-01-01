/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.wizards.emptyproject;

import org.eclipse.jface.dialogs.IMessageProvider;
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
 * The first page in the New Empty Project wizard
 */
public class EmptyProjectPage1 extends WizardPageTemplate
{
    private Text   txtTitle_;
    private Text   txtVersion_;
    private Text   txtTranslationDir_;
    private Text   txtAuthor_;
    private Text   txtEmail_;
    private Text   txtDescription_;
    private Text   txtPassphrase_;
    private Text   txtIcon_;
    private Text   txtType_;
    private Button chkGeneratePBL_;

    /**
     * Creates a new {@link EmptyProjectPage1}
     */
    public EmptyProjectPage1( )
    {
        super( "emptyProjectPage1" ); //$NON-NLS-1$
        setTitle( Messages.EmptyProjectPage1_1 );
        setDescription( Messages.EmptyProjectPage1_2 );
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

        chkGeneratePBL_ = new Button( container, SWT.CHECK );
        chkGeneratePBL_.setLayoutData( new GridData( SWT.LEFT, SWT.CENTER,
            false, false, 3, 1 ) );
        chkGeneratePBL_.setText( Messages.EmptyProjectPage1_4 );

        Label _lblTitle = new Label( container, SWT.NONE );
        _lblTitle.setText( Messages.EmptyProjectPage1_5 );

        txtTitle_ = new Text( container, SWT.BORDER );
        GridData gd_txtTitle_ = new GridData( SWT.FILL, SWT.CENTER, false,
            false, 1, 1 );
        gd_txtTitle_.heightHint = 15;
        txtTitle_.setLayoutData( gd_txtTitle_ );
        txtTitle_.addModifyListener( updatePageCompleteListener );

        Label lblThisIsThe = new Label( container, SWT.NONE );
        lblThisIsThe.setText( Messages.EmptyProjectPage1_6 );

        Label lblVersion = new Label( container, SWT.NONE );
        lblVersion.setText( Messages.EmptyProjectPage1_7 );

        txtVersion_ = new Text( container, SWT.BORDER );
        txtVersion_.setLayoutData( new GridData( SWT.FILL, SWT.CENTER, false,
            false, 1, 1 ) );
        txtVersion_.addModifyListener( updatePageCompleteListener );

        Label lblFormat = new Label( container, SWT.NONE );
        lblFormat.setToolTipText( Messages.EmptyProjectPage1_8 );
        lblFormat.setText( Messages.EmptyProjectPage1_14 );

        Label lblTranslationsDir = new Label( container, SWT.NONE );
        lblTranslationsDir.setText( Messages.EmptyProjectPage1_15 );

        txtTranslationDir_ = new Text( container, SWT.BORDER );
        txtTranslationDir_.setLayoutData( new GridData( SWT.FILL, SWT.CENTER,
            false, false, 1, 1 ) );

        Label lblRelativeToThe = new Label( container, SWT.NONE );
        lblRelativeToThe.setText( Messages.EmptyProjectPage1_16 );

        Label lblAuthor = new Label( container, SWT.NONE );
        lblAuthor.setText( Messages.EmptyProjectPage1_17 );

        txtAuthor_ = new Text( container, SWT.BORDER );
        txtAuthor_.setLayoutData( new GridData( SWT.FILL, SWT.CENTER, false,
            false, 1, 1 ) );
        new Label( container, SWT.NONE );

        Label lblDescription = new Label( container, SWT.NONE );
        lblDescription.setText( Messages.EmptyProjectPage1_18 );

        txtEmail_ = new Text( container, SWT.BORDER );
        txtEmail_.setLayoutData( new GridData( SWT.FILL, SWT.CENTER, false,
            false, 1, 1 ) );
        new Label( container, SWT.NONE );

        Label lblDescription_1 = new Label( container, SWT.NONE );
        lblDescription_1.setText( Messages.EmptyProjectPage1_19 );

        txtDescription_ = new Text( container, SWT.BORDER );
        txtDescription_.setLayoutData( new GridData( SWT.FILL, SWT.CENTER,
            false, false, 1, 1 ) );
        new Label( container, SWT.NONE );

        Label lblType = new Label( container, SWT.NONE );
        lblType.setLayoutData( new GridData( SWT.FILL, SWT.CENTER, false,
            false, 1, 1 ) );
        lblType.setText( Messages.EmptyProjectPage1_20 );

        txtType_ = new Text( container, SWT.BORDER );
        txtType_.setLayoutData( new GridData( SWT.FILL, SWT.CENTER, true,
            false, 1, 1 ) );
        new Label( container, SWT.NONE );

        Label lblIcon = new Label( container, SWT.NONE );
        lblIcon.setText( Messages.EmptyProjectPage1_21 );

        txtPassphrase_ = new Text( container, SWT.BORDER );
        txtPassphrase_.setLayoutData( new GridData( SWT.FILL, SWT.CENTER,
            false, false, 1, 1 ) );
        new Label( container, SWT.NONE );

        Label lblIcon_1 = new Label( container, SWT.NONE );
        lblIcon_1.setText( Messages.EmptyProjectPage1_22 );

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
        lblRelativeToThe_1.setToolTipText( Messages.EmptyProjectPage1_23 );
        lblRelativeToThe_1.setText( Messages.EmptyProjectPage1_32 );

        updateIsPageComplete( );
    }

    /**
     * Checks the mandatory fields and updates the isPageComplete status
     */
    public void updateIsPageComplete( )
    {
        setPageComplete( false );
        if( txtTitle_.getText( ).isEmpty( ) ) {
            setMessage( Messages.EmptyProjectPage1_33, IMessageProvider.WARNING );
            return;
        }

        // match the pattern x.y.z
        if( txtVersion_.getText( ).isEmpty( )
            || ! ( txtVersion_.getText( )
                .matches( "[\\d]+\\.[\\d]+\\.\\d[\\w\\W\\d\\D\\s\\S]*" ) ) ) //$NON-NLS-1$
        {
            setMessage( Messages.EmptyProjectPage1_35, IMessageProvider.WARNING );
            return;
        }

        setMessage( null );
        setPageComplete( true );
    }

    /**
     * @return the Campaign Name
     */
    public String getCampaignName( )
    {
        return txtTitle_.getText( );
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
    public String getPBLDescription( )
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
     * @return The type of the project
     */
    public String getType( )
    {
        return txtType_.getText( );
    }

    /**
     * @return True to generate a .pbl file, false otherwise
     */
    public boolean getGeneratePBLFile( )
    {
        return chkGeneratePBL_.getSelection( );
    }
}
