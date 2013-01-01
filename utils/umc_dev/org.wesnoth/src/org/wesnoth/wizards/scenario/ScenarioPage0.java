/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.wizards.scenario;

import java.io.File;

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
import org.eclipse.swt.widgets.FileDialog;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Spinner;
import org.eclipse.swt.widgets.Text;

import org.wesnoth.Messages;
import org.wesnoth.wizards.WizardPageTemplate;


/**
 * The "New" wizard page allows setting the container for the new file as well
 * as the file name. The page will only accept file name without the extension
 * OR with the extension that matches the expected one (cfg).
 */
public class ScenarioPage0 extends WizardPageTemplate
{
    private Text       txtProject_;

    private Text       txtFileName_;
    private Text       txtScenarioId_;
    private Text       txtScenarioName_;

    private GridData   gd_txtProject_;
    private Label      lblproject;
    private GridData   gd_txtFileName_;
    private GridData   gd_txtScenarioId_;
    private GridData   gd_txtScenarioName_;
    private Label      lblFileName;
    private Label      lblScenarioId;
    private Label      lblScenarioName;
    private Label      lblNextScenarioId;
    private Text       txtNextScenarioId_;
    private Label      lblNumberOfTurns;
    private Spinner    txtTurns_;
    private Label      lblMapData;
    private Text       txtMapData_;
    private Button     btnBrowseMap;
    private Button     chkEmbeddedMap_;

    private IContainer container_;
    private String     rawMapPath_;

    /**
     * Constructor for SampleNewWizardPage.
     * 
     * @param container
     *        The current selected container
     * @param pageName
     *        The name of the page
     */
    public ScenarioPage0( IContainer container )
    {
        super( "scenarioPage0" ); //$NON-NLS-1$
        setTitle( Messages.ScenarioPage0_1 );
        setDescription( Messages.ScenarioPage0_2 );

        container_ = container;
    }

    @Override
    public void createControl( Composite parent )
    {
        super.createControl( parent );
        Composite container = new Composite( parent, SWT.NULL );
        GridLayout layout = new GridLayout( );
        container.setLayout( layout );
        layout.numColumns = 3;
        layout.verticalSpacing = 9;

        ModifyListener modifyListener = new ModifyListener( ) {
            @Override
            public void modifyText( ModifyEvent e )
            {
                updatePageIsComplete( );
            }
        };

        lblproject = new Label( container, SWT.NULL );
        lblproject.setText( Messages.ScenarioPage0_3 );
        txtProject_ = new Text( container, SWT.BORDER | SWT.SINGLE );
        txtProject_.setEditable( false );
        gd_txtProject_ = new GridData( GridData.FILL_HORIZONTAL );
        txtProject_.setLayoutData( gd_txtProject_ );
        txtProject_.addModifyListener( new ModifyListener( ) {
            @Override
            public void modifyText( ModifyEvent e )
            {
                updatePageIsComplete( );
                updateMapPath( );
            }
        } );

        Button btnBrowse_ = new Button( container, SWT.PUSH );
        btnBrowse_.setText( Messages.ScenarioPage0_4 );
        btnBrowse_.addSelectionListener( new SelectionAdapter( ) {
            @Override
            public void widgetSelected( SelectionEvent e )
            {
                IPath path = handleBrowseContainer( );
                if( path != null ) {
                    txtProject_.setText( path.toString( ) );
                }
            }
        } );

        lblFileName = new Label( container, SWT.NULL );
        lblFileName.setText( Messages.ScenarioPage0_5 );
        txtFileName_ = new Text( container, SWT.BORDER | SWT.SINGLE );
        gd_txtFileName_ = new GridData( GridData.FILL_HORIZONTAL );
        txtFileName_.setLayoutData( gd_txtFileName_ );
        txtFileName_.addModifyListener( modifyListener );
        new Label( container, SWT.NONE );

        lblScenarioId = new Label( container, SWT.NULL );
        lblScenarioId.setText( Messages.ScenarioPage0_6 );
        txtScenarioId_ = new Text( container, SWT.BORDER | SWT.SINGLE );
        gd_txtScenarioId_ = new GridData( GridData.FILL_HORIZONTAL );
        txtScenarioId_.setLayoutData( gd_txtScenarioId_ );
        txtScenarioId_.addModifyListener( modifyListener );

        setControl( container );
        new Label( container, SWT.NONE );

        lblScenarioName = new Label( container, SWT.NULL );
        lblScenarioName.setText( Messages.ScenarioPage0_7 );
        txtScenarioName_ = new Text( container, SWT.BORDER | SWT.SINGLE );
        gd_txtScenarioName_ = new GridData( GridData.FILL_HORIZONTAL );
        txtScenarioName_.setLayoutData( gd_txtScenarioName_ );
        txtScenarioName_.addModifyListener( modifyListener );
        new Label( container, SWT.NONE );

        lblNextScenarioId = new Label( container, SWT.NONE );
        lblNextScenarioId.setText( Messages.ScenarioPage0_8 );

        txtNextScenarioId_ = new Text( container, SWT.BORDER );
        txtNextScenarioId_.setLayoutData( new GridData( SWT.FILL, SWT.CENTER,
            true, false, 1, 1 ) );
        new Label( container, SWT.NONE );

        lblNumberOfTurns = new Label( container, SWT.NONE );
        lblNumberOfTurns.setText( Messages.ScenarioPage0_9 );

        txtTurns_ = new Spinner( container, SWT.BORDER );
        txtTurns_.setMinimum( - 1 );
        txtTurns_.setSelection( - 1 );
        GridData gd_txtTurns_ = new GridData( SWT.LEFT, SWT.CENTER, false,
            false, 1, 1 );
        gd_txtTurns_.widthHint = 60;
        txtTurns_.setLayoutData( gd_txtTurns_ );
        new Label( container, SWT.NONE );

        lblMapData = new Label( container, SWT.NONE );
        lblMapData.setText( Messages.ScenarioPage0_10 );

        txtMapData_ = new Text( container, SWT.BORDER );
        txtMapData_.setLayoutData( new GridData( SWT.FILL, SWT.CENTER, true,
            false, 1, 1 ) );

        btnBrowseMap = new Button( container, SWT.NONE );
        btnBrowseMap.addSelectionListener( new SelectionAdapter( ) {
            @Override
            public void widgetSelected( SelectionEvent e )
            {
                handleBrowseMap( );
            }
        } );
        btnBrowseMap.setText( Messages.ScenarioPage0_11 );
        new Label( container, SWT.NONE );

        chkEmbeddedMap_ = new Button( container, SWT.CHECK );
        chkEmbeddedMap_.setText( Messages.ScenarioPage0_12 );
        new Label( container, SWT.NONE );

        if( getWizard( ).getSelectionContainer( ) != null ) {
            txtProject_.setText( getWizard( ).getSelectionContainer( )
                .getFullPath( ).toString( ) );
        }
        updatePageIsComplete( );
    }

    /**
     * Checks the mandatory fields and updates the isPageComplete status
     */
    private void updatePageIsComplete( )
    {
        setPageComplete( false );

        IResource container = ResourcesPlugin.getWorkspace( ).getRoot( )
            .findMember( new Path( getProjectName( ) ) );
        String fileName = getFileName( );
        String warningMessage = ""; //$NON-NLS-1$

        if( getProjectName( ).isEmpty( ) ) {
            setErrorMessage( Messages.ScenarioPage0_14 );
            return;
        }
        if( container == null
            || ( container.getType( ) & ( IResource.PROJECT | IResource.FOLDER ) ) == 0 ) {
            setErrorMessage( Messages.ScenarioPage0_15 );
            return;
        }

        if( ! container.isAccessible( ) ) {
            setErrorMessage( Messages.ScenarioPage0_16 );
            return;
        }

        if( ! getProjectName( ).contains( "scenarios" ) ) //$NON-NLS-1$
        {
            warningMessage += Messages.ScenarioPage0_18;
        }

        if( fileName.isEmpty( ) ) {
            setErrorMessage( Messages.ScenarioPage0_19 );
            return;
        }
        if( fileName.replace( '\\', '/' ).indexOf( '/', 1 ) > 0 ) {
            setErrorMessage( Messages.ScenarioPage0_20 );
            return;
        }

        int dotLoc = fileName.lastIndexOf( '.' );
        if( dotLoc == - 1
            || ! ( fileName.substring( dotLoc + 1 )
                .equalsIgnoreCase( "cfg" ) ) ) //$NON-NLS-1$
        {
            setErrorMessage( Messages.ScenarioPage0_22 );
            return;
        }

        if( txtScenarioId_.getText( ).isEmpty( ) ) {
            setErrorMessage( Messages.ScenarioPage0_23 );
            return;
        }

        setErrorMessage( null );
        setMessage( warningMessage.isEmpty( ) ? null: warningMessage, WARNING );
        setPageComplete( true );
    }

    private void updateMapPath( )
    {
        if( txtMapData_ == null ) {
            return;
        }

        if( rawMapPath_ == null ) {
            txtMapData_.setText( "" ); //$NON-NLS-1$
            return;
        }

        // make the map path to be relative to the ~addons directory

        String homePath = txtProject_.getText( );
        if( ! homePath.isEmpty( ) && homePath.charAt( 0 ) == '/' ) {
            homePath = homePath.substring( 1 );
        }

        txtMapData_.setText( String.format( "{~add-ons/%s/maps/%s}", //$NON-NLS-1$
            homePath, new File( rawMapPath_ ).getName( ) ) );
    }

    private void handleBrowseMap( )
    {
        FileDialog dialog = new FileDialog( getShell( ) );
        dialog.setText( Messages.ScenarioPage0_26 );
        if( container_ != null ) {
            dialog.setFilterPath( container_.getLocation( ).toOSString( ) );
        }
        dialog.setFilterExtensions( new String[] { "*.map" } ); //$NON-NLS-1$
        rawMapPath_ = dialog.open( );
        updateMapPath( );
    }

    /**
     * @return the project this new scenario will belong to
     */
    public String getProjectName( )
    {
        return txtProject_.getText( );
    }

    /**
     * @return the file name of the scenario
     */
    public String getFileName( )
    {
        return txtFileName_.getText( );
    }

    /**
     * @return the scenario id
     */
    public String getScenarioId( )
    {
        return txtScenarioId_.getText( );
    }

    /**
     * @return the scenario name
     */
    public String getScenarioName( )
    {
        return txtScenarioName_.getText( );
    }

    /**
     * @return the next scenario's id
     */
    public String getNextScenarioId( )
    {
        return txtNextScenarioId_.getText( );
    }

    /**
     * @return the number of scenario's turns
     */
    public int getTurnsNumber( )
    {
        return txtTurns_.getSelection( );
    }

    /**
     * @return the map data for the current scenario
     */
    public String getMapData( )
    {
        return txtMapData_.getText( );
    }

    /**
     * Returns the path to the scenario's map
     * 
     * @return the path to the scenario's map
     */
    public String getRawMapPath( )
    {
        return rawMapPath_;
    }

    /**
     * Returns true if the map should be embedded in the file
     * 
     * @return True if the map should be embedded in the file
     */
    public boolean getIsMapEmbedded( )
    {
        return chkEmbeddedMap_.getSelection( );
    }
}
