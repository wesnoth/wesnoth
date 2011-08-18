/*******************************************************************************
 * Copyright (c) 2011 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.preferences;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import org.eclipse.core.runtime.IPath;
import org.eclipse.core.runtime.Path;
import org.eclipse.jface.preference.DirectoryFieldEditor;
import org.eclipse.jface.preference.FieldEditor;
import org.eclipse.jface.preference.FileFieldEditor;
import org.eclipse.jface.preference.IPreferenceStore;
import org.eclipse.jface.preference.StringFieldEditor;
import org.eclipse.jface.util.PropertyChangeEvent;
import org.eclipse.jface.viewers.ArrayContentProvider;
import org.eclipse.jface.viewers.ITableLabelProvider;
import org.eclipse.jface.viewers.LabelProvider;
import org.eclipse.jface.viewers.TableViewer;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.FocusEvent;
import org.eclipse.swt.events.FocusListener;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.events.MouseAdapter;
import org.eclipse.swt.events.MouseEvent;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.events.VerifyEvent;
import org.eclipse.swt.events.VerifyListener;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.layout.FillLayout;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Combo;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Table;
import org.eclipse.swt.widgets.TableColumn;
import org.eclipse.swt.widgets.Text;
import org.eclipse.xtext.ui.editor.preferences.fields.LabelFieldEditor;

import org.wesnoth.Constants;
import org.wesnoth.Messages;
import org.wesnoth.WesnothPlugin;
import org.wesnoth.installs.WesnothInstall;
import org.wesnoth.installs.WesnothInstallsUtils;
import org.wesnoth.templates.ReplaceableParameter;
import org.wesnoth.templates.TemplateProvider;
import org.wesnoth.utils.GUIUtils;
import org.wesnoth.utils.StringUtils;

/**
 * Preference page that manages the Wesnoth installs
 */
public class WesnothInstallsPage extends AbstractPreferencePage
{
    private Text                          txtInstallName_;
    private Combo                         cmbVersion_;

    private Map< String, WesnothInstall > installs_;
    private Table                         installsTable_;
    private TableViewer                   installsTableViewer_;

    private DirectoryFieldEditor          wmlToolsField_;
    private DirectoryFieldEditor          wesnothWorkingDirField_;
    private DirectoryFieldEditor          wesnothUserDirField_;
    private FileFieldEditor               wesnothExecutableField_;

    private static List< String >         wmlToolsList_;
    private static String[]               wesnothExecutablePaths_;
    private static String[]               wesnothDataDirPaths_;
    private static String[]               wesnothUserDirPaths_;

    private Composite                     parentComposite_;

    static {
        wmlToolsList_ = new ArrayList< String >( );
        wmlToolsList_.add( "wmllint" ); //$NON-NLS-1$
        wmlToolsList_.add( "wmlindent" ); //$NON-NLS-1$
        wmlToolsList_.add( "wmlscope" ); //$NON-NLS-1$
        wmlToolsList_.add( "wesnoth_addon_manager" ); //$NON-NLS-1$

        String os = "linux"; //$NON-NLS-1$
        if( Constants.IS_MAC_MACHINE ) {
            os = "mac"; //$NON-NLS-1$
        }
        else if( Constants.IS_WINDOWS_MACHINE ) {
            os = "windows"; //$NON-NLS-1$
        }

        List< ReplaceableParameter > params = new ArrayList< ReplaceableParameter >( );
        params.add( new ReplaceableParameter(
            "$$home_path", System.getProperty( "user.home" ) ) ); //$NON-NLS-1$ //$NON-NLS-2$

        wesnothExecutablePaths_ = StringUtils.getLines( TemplateProvider
            .getInstance( )
            .getProcessedTemplate( os + "_exec", params ) ); //$NON-NLS-1$
        wesnothDataDirPaths_ = StringUtils.getLines( TemplateProvider
            .getInstance( )
            .getProcessedTemplate( os + "_data", params ) ); //$NON-NLS-1$
        wesnothUserDirPaths_ = StringUtils.getLines( TemplateProvider
            .getInstance( )
            .getProcessedTemplate( os + "_user", params ) ); //$NON-NLS-1$
    }

    /**
     * Creates a grid-style {@link WesnothInstallPage}
     */
    public WesnothInstallsPage( )
    {
        super( GRID );
        setPreferenceStore( WesnothPlugin.getDefault( ).getPreferenceStore( ) );
        setTitle( Messages.WesnothInstallsPage_0 );

        installs_ = new HashMap< String, WesnothInstall >( );

        List< WesnothInstall > installs = WesnothInstallsUtils.getInstalls( );
        for( WesnothInstall wesnothInstall: installs ) {
            installs_.put( wesnothInstall.getName( ), wesnothInstall );
        }

        setValid( true );
    }

    @Override
    protected void createFieldEditors( )
    {
        ModifyListener listener = new ModifyListener( ) {

            @Override
            public void modifyText( ModifyEvent e )
            {
                checkState( );
                guessDefaultPaths( );
            }
        };

        wesnothExecutableField_ = new FileFieldEditor( "", //$NON-NLS-1$
            Messages.WesnothPreferencesPage_5, getFieldEditorParent( ) );
        wesnothExecutableField_.getTextControl( getFieldEditorParent( ) )
            .addFocusListener( new FocusListener( ) {
                @Override
                public void focusLost( FocusEvent e )
                {
                    guessDefaultPaths( );
                }

                @Override
                public void focusGained( FocusEvent e )
                {
                }
            } );
        wesnothExecutableField_.getTextControl( getFieldEditorParent( ) )
            .addModifyListener( new ModifyListener( ) {

                @Override
                public void modifyText( ModifyEvent e )
                {
                    guessDefaultPaths( );
                }
            } );
        addField( wesnothExecutableField_, Messages.WesnothPreferencesPage_6 );

        wesnothWorkingDirField_ = new DirectoryFieldEditor( "", //$NON-NLS-1$
            Messages.WesnothPreferencesPage_7, getFieldEditorParent( ) );
        wesnothWorkingDirField_.getTextControl( getFieldEditorParent( ) )
            .addModifyListener( listener );
        addField( wesnothWorkingDirField_, Messages.WesnothPreferencesPage_8 );

        wesnothUserDirField_ = new DirectoryFieldEditor( "", //$NON-NLS-1$
            Messages.WesnothPreferencesPage_9, getFieldEditorParent( ) );
        addField( wesnothUserDirField_, Messages.WesnothPreferencesPage_10 );

        wmlToolsField_ = new DirectoryFieldEditor( "", //$NON-NLS-1$
            Messages.WesnothPreferencesPage_11, getFieldEditorParent( ) );
        addField( wmlToolsField_, Messages.WesnothPreferencesPage_12 );

        addField( new FileFieldEditor( Preferences.PYTHON_PATH,
            Messages.WesnothPreferencesPage_13, getFieldEditorParent( ) ) );

        addField( new LabelFieldEditor( Messages.WesnothPreferencesPage_14,
            getFieldEditorParent( ) ) );

        // update the default
        updateInterface( installs_.get( Preferences.getDefaultInstallName( ) ) );
        guessDefaultPaths( );
    }

    @Override
    protected Control createContents( Composite parent )
    {
        Composite installComposite = new Composite( parent, 0 );
        installComposite.setLayout( new GridLayout( 2, false ) );
        installComposite.setLayoutData( new GridData( SWT.FILL, SWT.FILL, true,
            true, 1, 1 ) );

        // create install manager
        installsTableViewer_ = new TableViewer( installComposite, SWT.BORDER
            | SWT.FULL_SELECTION );
        installsTable_ = installsTableViewer_.getTable( );
        installsTable_.addMouseListener( new MouseAdapter( ) {
            @Override
            public void mouseDown( MouseEvent e )
            {
                updateInterface( getSelectedInstall( ) );
            }
        } );
        installsTable_.setHeaderVisible( true );
        installsTable_.setLayoutData( new GridData( SWT.FILL, SWT.FILL, true,
            true, 1, 1 ) );

        TableColumn tblclmnName = new TableColumn( installsTable_, SWT.NONE );
        tblclmnName.setWidth( 150 );
        tblclmnName.setText( Messages.WesnothInstallsPage_5 );

        TableColumn tblclmnWesnothVersion = new TableColumn( installsTable_,
            SWT.NONE );
        tblclmnWesnothVersion.setWidth( 70 );
        tblclmnWesnothVersion.setText( Messages.WesnothInstallsPage_6 );

        TableColumn tblclmnIsDefault = new TableColumn( installsTable_,
            SWT.NONE );
        tblclmnIsDefault.setWidth( 70 );
        tblclmnIsDefault.setText( Messages.WesnothInstallsPage_7 );

        installsTableViewer_.setContentProvider( new ArrayContentProvider( ) );
        installsTableViewer_.setLabelProvider( new TableLabelProvider( ) );
        installsTableViewer_.setInput( installs_.values( ) );

        Composite composite = new Composite( installComposite, SWT.NONE );
        FillLayout fl_composite = new FillLayout( SWT.VERTICAL );
        fl_composite.spacing = 10;
        fl_composite.marginHeight = 10;
        composite.setLayout( fl_composite );
        GridData gd_composite = new GridData( SWT.FILL, SWT.CENTER, true,
            false, 1, 1 );
        gd_composite.widthHint = 80;
        composite.setLayoutData( gd_composite );

        Button btnNew = new Button( composite, SWT.NONE );
        btnNew.addSelectionListener( new SelectionAdapter( ) {
            @Override
            public void widgetSelected( SelectionEvent e )
            {
                newInstall( );
            }
        } );
        btnNew.setText( Messages.WesnothInstallsPage_8 );

        Button btnRemove = new Button( composite, SWT.NONE );
        btnRemove.addSelectionListener( new SelectionAdapter( ) {
            @Override
            public void widgetSelected( SelectionEvent e )
            {
                removeInstall( getSelectedInstall( ) );
            }
        } );
        btnRemove.setText( Messages.WesnothInstallsPage_9 );

        Button btnSetAsDefault = new Button( composite, SWT.NONE );
        btnSetAsDefault.addSelectionListener( new SelectionAdapter( ) {
            @Override
            public void widgetSelected( SelectionEvent e )
            {
                setInstallAsDefault( getSelectedInstall( ) );
            }
        } );
        btnSetAsDefault.setText( Messages.WesnothInstallsPage_10 );

        Label lblInstallName = new Label( parent, SWT.NONE );
        lblInstallName.setText( Messages.WesnothInstallsPage_11 );

        txtInstallName_ = new Text( parent, SWT.SINGLE );
        txtInstallName_.setText( Messages.WesnothInstallsPage_12 );
        txtInstallName_.setEditable( false );
        txtInstallName_.setLayoutData( new GridData( SWT.FILL, SWT.CENTER,
            false, false, 1, 1 ) );
        txtInstallName_.addVerifyListener( new VerifyListener( ) {

            private boolean isCharOk( char character )
            {
                return ( character >= 'a' && character <= 'z' )
                    || ( character >= 'A' && character <= 'Z' )
                    || ( character >= '0' && character <= '9' );
            }

            @Override
            public void verifyText( VerifyEvent e )
            {
                if( e.character == 0 ) {
                    // we got a text copied. Check for invalid chars.
                    for( int index = e.text.length( ) - 1; index >= 0; --index ) {
                        if( isCharOk( e.text.charAt( index ) ) == false ) {
                            e.doit = false;
                            break;
                        }
                    }

                }
                else {
                    e.doit = isCharOk( e.character ) || e.keyCode == SWT.BS
                        || e.keyCode == SWT.ARROW_LEFT
                        || e.keyCode == SWT.ARROW_RIGHT
                        || e.keyCode == SWT.DEL;
                }
            }
        } );

        Label lblVersion = new Label( parent, SWT.NONE );
        lblVersion.setText( Messages.WesnothInstallsPage_13 );

        cmbVersion_ = new Combo( parent, SWT.READ_ONLY );
        cmbVersion_.setLayoutData( new GridData( SWT.FILL, SWT.CENTER, true,
            false, 1, 1 ) );

        cmbVersion_.add( "1.9.x" ); //$NON-NLS-1$
        cmbVersion_.add( "trunk" ); //$NON-NLS-1$
        cmbVersion_.select( 0 );

        // create fields
        parentComposite_ = ( Composite ) super.createContents( parent );

        return parentComposite_;
    }

    protected void newInstall( )
    {
        updateInterface( null );
    }

    protected void setInstallAsDefault( WesnothInstall install )
    {
        if( install != null ) {
            Preferences.setDefaultInstallName( install.getName( ) );
            installsTableViewer_.refresh( );
        }
    }

    protected void removeInstall( WesnothInstall install )
    {
        if( install != null ) {
            installs_.remove( install.getName( ) );
            installsTableViewer_.refresh( );

            // unset all settings.
            IPreferenceStore prefs = Preferences.getPreferences( );
            String installPrefix = Preferences.getInstallPrefix( install
                .getName( ) );
            prefs.setToDefault( installPrefix + Preferences.WESNOTH_EXEC_PATH );
            prefs.setToDefault( installPrefix + Preferences.WESNOTH_USER_DIR );
            prefs.setToDefault( installPrefix
                + Preferences.WESNOTH_WMLTOOLS_DIR );
            prefs
                .setToDefault( installPrefix + Preferences.WESNOTH_WORKING_DIR );

            // unset the default install if this was that
            // and select another one (the first) - if any - as default
            if( install.getName( )
                .equals( Preferences.getDefaultInstallName( ) ) ) {
                Preferences.setDefaultInstallName( "" ); //$NON-NLS-1$

                if( ! installs_.isEmpty( ) ) {
                    // get the first item from the iterator
                    Iterator< WesnothInstall > itor = installs_.values( )
                        .iterator( );
                    setInstallAsDefault( itor.next( ) );
                }
            }

            // clear the current info
            newInstall( );
        }
    }

    private WesnothInstall getSelectedInstall( )
    {
        if( installsTable_.getSelectionIndex( ) == - 1 ) {
            return null;
        }
        return installs_.get( installsTable_.getSelection( )[0].getText( 0 ) );
    }

    /**
     * Updates the interface with the specified install
     * 
     * @param install
     *        The install
     */
    private void updateInterface( WesnothInstall install )
    {
        txtInstallName_.setText( install == null ? "": install.getName( ) ); //$NON-NLS-1$
        txtInstallName_.setEditable( install == null ? true: false );

        cmbVersion_.setText( install == null ? "": install.getVersion( ) ); //$NON-NLS-1$

        setFieldsPreferenceName(
            install == null ? "": Preferences.getInstallPrefix( install.getName( ) ), //$NON-NLS-1$
            true );
    }

    /**
     * Sets the fields's internal preference name based on the installPrefix
     * 
     * @param installPrefix
     *        The install prefix
     * @param loadPreferences
     *        True to load the current stored preference
     */
    private void setFieldsPreferenceName( String installPrefix,
        boolean loadPreferences )
    {
        wesnothExecutableField_.setPreferenceName( installPrefix
            + Preferences.WESNOTH_EXEC_PATH );

        wesnothUserDirField_.setPreferenceName( installPrefix
            + Preferences.WESNOTH_USER_DIR );

        wesnothWorkingDirField_.setPreferenceName( installPrefix
            + Preferences.WESNOTH_WORKING_DIR );

        wmlToolsField_.setPreferenceName( installPrefix
            + Preferences.WESNOTH_WMLTOOLS_DIR );

        if( loadPreferences ) {
            wesnothUserDirField_.setStringValue( "" ); //$NON-NLS-1$
            wesnothWorkingDirField_.setStringValue( "" ); //$NON-NLS-1$
            wesnothExecutableField_.setStringValue( "" ); //$NON-NLS-1$
            wmlToolsField_.setStringValue( "" ); //$NON-NLS-1$

            wesnothUserDirField_.load( );
            wesnothWorkingDirField_.load( );
            wesnothExecutableField_.load( );
            wmlToolsField_.load( );
        }
    }

    /**
     * Tries the list of available paths for current os
     */
    private void guessDefaultPaths( )
    {
        testAndSetPaths( wesnothExecutablePaths_, wesnothExecutableField_ );
        testAndSetPaths( wesnothDataDirPaths_, wesnothWorkingDirField_ );
        testAndSetPaths( wesnothUserDirPaths_, wesnothUserDirField_ );

        // guess the working dir based on executable's path
        Text textControl = wesnothWorkingDirField_
            .getTextControl( getFieldEditorParent( ) );

        String workingDirValue = wesnothWorkingDirField_.getStringValue( );
        String wesnothExecValue = wesnothExecutableField_.getStringValue( );

        IPath guessedWorkingDir = new Path( wesnothExecValue );
        // remove the filename
        guessedWorkingDir = guessedWorkingDir.removeLastSegments( 1 );
        // The working dir should contain the data directory
        guessedWorkingDir = guessedWorkingDir.append( "/data/" );

        if( workingDirValue.isEmpty( )
            && ! wesnothExecValue.isEmpty( )
            && new File( guessedWorkingDir.toOSString( ) ).exists( ) ) {

            workingDirValue = guessedWorkingDir.removeLastSegments( 1 )
                .toOSString( );
            // don't retain the /data/ directory
            textControl.setText( workingDirValue );
        }

        // guess the wmltools path
        if( wmlToolsField_.getStringValue( ).isEmpty( )
            && ! workingDirValue.isEmpty( ) ) {
            String path = workingDirValue + "/data/tools"; //$NON-NLS-1$
            if( testWMLToolsPath( path ) ) {
                wmlToolsField_.setStringValue( path );
            }
        }

        String userDirValue = wesnothUserDirField_.getStringValue( );
        // guess the userdata path
        if( userDirValue.isEmpty( ) && ! workingDirValue.isEmpty( ) ) {
            String path = workingDirValue + "/userdata"; //$NON-NLS-1$
            testAndSetPaths( new String[] { path }, wesnothUserDirField_ );
        }
    }

    /**
     * Tests for wmltools in the specified path
     * 
     * @param path
     * @return
     */
    private boolean testWMLToolsPath( String path )
    {
        for( String tool: wmlToolsList_ ) {
            if( ! ( new File( path + IPath.SEPARATOR + tool ).exists( ) ) ) {
                setErrorMessage( String.format(
                    Messages.WesnothPreferencesPage_24, tool ) );
                return false;
            }
        }
        return true;
    }

    /**
     * Tests the list of paths and if any path exists it will
     * set it as the string value to the field editor
     * if the field editor value is empty
     * 
     * @param list
     *        The list to search in
     * @param field
     *        The field to put the path in
     */
    private void testAndSetPaths( String[] list, StringFieldEditor field )
    {
        if( ! ( field.getStringValue( ).isEmpty( ) ) ) {
            return;
        }

        for( String path: list ) {
            if( new File( path ).exists( ) ) {
                field.setStringValue( path );
                return;
            }
        }
    }

    /**
     * Checks whether the fields are empty (contain no text)
     * and the combobox/name don't have any values also
     * (the user doesn't create a new install)
     * 
     * @return
     */
    private boolean isFieldsEmpty( )
    {
        return wmlToolsField_.getStringValue( ).isEmpty( )
            && wesnothExecutableField_.getStringValue( ).isEmpty( )
            && wesnothUserDirField_.getStringValue( ).isEmpty( )
            && wesnothWorkingDirField_.getStringValue( ).isEmpty( )
            && txtInstallName_.getText( ).isEmpty( );
    }

    /**
     * Saves the current install
     * 
     * @return true if the save was successfully, false otherwise
     */
    private boolean saveInstall( )
    {
        String installName = txtInstallName_.getText( );

        // if it's editable, it means we are creating a new install
        if( txtInstallName_.getEditable( ) ) {
            boolean isFieldsEmpty = isFieldsEmpty( );

            if( installName.isEmpty( ) == true ) {
                // if we haven't completed anything,
                // we can skip the saving without alerting the user.
                if( ! isFieldsEmpty ) {
                    GUIUtils
                        .showErrorMessageBox( Messages.WesnothInstallsPage_19 );
                }

                // we consider successfully save if the fields are all
                // empty
                return isFieldsEmpty;
            }

            if( cmbVersion_.getText( ).isEmpty( ) == true ) {
                GUIUtils.showErrorMessageBox( Messages.WesnothInstallsPage_20 );
                return false;
            }

            // update the fields preferences names
            setFieldsPreferenceName(
                Preferences.getInstallPrefix( installName ), false );

            WesnothInstall newInstall = new WesnothInstall( installName,
                cmbVersion_.getText( ) );

            installs_.put( installName, newInstall );

            // if there is not any install set as default, set this one
            if( Preferences.getDefaultInstallName( ).isEmpty( ) ) {
                setInstallAsDefault( newInstall );
            }

            // we are creating a new install. Clear the editable
            // flag after we save it, to prevent renaming.
            txtInstallName_.setEditable( false );
        }
        else if( getSelectedInstall( ) != null ) { // just saving
            // the fields are automatically saved by Eclipse.
            // we just need to save the new version.
            getSelectedInstall( ).setVersion( cmbVersion_.getText( ) );
        }

        installsTableViewer_.refresh( );

        return true;
    }

    /**
     * This method will unset invalid properties's values,
     * and saving only valid ones.
     */
    private boolean savePreferences( )
    {
        if( ! wesnothExecutableField_.isValid( ) ) {
            wesnothExecutableField_.setStringValue( "" ); //$NON-NLS-1$
        }
        if( ! wesnothUserDirField_.isValid( ) ) {
            wesnothUserDirField_.setStringValue( "" ); //$NON-NLS-1$
        }
        if( ! wesnothWorkingDirField_.isValid( ) ) {
            wesnothWorkingDirField_.setStringValue( "" ); //$NON-NLS-1$
        }
        if( ! wmlToolsField_.isValid( ) ) {
            wmlToolsField_.setStringValue( "" ); //$NON-NLS-1$
        }

        if( saveInstall( ) == false ) {
            return false;
        }

        WesnothInstallsUtils.setInstalls( installs_.values( ) );
        return true;
    }

    @Override
    public boolean performOk( )
    {
        return savePreferences( ) && super.performOk( );
    }

    @Override
    protected void checkState( )
    {
        super.checkState( );
        // we won't stop the user saving wrong values.
        setValid( true );
    }

    @Override
    public void propertyChange( PropertyChangeEvent event )
    {
        super.propertyChange( event );
        if( event.getProperty( ).equals( FieldEditor.VALUE ) ) {
            checkState( );
        }
    }

    private static class TableLabelProvider extends LabelProvider implements
        ITableLabelProvider
    {
        @Override
        public Image getColumnImage( Object element, int columnIndex )
        {
            return null;
        }

        @Override
        public String getColumnText( Object element, int columnIndex )
        {
            if( element instanceof WesnothInstall ) {
                WesnothInstall install = ( WesnothInstall ) element;

                if( columnIndex == 0 ) { // name
                    return install.getName( );
                }
                else if( columnIndex == 1 ) { // version
                    return install.getVersion( );
                }
                else if( columnIndex == 2 ) { // is Default ?

                    if( install.getName( ).equals(
                        Preferences.getDefaultInstallName( ) ) ) {
                        return Messages.WesnothInstallsPage_21;
                    }

                    return ""; //$NON-NLS-1$
                }
            }
            return ""; //$NON-NLS-1$
        }
    }
}
