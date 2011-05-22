/*******************************************************************************
 * Copyright (c) 2011 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.preferences;

import org.eclipse.jface.preference.PreferencePage;
import org.eclipse.jface.viewers.ArrayContentProvider;
import org.eclipse.jface.viewers.ITableLabelProvider;
import org.eclipse.jface.viewers.LabelProvider;
import org.eclipse.jface.viewers.TableViewer;
import org.eclipse.swt.SWT;
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
            return element.toString();
        }
    }

    private Button btnAdd_;
    private Button btnEdit_;
    private Button btnRemove_;
    private Button btnSetAsDefault_;
    public WesnothInstallsPage()
    {
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
        table.setHeaderVisible(true);
        table.setLayoutData(new GridData(SWT.FILL, SWT.FILL, true, true, 1, 1));

        TableColumn tblclmnName = new TableColumn(table, SWT.NONE);
        tblclmnName.setWidth(100);
        tblclmnName.setText("Name");

        TableColumn tblclmnWesnothVersion = new TableColumn(table, SWT.NONE);
        tblclmnWesnothVersion.setWidth(100);
        tblclmnWesnothVersion.setText("Wesnoth version");
        tableViewer.setContentProvider(new ContentProvider());
        tableViewer.setLabelProvider(new TableLabelProvider());

        Composite composite = new Composite(comp, SWT.NONE);
        FillLayout fl_composite = new FillLayout(SWT.VERTICAL);
        fl_composite.spacing = 10;
        fl_composite.marginHeight = 10;
        composite.setLayout(fl_composite);
        GridData gd_composite = new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1);
        gd_composite.widthHint = 23;
        composite.setLayoutData(gd_composite);

        btnAdd_ = new Button(composite, SWT.NONE);
        btnAdd_.setText("Add");

        btnEdit_ = new Button(composite, SWT.NONE);
        btnEdit_.setText("Edit");

        btnRemove_ = new Button(composite, SWT.NONE);
        btnRemove_.setText("Remove");

        btnSetAsDefault_ = new Button(composite, SWT.NONE);
        btnSetAsDefault_.setText("Set as default");

        return comp;
    }

    @Override
    public void init(IWorkbench workbench)
    {
    }
}
