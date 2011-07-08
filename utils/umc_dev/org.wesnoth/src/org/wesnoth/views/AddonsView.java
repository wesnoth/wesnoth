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
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.FillLayout;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Combo;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Table;
import org.eclipse.swt.widgets.TableColumn;
import org.eclipse.swt.widgets.TableItem;
import org.eclipse.ui.part.ViewPart;
import org.wesnoth.preferences.AddonUploadPreferencePage;
import org.wesnoth.utils.ExternalToolInvoker;
import org.wesnoth.utils.GUIUtils;
import org.wesnoth.utils.StringUtils;
import org.wesnoth.utils.WMLTools;

public class AddonsView extends ViewPart
{
    public final static String ID_ADDONS_VIEW = "org.wesnoth.addonsView";

    private Combo cmbAddonServer_;
    private List<String> ports_;

    private String currentPort_;
    /**
     * Flag whether we area already loading some addons or not
     */
    private boolean loading_;
    private Table table_;

    public AddonsView()
    {
        ports_ = new ArrayList<String>();
    }

    @Override
    public void createPartControl( Composite parent )
    {
        parent.setLayout(new FillLayout(SWT.VERTICAL));

        Composite composite = new Composite(parent, SWT.NONE);
        composite.setLayout(new GridLayout(2, false));

        Label lblSelectAddonServer = new Label(composite, SWT.NONE);
        lblSelectAddonServer.setText("Select addon server: ");

        cmbAddonServer_ = new Combo(composite, SWT.NONE);
        cmbAddonServer_.addSelectionListener(new SelectionAdapter() {
            @Override
            public void widgetSelected(SelectionEvent e) {
                if ( e.getSource( ) instanceof Combo == false)
                    return;

                Combo combo = (Combo) e.getSource( );
                if ( combo.getSelectionIndex( ) == -1 )
                    return;

                currentPort_ = ports_.get( combo.getSelectionIndex( ) );
                refreshAddons();
            }
        });

        GridData gd_cmbAddonServer = new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1);
        gd_cmbAddonServer.widthHint = 267;
        cmbAddonServer_.setLayoutData(gd_cmbAddonServer);

        Group grpAddonsList = new Group(composite, SWT.NONE);
        grpAddonsList.setText("Addons list");
        grpAddonsList.setLayout(new FillLayout(SWT.HORIZONTAL));
        GridData gd_grpAddonsList = new GridData(SWT.FILL, SWT.CENTER, false, false, 2, 1);
        gd_grpAddonsList.heightHint = 105;
        gd_grpAddonsList.widthHint = 579;
        grpAddonsList.setLayoutData(gd_grpAddonsList);

        table_ = new Table(grpAddonsList, SWT.BORDER | SWT.FULL_SELECTION);
        table_.setHeaderVisible(true);
        table_.setLinesVisible(true);

        TableColumn tblclmnType = new TableColumn(table_, SWT.NONE);
        tblclmnType.setWidth(92);
        tblclmnType.setText("Type");

        TableColumn tblclmnAddonName = new TableColumn(table_, SWT.NONE);
        tblclmnAddonName.setWidth(100);
        tblclmnAddonName.setText("Addon Name");

        TableColumn tblclmnAddonTitle = new TableColumn(table_, SWT.NONE);
        tblclmnAddonTitle.setWidth(121);
        tblclmnAddonTitle.setText("Addon Title");

        TableColumn tblclmnAuthors = new TableColumn(table_, SWT.NONE);
        tblclmnAuthors.setWidth(139);
        tblclmnAuthors.setText("Author(s)");

        TableColumn tblclmnVersion = new TableColumn(table_, SWT.NONE);
        tblclmnVersion.setWidth(100);
        tblclmnVersion.setText("Version");

        TableColumn tblclmnDownloads = new TableColumn(table_, SWT.NONE);
        tblclmnDownloads.setWidth(100);
        tblclmnDownloads.setText("Downloads");

        ports_.clear( );
        // fill the addons
        for ( Entry<String, String> server :
            AddonUploadPreferencePage.ADDON_SERVER_PORTS.entrySet( ) ) {

            cmbAddonServer_.add( String.format( "%s ( port: %s )"
                    , server.getValue( )
                    , server.getKey( ) ) );
            ports_.add( server.getKey( ) );
        }
    }

    /**
     * Refreshes the list of addons
     */
    protected void refreshAddons()
    {
        if ( loading_ ) {
            GUIUtils.showInfoMessageBox( "Please wait for the previous query to finish." );
            return;
        }

        loading_ = true;
        table_.clearAll( );

        if ( ! StringUtils.isNullOrEmpty( currentPort_ ) ) {
            WorkspaceJob loadAddons = new WorkspaceJob( "Retrieving list..." ) {

                @Override
                public IStatus runInWorkspace( IProgressMonitor monitor ) throws CoreException
                {
                    monitor.beginTask( "Retrieving list...", 100 );
                    String installName = "";

                    OutputStream stderr = GUIUtils.createConsole(
                            "Wesnoth Addon Manager", null, false ).newOutputStream( );

                    ExternalToolInvoker tool = WMLTools.runWesnothAddonManager(
                            installName,
                            null,
                            currentPort_,
                            Arrays.asList( "-l" ), // list addons
                            null,
                            new OutputStream[] { stderr } );
                    tool.waitForTool( );

                    /**
                     * parse the contents
                     *  columns = [["type", "name", "title", "author",
                     *   "version", "uploads", "downloads",
                     *   "size", "timestamp", "translate"]]
                     */
                    final String[] lines = StringUtils.getLines( tool.getOutputContent( ) );

                    // need GUI Thread access
                    Display.getDefault( ).syncExec( new Runnable( ) {
                        @Override
                        public void run()
                        {
                            // skipp 1st line since it's just the header
                            for ( int i = 1; i < lines.length; ++i ) {
                                String[] columns = lines[i].split( " +" );
                                //System.out.println( columns );

                                TableItem tableItem= new TableItem(table_, SWT.NONE);
                                tableItem.setText( new String[] {
                                        columns[0],
                                        columns[1],
                                        columns[2],
                                        columns[3],
                                        columns[4],
                                        columns[6]
                                });

                            }
                        }
                    });
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
    public void setFocus()
    {
    }
}
