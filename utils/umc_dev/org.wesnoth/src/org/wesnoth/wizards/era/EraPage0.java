/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.wizards.era;

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
import org.wesnoth.wizards.WizardPageTemplate;

/**
 * The first page in the New Era Wizard
 */
public class EraPage0 extends WizardPageTemplate
{
    private Text   txtDirectory_;
    private Text   txtFileName_;
    private Text   txtEraID_;
    private Text   txtEraName_;
    private Button chkRequireEra_;

    /**
     * Create a new {@link EraPage0}
     */
    public EraPage0( )
    {
        super( "eraPage0" ); //$NON-NLS-1$
        setTitle( Messages.EraPage0_1 );
        setDescription( Messages.EraPage0_2 );
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
        ModifyListener modifyListener = new ModifyListener( ) {
            @Override
            public void modifyText( ModifyEvent e )
            {
                updatePageIsComplete( );
            }
        };

        Composite container = new Composite( parent, SWT.NULL );

        setControl( container );
        container.setLayout( new GridLayout( 3, false ) );

        Label lblProject = new Label( container, SWT.NONE );
        GridData gd_lblProject = new GridData( SWT.LEFT, SWT.CENTER, false,
            false, 1, 1 );
        gd_lblProject.widthHint = 99;
        lblProject.setLayoutData( gd_lblProject );
        lblProject.setText( Messages.EraPage0_3 );

        txtDirectory_ = new Text( container, SWT.BORDER );
        txtDirectory_.setLayoutData( new GridData( SWT.FILL, SWT.CENTER, true,
            false, 1, 1 ) );
        txtDirectory_.addModifyListener( modifyListener );
        txtDirectory_.setEditable( false );

        Button btnBrowse = new Button( container, SWT.NONE );
        btnBrowse.setText( Messages.EraPage0_4 );
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

        Label lblFileName = new Label( container, SWT.NONE );
        lblFileName.setText( Messages.EraPage0_5 );

        txtFileName_ = new Text( container, SWT.BORDER );
        txtFileName_.setLayoutData( new GridData( SWT.FILL, SWT.CENTER, true,
            false, 1, 1 ) );
        new Label( container, SWT.NONE );
        txtFileName_.addModifyListener( modifyListener );

        Label lblEraID = new Label( container, SWT.NONE );
        lblEraID.setText( Messages.EraPage0_6 );

        txtEraID_ = new Text( container, SWT.BORDER );
        txtEraID_.setLayoutData( new GridData( SWT.FILL, SWT.CENTER, true,
            false, 1, 1 ) );
        new Label( container, SWT.NONE );
        txtEraID_.addModifyListener( modifyListener );

        Label lblEraName = new Label( container, SWT.NONE );
        lblEraName.setText( Messages.EraPage0_7 );

        txtEraName_ = new Text( container, SWT.BORDER );
        txtEraName_.setLayoutData( new GridData( SWT.FILL, SWT.CENTER, true,
            false, 1, 1 ) );
        new Label( container, SWT.NONE );
        txtEraName_.addModifyListener( modifyListener );

        chkRequireEra_ = new Button( container, SWT.CHECK );
        chkRequireEra_.setToolTipText( Messages.EraPage0_8 );
        chkRequireEra_.setText( Messages.EraPage0_9 );
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
        IResource container = ResourcesPlugin.getWorkspace( ).getRoot( )
            .findMember( new Path( getDirectoryName( ) ) );
        setPageComplete( false );
        String fileName = getFileName( );

        if( getDirectoryName( ).isEmpty( ) ) {
            setErrorMessage( Messages.EraPage0_10 );
            return;
        }

        if( container == null || ! container.exists( )
            || ! ( container instanceof IContainer ) ) {
            setErrorMessage( Messages.EraPage0_11 );
            return;
        }

        if( fileName.isEmpty( ) ) {
            setErrorMessage( Messages.EraPage0_12 );
            return;
        }

        if( fileName.replace( '\\', '/' ).indexOf( '/', 1 ) > 0 ) {
            setErrorMessage( Messages.EraPage0_13 );
            return;
        }

        int dotLoc = fileName.lastIndexOf( '.' );
        if( dotLoc == - 1
            || fileName.substring( dotLoc + 1 ).equalsIgnoreCase( "cfg" ) == false ) //$NON-NLS-1$
        {
            setErrorMessage( Messages.EraPage0_15 );
            return;
        }

        if( getEraID( ).isEmpty( ) ) {
            setErrorMessage( Messages.EraPage0_16 );
            return;
        }

        if( getEraName( ).isEmpty( ) ) {
            setErrorMessage( Messages.EraPage0_17 );
            return;
        }

        setErrorMessage( null );
        setPageComplete( true );
    }

    /**
     * @return true if this requires the era to be installed in user's game
     */
    public boolean getRequiresEra( )
    {
        return chkRequireEra_.getSelection( );
    }

    /**
     * @return the era id
     */
    public String getEraID( )
    {
        return txtEraID_.getText( );
    }

    /**
     * @return the era name
     */
    public String getEraName( )
    {
        return txtEraName_.getText( );
    }

    /**
     * @return the filename containing the era
     */
    public String getFileName( )
    {
        return txtFileName_.getText( );
    }

    /**
     * @return the directory where the file will be placed
     */
    public String getDirectoryName( )
    {
        return txtDirectory_.getText( );
    }
}
