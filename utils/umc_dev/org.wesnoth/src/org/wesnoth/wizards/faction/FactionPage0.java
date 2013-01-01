/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.wizards.faction;

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
 * The first page of the New Faction Wizard
 */
public class FactionPage0 extends WizardPageTemplate
{
    private Text txtFileName_;
    private Text txtDirectory_;
    private Text txtFactionId_;
    private Text txtFactionName_;
    private Text txtType_;
    private Text txtLeader_;
    private Text txtRandomLeader_;
    private Text txtTerrainLiked_;
    private Text text;

    /**
     * Create the wizard.
     */
    public FactionPage0( )
    {
        super( "factionPage0" ); //$NON-NLS-1$
        setTitle( Messages.FactionPage0_1 );
        setDescription( Messages.FactionPage0_2 );
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

        ModifyListener modifyListener = new ModifyListener( ) {
            @Override
            public void modifyText( ModifyEvent e )
            {
                updatePageIsComplete( );
            }
        };

        setControl( container );
        container.setLayout( new GridLayout( 3, false ) );

        Label label = new Label( container, SWT.NONE );
        GridData gd_label = new GridData( SWT.FILL, SWT.CENTER, false, false,
            1, 1 );
        gd_label.widthHint = 95;
        label.setLayoutData( gd_label );
        label.setText( Messages.FactionPage0_3 );

        txtDirectory_ = new Text( container, SWT.BORDER );
        txtDirectory_.setLayoutData( new GridData( SWT.FILL, SWT.CENTER, true,
            false, 1, 1 ) );
        txtDirectory_.addModifyListener( modifyListener );
        txtDirectory_.setEditable( false );

        Button button = new Button( container, SWT.NONE );
        button.setText( Messages.FactionPage0_4 );
        button.addSelectionListener( new SelectionAdapter( ) {
            @Override
            public void widgetSelected( SelectionEvent e )
            {
                IPath path = handleBrowseContainer( );
                if( path != null ) {
                    txtDirectory_.setText( path.toString( ) );
                }
            }
        } );

        Label label_4 = new Label( container, SWT.NONE );
        label_4.setLayoutData( new GridData( SWT.FILL, SWT.CENTER, false,
            false, 1, 1 ) );
        label_4.setText( Messages.FactionPage0_5 );

        txtFileName_ = new Text( container, SWT.BORDER );
        txtFileName_.setLayoutData( new GridData( SWT.FILL, SWT.CENTER, false,
            false, 1, 1 ) );
        new Label( container, SWT.NONE );
        txtFileName_.addModifyListener( modifyListener );

        Label lblFactionId = new Label( container, SWT.NONE );
        lblFactionId.setLayoutData( new GridData( SWT.FILL, SWT.CENTER, false,
            false, 1, 1 ) );
        lblFactionId.setText( Messages.FactionPage0_6 );

        txtFactionId_ = new Text( container, SWT.BORDER );
        txtFactionId_.setLayoutData( new GridData( SWT.FILL, SWT.CENTER, true,
            false, 1, 1 ) );
        new Label( container, SWT.NONE );
        txtFactionId_.addModifyListener( modifyListener );

        Label lblName = new Label( container, SWT.NONE );
        lblName.setLayoutData( new GridData( SWT.FILL, SWT.CENTER, false,
            false, 1, 1 ) );
        lblName.setText( Messages.FactionPage0_7 );

        txtFactionName_ = new Text( container, SWT.BORDER );
        txtFactionName_.setLayoutData( new GridData( SWT.FILL, SWT.CENTER,
            true, false, 1, 1 ) );
        new Label( container, SWT.NONE );
        txtFactionName_.addModifyListener( modifyListener );

        Label lblType = new Label( container, SWT.NONE );
        lblType.setLayoutData( new GridData( SWT.FILL, SWT.CENTER, false,
            false, 1, 1 ) );
        lblType.setText( Messages.FactionPage0_8 );

        txtType_ = new Text( container, SWT.BORDER );
        txtType_.setLayoutData( new GridData( SWT.FILL, SWT.CENTER, true,
            false, 1, 1 ) );
        new Label( container, SWT.NONE );

        Label lblLeader = new Label( container, SWT.NONE );
        lblLeader.setLayoutData( new GridData( SWT.FILL, SWT.CENTER, false,
            false, 1, 1 ) );
        lblLeader.setText( Messages.FactionPage0_9 );

        txtLeader_ = new Text( container, SWT.BORDER );
        txtLeader_.setLayoutData( new GridData( SWT.FILL, SWT.CENTER, true,
            false, 1, 1 ) );
        new Label( container, SWT.NONE );

        Label lblRandomLeaders = new Label( container, SWT.NONE );
        lblRandomLeaders.setLayoutData( new GridData( SWT.FILL, SWT.CENTER,
            false, false, 1, 1 ) );
        lblRandomLeaders.setText( Messages.FactionPage0_10 );

        txtRandomLeader_ = new Text( container, SWT.BORDER );
        txtRandomLeader_.setLayoutData( new GridData( SWT.FILL, SWT.CENTER,
            true, false, 1, 1 ) );
        new Label( container, SWT.NONE );

        Label lblTerrainLiked = new Label( container, SWT.NONE );
        lblTerrainLiked.setLayoutData( new GridData( SWT.FILL, SWT.CENTER,
            false, false, 1, 1 ) );
        lblTerrainLiked.setText( Messages.FactionPage0_11 );

        txtTerrainLiked_ = new Text( container, SWT.BORDER );
        txtTerrainLiked_.setLayoutData( new GridData( SWT.FILL, SWT.CENTER,
            true, false, 1, 1 ) );
        new Label( container, SWT.NONE );

        Label lblRecruit = new Label( container, SWT.NONE );
        lblRecruit.setLayoutData( new GridData( SWT.FILL, SWT.CENTER, false,
            false, 1, 1 ) );
        lblRecruit.setText( Messages.FactionPage0_12 );

        text = new Text( container, SWT.BORDER );
        text.setLayoutData( new GridData( SWT.FILL, SWT.CENTER, true, false, 1,
            1 ) );
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
            .findMember( new Path( getDirectory( ) ) );
        setPageComplete( false );
        String fileName = getFileName( );

        if( getDirectory( ).isEmpty( ) ) {
            setErrorMessage( Messages.FactionPage0_13 );
            return;
        }

        if( container == null || ! container.exists( )
            || ! ( container instanceof IContainer ) ) {
            setErrorMessage( Messages.FactionPage0_14 );
            return;
        }

        if( fileName.isEmpty( ) ) {
            setErrorMessage( Messages.FactionPage0_15 );
            return;
        }

        if( fileName.replace( '\\', '/' ).indexOf( '/', 1 ) > 0 ) {
            setErrorMessage( Messages.FactionPage0_16 );
            return;
        }

        int dotLoc = fileName.lastIndexOf( '.' );
        if( dotLoc == - 1
            || fileName.substring( dotLoc + 1 ).equalsIgnoreCase( "cfg" ) == false ) //$NON-NLS-1$
        {
            setErrorMessage( Messages.FactionPage0_18 );
            return;
        }

        if( getFactionId( ).isEmpty( ) ) {
            setErrorMessage( Messages.FactionPage0_19 );
            return;
        }

        if( getFactionName( ).isEmpty( ) ) {
            setErrorMessage( Messages.FactionPage0_20 );
            return;
        }

        setErrorMessage( null );
        setPageComplete( true );
    }

    /**
     * @return The name of the file that contains the faction definition
     */
    public String getFileName( )
    {
        return txtFileName_.getText( );
    }

    /**
     * @return The directory where to create the faction file
     */
    public String getDirectory( )
    {
        return txtDirectory_.getText( );
    }

    /**
     * @return The ID of the new faction
     */
    public String getFactionId( )
    {
        return txtFactionId_.getText( );
    }

    /**
     * @return The name of the new faction
     */
    public String getFactionName( )
    {
        return txtFactionName_.getText( );
    }

    /**
     * @return The leader of the faction
     */
    public String getLeader( )
    {
        return txtLeader_.getText( );
    }

    /**
     * @return The faction's favourite terrain
     */
    public String getTerrainLiked( )
    {
        return txtTerrainLiked_.getText( );
    }

    /**
     * @return The random leader of the faction
     */
    public String getRandomLeader( )
    {
        return txtRandomLeader_.getText( );
    }

    /**
     * @return The type of the faction
     */
    public String getType( )
    {
        return txtType_.getText( );
    }
}
