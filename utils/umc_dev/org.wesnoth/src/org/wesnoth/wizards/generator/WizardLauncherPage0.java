/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.wizards.generator;

import org.eclipse.core.resources.IContainer;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.IPath;
import org.eclipse.core.runtime.Path;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Text;

import org.wesnoth.Messages;
import org.wesnoth.utils.EditorUtils;
import org.wesnoth.wizards.WizardPageTemplate;

/**
 * First page of the Wizard Launcher
 */
public class WizardLauncherPage0 extends WizardPageTemplate
{
    private Text   txtDirectory_;
    private Text   txtFileName_;
    private Button radioNewFile;
    private Label  lblCurrentFileOpened;
    private Label  lblDirectory;
    private Button btnBrowse;
    private Label  lblFileName;

    /**
     * Creates a new {@link WizardLauncherPage0}
     */
    public WizardLauncherPage0( )
    {
        super( "wizardLauncherPage0" ); //$NON-NLS-1$
        setTitle( Messages.WizardLauncherPage0_1 );
        setDescription( Messages.WizardLauncherPage0_2 );
    }

    @Override
    public void createControl( Composite parent )
    {
        super.createControl( parent );
        Composite container = new Composite( parent, SWT.NULL );

        setControl( container );
        container.setLayout( new GridLayout( 4, false ) );

        radioNewFile = new Button( container, SWT.RADIO );
        radioNewFile.addSelectionListener( new SelectionAdapter( ) {
            @Override
            public void widgetSelected( SelectionEvent e )
            {
                updateEnabledStatus( );
            }
        } );
        radioNewFile.setSelection( true );
        radioNewFile.setLayoutData( new GridData( SWT.LEFT, SWT.CENTER, false,
            false, 2, 1 ) );
        radioNewFile.setText( Messages.WizardLauncherPage0_3 );
        new Label( container, SWT.NONE );
        new Label( container, SWT.NONE );
        new Label( container, SWT.NONE );

        lblDirectory = new Label( container, SWT.NONE );
        lblDirectory.setLayoutData( new GridData( SWT.FILL, SWT.CENTER, false,
            false, 1, 1 ) );
        lblDirectory.setText( Messages.WizardLauncherPage0_4 );

        txtDirectory_ = new Text( container, SWT.BORDER );
        txtDirectory_.addModifyListener( new ModifyListener( ) {
            @Override
            public void modifyText( ModifyEvent e )
            {
                updatePageIsComplete( );
            }
        } );
        txtDirectory_.setLayoutData( new GridData( SWT.FILL, SWT.CENTER, true,
            false, 1, 1 ) );
        txtDirectory_.setEditable( false );

        btnBrowse = new Button( container, SWT.NONE );
        btnBrowse.addSelectionListener( new SelectionAdapter( ) {
            @Override
            public void widgetSelected( SelectionEvent e )
            {
                IPath path = handleBrowseContainer( );
                if( path != null ) {
                    txtDirectory_.setText( path.toString( ) );
                }
            }
        } );

        btnBrowse.setText( Messages.WizardLauncherPage0_5 );
        new Label( container, SWT.NONE );

        lblFileName = new Label( container, SWT.NONE );
        lblFileName.setLayoutData( new GridData( SWT.FILL, SWT.CENTER, false,
            false, 1, 1 ) );
        lblFileName.setText( Messages.WizardLauncherPage0_6 );

        txtFileName_ = new Text( container, SWT.BORDER );
        txtFileName_.addModifyListener( new ModifyListener( ) {
            @Override
            public void modifyText( ModifyEvent e )
            {
                updatePageIsComplete( );
            }
        } );
        txtFileName_.setLayoutData( new GridData( SWT.FILL, SWT.CENTER, true,
            false, 1, 1 ) );
        new Label( container, SWT.NONE );
        new Label( container, SWT.NONE );
        new Label( container, SWT.NONE );
        new Label( container, SWT.NONE );
        new Label( container, SWT.NONE );

        Button radioCurrentFile = new Button( container, SWT.RADIO );
        radioCurrentFile.addSelectionListener( new SelectionAdapter( ) {
            @Override
            public void widgetSelected( SelectionEvent e )
            {
                updateEnabledStatus( );
            }
        } );
        radioCurrentFile.setLayoutData( new GridData( SWT.LEFT, SWT.CENTER,
            false, false, 2, 1 ) );
        radioCurrentFile.setText( Messages.WizardLauncherPage0_7 );
        new Label( container, SWT.NONE );
        new Label( container, SWT.NONE );
        new Label( container, SWT.NONE );

        lblCurrentFileOpened = new Label( container, SWT.NONE );
        lblCurrentFileOpened.setEnabled( false );
        lblCurrentFileOpened.setLayoutData( new GridData( SWT.FILL, SWT.CENTER,
            false, false, 2, 1 ) );
        lblCurrentFileOpened.setText( Messages.WizardLauncherPage0_8 );
        new Label( container, SWT.NONE );
        new Label( container, SWT.NONE );
        new Label( container, SWT.NONE );
        new Label( container, SWT.NONE );
        new Label( container, SWT.NONE );
        new Label( container, SWT.NONE );
        new Label( container, SWT.NONE );
        new Label( container, SWT.NONE );
        new Label( container, SWT.NONE );
        new Label( container, SWT.NONE );
        new Label( container, SWT.NONE );
        new Label( container, SWT.NONE );
        new Label( container, SWT.NONE );
        new Label( container, SWT.NONE );
        new Label( container, SWT.NONE );
        new Label( container, SWT.NONE );
        new Label( container, SWT.NONE );

        Label label = new Label( container, SWT.NONE );
        label.setText( "            " ); //$NON-NLS-1$
        new Label( container, SWT.NONE );
        new Label( container, SWT.NONE );
        new Label( container, SWT.NONE );

        if( getWizard( ).getSelectionContainer( ) != null ) {
            txtDirectory_.setText( getWizard( ).getSelectionContainer( )
                .getFullPath( ).toString( ) );
        }
        updatePageIsComplete( );
    }

    private void updatePageIsComplete( )
    {
        setPageComplete( false );
        if( radioNewFile.getSelection( ) ) {
            IResource container = ResourcesPlugin.getWorkspace( ).getRoot( )
                .findMember( new Path( getDirectoryName( ) ) );
            String fileName = getFileName( );

            if( getDirectoryName( ).isEmpty( ) ) {
                setErrorMessage( Messages.WizardLauncherPage0_10 );
                return;
            }

            if( container == null || ! container.exists( )
                || ! ( container instanceof IContainer ) ) {
                setErrorMessage( Messages.WizardLauncherPage0_11 );
                return;
            }

            if( fileName.isEmpty( ) ) {
                setErrorMessage( Messages.WizardLauncherPage0_12 );
                return;
            }

            if( fileName.replace( '\\', '/' ).indexOf( '/', 1 ) > 0 ) {
                setErrorMessage( Messages.WizardLauncherPage0_13 );
                return;
            }

            int dotLoc = fileName.lastIndexOf( '.' );
            if( dotLoc == - 1
                || fileName.substring( dotLoc + 1 )
                    .equalsIgnoreCase( "cfg" ) == false ) //$NON-NLS-1$
            {
                setErrorMessage( Messages.WizardLauncherPage0_15 );
                return;
            }
        }
        else {
            // current file checking
            if( EditorUtils.getEditedFile( ) != null ) {
                lblCurrentFileOpened.setText( Messages.WizardLauncherPage0_16
                    + EditorUtils.getEditedFile( ).getEditorInput( )
                        .getName( ) + Messages.WizardLauncherPage0_17 );
            }
            else {
                lblCurrentFileOpened.setText( Messages.WizardLauncherPage0_18 );
                setErrorMessage( Messages.WizardLauncherPage0_19 );
                return;
            }
        }
        setPageComplete( true );
        setErrorMessage( null );
    }

    private void updateEnabledStatus( )
    {
        // new file section
        btnBrowse.setEnabled( radioNewFile.getSelection( ) );
        lblDirectory.setEnabled( radioNewFile.getSelection( ) );
        lblFileName.setEnabled( radioNewFile.getSelection( ) );
        txtDirectory_.setEnabled( radioNewFile.getSelection( ) );
        txtFileName_.setEnabled( radioNewFile.getSelection( ) );

        // opened file
        lblCurrentFileOpened.setEnabled( ! radioNewFile.getSelection( ) );

        if( getWizard( ).getSelectionContainer( ) != null ) {
            txtDirectory_.setText( getWizard( ).getSelectionContainer( )
                .getFullPath( ).toString( ) );
        }
        updatePageIsComplete( );
    }

    /**
     * @return The filename of the file to generate the WML Code into
     */
    public String getFileName( )
    {
        return radioNewFile.getSelection( ) == true ? txtFileName_.getText( )
            : EditorUtils.getEditedFile( ).getEditorInput( ).getName( );
    }

    /**
     * @return The target directory where the new file should be created,
     *         or empty string if the destination is the current file
     */
    public String getDirectoryName( )
    {
        return radioNewFile.getSelection( ) == true ? txtDirectory_.getText( )
            : ""; //$NON-NLS-1$
    }

    /**
     * @return True if the target for the generated WML code should be a
     *         new file, false otherwise ( the code will be pasted in current
     *         edited file )
     */
    public boolean getIsTargetNewFile( )
    {
        return radioNewFile.getSelection( );
    }
}
