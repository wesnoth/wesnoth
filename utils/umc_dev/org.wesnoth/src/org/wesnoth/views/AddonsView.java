/*******************************************************************************
 * Copyright (c) 2011 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.views;

import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Map.Entry;

import org.eclipse.core.resources.WorkspaceJob;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.runtime.IStatus;
import org.eclipse.core.runtime.Status;
import org.eclipse.jface.action.Action;
import org.eclipse.jface.action.IMenuListener;
import org.eclipse.jface.action.IMenuManager;
import org.eclipse.jface.action.MenuManager;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.FillLayout;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Combo;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Menu;
import org.eclipse.swt.widgets.Table;
import org.eclipse.swt.widgets.TableColumn;
import org.eclipse.swt.widgets.TableItem;
import org.eclipse.ui.dialogs.PreferencesUtil;
import org.eclipse.ui.part.ViewPart;

import org.wesnoth.installs.SelectWesnothInstallDialog;
import org.wesnoth.preferences.AddonManagerPreferencePage;
import org.wesnoth.preferences.Preferences;
import org.wesnoth.preferences.Preferences.Paths;
import org.wesnoth.projects.ProjectUtils;
import org.wesnoth.utils.ExternalToolInvoker;
import org.wesnoth.utils.GUIUtils;
import org.wesnoth.utils.RunnableWithResult;
import org.wesnoth.utils.StringUtils;
import org.wesnoth.utils.WMLTools;


/**
 * A view that manages addons
 */
public class AddonsView extends ViewPart
{
    /**
     * The id of this addons view
     */
    public final static String ID_ADDONS_VIEW = "org.wesnoth.addonsView";

    private Combo              cmbAddonServer_;
    private List< String >     ports_;

    private String             currentPort_;
    /**
     * Flag whether we area already loading some addons or not
     */
    private boolean            loading_;
    private Table              tableAddons_;

    /**
     * Creates a new {@link AddonsView}
     */
    public AddonsView( )
    {
        ports_ = new ArrayList< String >( );

        cmbAddonServer_ = null;
        tableAddons_ = null;
    }

    @Override
    public void createPartControl( Composite parent )
    {
        parent.setLayout( new FillLayout( SWT.HORIZONTAL ) );

        Group grpAddonsList = new Group( parent, SWT.NONE );
        grpAddonsList.setText( "Addons list" );
        grpAddonsList.setLayout( new FillLayout( SWT.HORIZONTAL ) );

        tableAddons_ = new Table( grpAddonsList, SWT.BORDER
            | SWT.FULL_SELECTION );
        tableAddons_.setHeaderVisible( true );
        tableAddons_.setLinesVisible( true );

        TableColumn tblclmnType = new TableColumn( tableAddons_, SWT.NONE );
        tblclmnType.setWidth( 92 );
        tblclmnType.setText( "Type" );

        TableColumn tblclmnAddonName = new TableColumn( tableAddons_, SWT.NONE );
        tblclmnAddonName.setWidth( 100 );
        tblclmnAddonName.setText( "Addon Name" );

        TableColumn tblclmnAddonTitle = new TableColumn( tableAddons_, SWT.NONE );
        tblclmnAddonTitle.setWidth( 121 );
        tblclmnAddonTitle.setText( "Addon Title" );

        TableColumn tblclmnAuthors = new TableColumn( tableAddons_, SWT.NONE );
        tblclmnAuthors.setWidth( 139 );
        tblclmnAuthors.setText( "Author(s)" );

        TableColumn tblclmnVersion = new TableColumn( tableAddons_, SWT.NONE );
        tblclmnVersion.setWidth( 100 );
        tblclmnVersion.setText( "Version" );

        TableColumn tblclmnDownloads = new TableColumn( tableAddons_, SWT.NONE );
        tblclmnDownloads.setWidth( 100 );
        tblclmnDownloads.setText( "Downloads" );

        Group grpOptions = new Group( parent, SWT.NONE );
        grpOptions.setText( "Options" );
        grpOptions.setLayout( new GridLayout( 2, false ) );

        Label lblSelectAddonServer = new Label( grpOptions, SWT.NONE );
        lblSelectAddonServer.setText( "Select addon server: " );

        cmbAddonServer_ = new Combo( grpOptions, SWT.NONE );
        GridData gd_cmbAddonServer = new GridData( SWT.FILL, SWT.FILL, false,
            false, 1, 1 );
        gd_cmbAddonServer.widthHint = 148;
        cmbAddonServer_.setLayoutData( gd_cmbAddonServer );

        Button btnRefresh = new Button( grpOptions, SWT.NONE );
        btnRefresh.addSelectionListener( new SelectionAdapter( ) {
            @Override
            public void widgetSelected( SelectionEvent e )
            {
                refreshAddons( );
            }
        } );
        btnRefresh.setText( "Refresh" );

        Button btnOpenAddonManager = new Button( grpOptions, SWT.NONE );
        btnOpenAddonManager.addSelectionListener( new SelectionAdapter( ) {
            @Override
            public void widgetSelected( SelectionEvent e )
            {
                PreferencesUtil.createPreferenceDialogOn(
                    getViewSite( ).getShell( ),
                    AddonManagerPreferencePage.ID_ADDON_PREFERENCE_PAGE,
                    null, null ).open( );
            }
        } );
        btnOpenAddonManager.setText( "Open Addon Manager preferences" );

        cmbAddonServer_.addSelectionListener( new SelectionAdapter( ) {
            @Override
            public void widgetSelected( SelectionEvent e )
            {
                refreshAddons( );
            }
        } );

        ports_.clear( );
        // fill the addons
        for( Entry< String, String > server: AddonManagerPreferencePage.ADDON_SERVER_PORTS
            .entrySet( ) ) {

            cmbAddonServer_.add( String.format( "%s ( port: %s )",
                server.getValue( ), server.getKey( ) ) );
            ports_.add( server.getKey( ) );
        }

        MenuManager menuManager = new MenuManager( );
        menuManager.setRemoveAllWhenShown( true );
        menuManager.addMenuListener( new IMenuListener( ) {

            @Override
            public void menuAboutToShow( IMenuManager manager )
            {
                Action downloadAction = new Action( "Download" ) {
                    @Override
                    public void run( )
                    {
                        downloadAddon( );
                    }
                };
                manager.add( downloadAction );
            }
        } );

        Menu menu = menuManager.createContextMenu( tableAddons_ );
        tableAddons_.setMenu( menu );
    }

    /**
     * Downloads the currently selected addon
     */
    protected void downloadAddon( )
    {
        if( tableAddons_.getSelectionIndex( ) == - 1 ) {
            GUIUtils.showErrorMessageBox( "No addon selected" );
            return;
        }

        TableItem[] selection = tableAddons_.getSelection( );
        final String addonName = selection[0].getText( 1 );

        WorkspaceJob downloadJob = new WorkspaceJob( "Download" ) {

            @Override
            public IStatus runInWorkspace( final IProgressMonitor monitor )
                throws CoreException
            {
                monitor.beginTask( "Downloading addon " + addonName, 100 );

                String installName = Preferences.getDefaultInstallName( );

                RunnableWithResult< String > runnable = new RunnableWithResult< String >( ) {
                    @Override
                    public void run( )
                    {
                        // ask the user to select the install for the project
                        SelectWesnothInstallDialog dialog = new SelectWesnothInstallDialog(
                            null );
                        if( dialog.open( ) == SWT.OK ) {
                            setResult( dialog.getSelectedInstallName( ) );
                        }
                    }
                };

                Display.getDefault( ).syncExec( runnable );
                if( ! StringUtils.isNullOrEmpty( runnable.getResult( ) ) ) {
                    installName = runnable.getResult( );
                }

                final Paths paths = Preferences.getPaths( installName );

                OutputStream console = GUIUtils.createConsole(
                    "Wesnoth Addon Manager", null, false )
                    .newOutputStream( );

                ExternalToolInvoker tool = WMLTools.runWesnothAddonManager(
                    installName,
                    null,
                    currentPort_,
                    Arrays.asList( "-d", addonName, "-c",
                        paths.getAddonsDir( ) ),
                    new OutputStream[] { console },
                    new OutputStream[] { console } );

                tool.waitForTool( );

                monitor.worked( 50 );

                // ask user if he wants to create a project
                if( GUIUtils
                    .showMessageBox(
                        "Do you want to create a new project for the downloaded addon?",
                        SWT.YES | SWT.NO ) == SWT.YES ) {

                    ProjectUtils.createWesnothProject( addonName,
                        paths.getAddonsDir( ) + addonName, installName,
                        monitor );
                }
                monitor.done( );

                return Status.OK_STATUS;
            }
        };
        downloadJob.schedule( );
    }

    /**
     * Refreshes the list of addons
     */
    protected void refreshAddons( )
    {
        if( cmbAddonServer_ == null || tableAddons_ == null ) {
            return;
        }

        if( loading_ ) {
            GUIUtils
                .showInfoMessageBox( "Please wait for the previous query to finish." );
            return;
        }

        if( cmbAddonServer_.getSelectionIndex( ) == - 1 ) {
            return;
        }

        currentPort_ = ports_.get( cmbAddonServer_.getSelectionIndex( ) );
        if( StringUtils.isNullOrEmpty( currentPort_ ) ) {
            return;
        }

        loading_ = true;
        tableAddons_.setItemCount( 0 );
        tableAddons_.clearAll( );

        if( ! StringUtils.isNullOrEmpty( currentPort_ ) ) {
            WorkspaceJob loadAddons = new WorkspaceJob( "Retrieving list..." ) {

                @Override
                public IStatus runInWorkspace( IProgressMonitor monitor )
                    throws CoreException
                {
                    monitor.beginTask( "Retrieving list...", 100 );
                    monitor.worked( 10 );

                    String installName = Preferences.getDefaultInstallName( );

                    OutputStream stderr = GUIUtils.createConsole(
                        "Wesnoth Addon Manager", null, false )
                        .newOutputStream( );

                    ExternalToolInvoker tool = WMLTools.runWesnothAddonManager(
                        installName, null, currentPort_,
                        Arrays.asList( "-w", "-l" ), // list addons in raw
                                                     // mode
                        null, new OutputStream[] { stderr } );
                    tool.waitForTool( );

                    /**
                     * parse the contents columns = [[" 0 - type", "1 - name",
                     * "2 - title", "3 - author", "4 - version", "uploads",
                     * "5 - downloads", "size", "timestamp", "translate"]]
                     */
                    final String[] lines = StringUtils.getLines( tool
                        .getOutputContent( ) );
                    final List< String[] > addons = new ArrayList< String[] >( );

                    String[] tmpColumns = null;
                    int index = - 1;

                    for( String line: lines ) {
                        index = - 1;

                        if( line.startsWith( "\\ campaign" ) ) {
                            if( tmpColumns != null ) {
                                addons.add( tmpColumns );
                            }
                            tmpColumns = new String[6];
                        }
                        else if( line.startsWith( "  \\ type" ) ) {
                            index = 0;
                        }
                        else if( line.startsWith( "  \\ name" ) ) {
                            index = 1;
                        }
                        else if( line.startsWith( "  \\ title" ) ) {
                            index = 2;
                        }
                        else if( line.startsWith( "  \\ author" ) ) {
                            index = 3;
                        }
                        else if( line.startsWith( "  \\ version" ) ) {
                            index = 4;
                        }
                        else if( line.startsWith( "  \\ downloads" ) ) {
                            index = 5;
                        }

                        // got something interesting? parse it
                        if( tmpColumns != null && index != - 1 ) {
                            int firstIndex = line.indexOf( '\'' ) + 1;
                            int lastIndex = line.lastIndexOf( '\'' );

                            if( lastIndex < firstIndex ) {
                                lastIndex = line.length( );
                            }

                            tmpColumns[index] = line.substring(
                                firstIndex,
                                lastIndex ).trim( );
                        }
                    }

                    // need GUI Thread access
                    Display.getDefault( ).syncExec( new Runnable( ) {
                        @Override
                        public void run( )
                        {
                            // skipp 1st line since it's just the header
                            for( String[] addon: addons ) {

                                TableItem tableItem = new TableItem(
                                    tableAddons_, SWT.NONE );
                                tableItem.setText( new String[] { addon[0],
                                    addon[1], addon[2], addon[3], addon[4],
                                    addon[5] } );

                            }

                            tableAddons_.forceFocus( );
                        }
                    } );
                    loading_ = false;

                    monitor.worked( 100 );
                    monitor.done( );
                    return Status.OK_STATUS;
                }
            };
            loadAddons.schedule( );
        }
    }

    @Override
    public void setFocus( )
    {
    }
}
