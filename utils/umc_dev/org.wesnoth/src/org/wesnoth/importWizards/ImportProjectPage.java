/*******************************************************************************
 * Copyright (c) 2011 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.importWizards;

import java.io.File;

import org.eclipse.jface.preference.DirectoryFieldEditor;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.widgets.Combo;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Text;

import org.wesnoth.installs.WesnothInstallsUtils;
import org.wesnoth.wizards.WizardPageTemplate;

/**
 * The page of the Import Project Wizard
 */
public class ImportProjectPage extends WizardPageTemplate
{
    private DirectoryFieldEditor projectPathField_;
    private Text                 txtProjectName_;
    private Combo                cmbInstalls_;

    protected ImportProjectPage( )
    {
        super( "importProjectPage0" );

        setTitle( "Import directory as wesnoth project" );
        setMessage( "Import a directory as a wesnoth project." );
        setPageComplete( false );
    }

    @Override
    public void createControl( Composite parent )
    {
        super.createControl( parent );
        Composite container = new Composite( parent, SWT.NULL );
        setControl( container );

        ModifyListener listener = new ModifyListener( ) {

            @Override
            public void modifyText( ModifyEvent e )
            {
                updatePageIsComplete( );
            }
        };

        projectPathField_ = new DirectoryFieldEditor( "project_path",
            "Directory to import:", container );
        projectPathField_.getTextControl( container ).addModifyListener(
            listener );

        Label lblNewLabel = new Label( container, SWT.NONE );
        lblNewLabel.setLayoutData( new GridData( SWT.RIGHT, SWT.CENTER, false,
            false, 1, 1 ) );
        lblNewLabel.setText( "Project Name:" );

        txtProjectName_ = new Text( container, SWT.BORDER );
        txtProjectName_.setLayoutData( new GridData( SWT.FILL, SWT.TOP, true,
            false, 2, 1 ) );
        txtProjectName_.addModifyListener( listener );

        Label lblWesnothInstall = new Label( container, SWT.NONE );
        lblWesnothInstall.setLayoutData( new GridData( SWT.RIGHT, SWT.CENTER,
            false, false, 1, 1 ) );
        lblWesnothInstall.setText( "Wesnoth install:" );

        cmbInstalls_ = new Combo( container, SWT.READ_ONLY );
        cmbInstalls_.setLayoutData( new GridData( SWT.FILL, SWT.CENTER, true,
            false, 2, 1 ) );
        WesnothInstallsUtils.fillComboWithInstalls( cmbInstalls_ );
    }

    protected void updatePageIsComplete( )
    {
        setPageComplete( ! txtProjectName_.getText( ).isEmpty( )
            && new File( projectPathField_.getStringValue( ) ).exists( ) );
    }

    /**
     * Returns the imported project's path
     * 
     * @return A string with the project path
     */
    public String getProjectPath( )
    {
        return projectPathField_.getStringValue( );
    }

    /**
     * Returns the imported project's name
     * 
     * @return A string with the imported project's name
     */
    public String getProjectName( )
    {
        return txtProjectName_.getText( );
    }

    /**
     * Returns the selected install name
     * 
     * @return A string with the selected install name
     */
    public String getSelectedInstallName( )
    {
        return cmbInstalls_.getText( );
    }
}
