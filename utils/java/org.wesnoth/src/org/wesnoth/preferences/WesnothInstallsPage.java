/*******************************************************************************
 * Copyright (c) 2011 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.preferences;

import java.util.ArrayList;
import java.util.List;

import org.eclipse.jface.preference.PreferencePage;
import org.eclipse.jface.viewers.ArrayContentProvider;
import org.eclipse.jface.viewers.ITableLabelProvider;
import org.eclipse.jface.viewers.LabelProvider;
import org.eclipse.jface.viewers.TableViewer;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.MouseAdapter;
import org.eclipse.swt.events.MouseEvent;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.layout.FillLayout;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Table;
import org.eclipse.swt.widgets.TableColumn;
import org.eclipse.ui.IWorkbench;
import org.eclipse.ui.IWorkbenchPreferencePage;
import org.wesnoth.WesnothPlugin;

public class WesnothInstallsPage extends PreferencePage implements IWorkbenchPreferencePage
{
    private static class ContentProvider extends ArrayContentProvider {
        @Override
        public Object[] getElements(Object inputElement)
        {
            return super.getElements(inputElement);
        }
    }

    private class TableLabelProvider extends LabelProvider implements ITableLabelProvider {
        public Image getColumnImage(Object element, int columnIndex) {
            return null;
        }
        public String getColumnText(Object element, int columnIndex) {
            if (element instanceof WesnothInstall){

                if (columnIndex == 0){ // name
                   return ((WesnothInstall)element).Name;
                }else if (columnIndex == 1){ // version
                    return ((WesnothInstall)element).Version;
                }else if ( columnIndex == 2 ) { // default or not
                    return ((WesnothInstall)element).Default == true ? "Default" : "";
                }
            }
            return "";
        }
    }

    public static class WesnothInstall
    {
        public String Name;
        public String Version;
        public boolean Default;

        public WesnothInstall(String name, String version)
        {
            Name = name;
            Version = version;
        }
    }

    private List<WesnothInstall> _installs;

    public WesnothInstallsPage()
    {
        _installs = new ArrayList<WesnothInstall>();

        setPreferenceStore(WesnothPlugin.getDefault().getPreferenceStore());
        setTitle("Wesnoth Installs Preferences");
    }

    @Override
    protected Control createContents(Composite parent)
    {
        Composite comp = new Composite(parent, 0);
        comp.setLayout(new GridLayout(2, false));

        TableViewer tableViewer = new TableViewer(comp, SWT.BORDER | SWT.FULL_SELECTION);
        Table table = tableViewer.getTable();
        table.addMouseListener(new MouseAdapter() {
            @Override
            public void mouseDoubleClick(MouseEvent e) {
                editInstall();
            }
        });
        table.setHeaderVisible(true);
        table.setLayoutData(new GridData(SWT.FILL, SWT.FILL, true, true, 1, 1));

        TableColumn tblclmnName = new TableColumn(table, SWT.NONE);
        tblclmnName.setWidth(100);
        tblclmnName.setText("Install Name");

        TableColumn tblclmnWesnothVersion = new TableColumn(table, SWT.NONE);
        tblclmnWesnothVersion.setWidth(100);
        tblclmnWesnothVersion.setText("Wesnoth version");
        tableViewer.setContentProvider(new ContentProvider());
        tableViewer.setLabelProvider(new TableLabelProvider());
        tableViewer.setInput(_installs);

        Composite composite = new Composite(comp, SWT.NONE);
        FillLayout fl_composite = new FillLayout(SWT.VERTICAL);
        fl_composite.spacing = 10;
        fl_composite.marginHeight = 10;
        composite.setLayout(fl_composite);
        GridData gd_composite = new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1);
        gd_composite.widthHint = 23;
        composite.setLayoutData(gd_composite);

        Button btnAdd = new Button(composite, SWT.NONE);
        btnAdd.addSelectionListener(new SelectionAdapter() {
            @Override
            public void widgetSelected(SelectionEvent e) {
                addInstall();
            }
        });
        btnAdd.setText("Add");

        Button btnEdit = new Button(composite, SWT.NONE);
        btnEdit.addSelectionListener(new SelectionAdapter() {
            @Override
            public void widgetSelected(SelectionEvent e) {
                editInstall();
            }
        });
        btnEdit.setText("Edit");

        Button btnRemove = new Button(composite, SWT.NONE);
        btnRemove.addSelectionListener(new SelectionAdapter() {
            @Override
            public void widgetSelected(SelectionEvent e) {
                removeInstall();
            }
        });
        btnRemove.setText("Remove");

        Button btnSetAsDefault = new Button(composite, SWT.NONE);
        btnSetAsDefault.addSelectionListener(new SelectionAdapter() {
            @Override
            public void widgetSelected(SelectionEvent e) {
                setInstallAsDefault();
            }
        });
        btnSetAsDefault.setText("Set as default");

        return comp;
    }

    protected void removeInstall()
    {

    }

    protected void addInstall()
    {
        WesnothInstallDialog dlg = new WesnothInstallDialog(getShell());
        dlg.setTitle("Add a new Wesnoth Install");
        dlg.open();
    }

    protected void editInstall()
    {
        WesnothInstallDialog dlg = new WesnothInstallDialog(getShell());
        dlg.setTitle("Edit the Wesnoth Install");
        dlg.open();
    }

    protected void setInstallAsDefault()
    {

    }
    @Override
    public void init(IWorkbench workbench)
    {
        // load the installs strings

        // dummy entries
        _installs.add(new WesnothInstall("name1", "1.9.0"));
        _installs.add(new WesnothInstall("name2", "1.9.0+svn"));
        _installs.add(new WesnothInstall("name3", "1.9.0x"));
    }

    @Override
    public boolean performOk()
    {
        // pack settings
        return true;
    }
}
