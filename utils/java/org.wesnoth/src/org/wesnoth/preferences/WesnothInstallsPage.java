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
import java.util.List;
import java.util.Map;

import org.eclipse.core.runtime.Path;
import org.eclipse.jface.preference.DirectoryFieldEditor;
import org.eclipse.jface.preference.FieldEditor;
import org.eclipse.jface.preference.FileFieldEditor;
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
import org.wesnoth.templates.ReplaceableParameter;
import org.wesnoth.templates.TemplateProvider;
import org.wesnoth.utils.GUIUtils;
import org.wesnoth.utils.StringUtils;
import org.wesnoth.utils.WesnothInstallsUtils;
import org.wesnoth.utils.WesnothInstallsUtils.WesnothInstall;

public class WesnothInstallsPage extends AbstractPreferencePage
{
    private Text                    txtInstallName_;
    private Combo                   cmbVersion_;

    private Map<String, WesnothInstall>   installs_;
    private Table                   installsTable_;
    private TableViewer             installsTableViewer_;


    private DirectoryFieldEditor    wmlToolsField_;
    private DirectoryFieldEditor    wesnothWorkingDirField_;
    private DirectoryFieldEditor    wesnothUserDirField_;
    private FileFieldEditor         wesnothExecutableField_;

    private List<String>            wmlToolsList_;
    private Composite               parentComposite_;

    public WesnothInstallsPage()
    {
        super(GRID);
        setPreferenceStore(WesnothPlugin.getDefault().getPreferenceStore());
        setTitle("Wesnoth Installs Preferences");

        wmlToolsList_ = new ArrayList<String>();
        wmlToolsList_.add("wmllint"); //$NON-NLS-1$
        wmlToolsList_.add("wmlindent"); //$NON-NLS-1$
        wmlToolsList_.add("wmlscope"); //$NON-NLS-1$
        wmlToolsList_.add("wesnoth_addon_manager"); //$NON-NLS-1$

        installs_ = new HashMap<String, WesnothInstall>();
        // add the default install first
        installs_.put( "Default", new WesnothInstall( "Default", "" ) ); //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$

        List<WesnothInstall> installs = WesnothInstallsUtils.getInstalls( );
        for ( WesnothInstall wesnothInstall : installs )
        {
            installs_.put( wesnothInstall.Name, wesnothInstall );
        }
    }

    @Override
    protected void createFieldEditors()
    {
        ModifyListener listener = new ModifyListener() {

            @Override
            public void modifyText(ModifyEvent e)
            {
                checkState();
                guessDefaultPaths();
            }
        };

        wesnothExecutableField_ = new FileFieldEditor(Constants.P_WESNOTH_EXEC_PATH,
                Messages.WesnothPreferencesPage_5, getFieldEditorParent());
        wesnothExecutableField_.getTextControl(getFieldEditorParent()).
            addFocusListener(new FocusListener() {
                @Override
                public void focusLost(FocusEvent e)
                {
                    checkState();
                }
                @Override
                public void focusGained(FocusEvent e)
                {
                }
            });
        wesnothExecutableField_.getTextControl(getFieldEditorParent()).
            addModifyListener(new ModifyListener() {

                @Override
                public void modifyText(ModifyEvent e)
                {
                    checkState();
                }
            });
        addField(wesnothExecutableField_, Messages.WesnothPreferencesPage_6);

        wesnothWorkingDirField_ = new DirectoryFieldEditor(Constants.P_WESNOTH_WORKING_DIR,
                Messages.WesnothPreferencesPage_7, getFieldEditorParent());
        wesnothWorkingDirField_.getTextControl(getFieldEditorParent()).
            addModifyListener(listener);
        addField(wesnothWorkingDirField_, Messages.WesnothPreferencesPage_8);

        wesnothUserDirField_ = new DirectoryFieldEditor(Constants.P_WESNOTH_USER_DIR,
                Messages.WesnothPreferencesPage_9, getFieldEditorParent());
        addField(wesnothUserDirField_, Messages.WesnothPreferencesPage_10);

        wmlToolsField_ = new DirectoryFieldEditor(Constants.P_WESNOTH_WMLTOOLS_DIR,
                Messages.WesnothPreferencesPage_11, getFieldEditorParent());
        addField(wmlToolsField_, Messages.WesnothPreferencesPage_12);

        addField(new FileFieldEditor(Constants.P_PYTHON_PATH, Messages.WesnothPreferencesPage_13, getFieldEditorParent()));

        addField(new LabelFieldEditor(Messages.WesnothPreferencesPage_14, getFieldEditorParent()));

        guessDefaultPaths();
    }

    @Override
    protected Control createContents(Composite parent)
    {
        Composite installComposite = new Composite(parent, 0);
        installComposite.setLayout(new GridLayout(2, false));
        installComposite.setLayoutData(new GridData(SWT.FILL, SWT.FILL, true, true, 1, 1));

        // create install manager
        installsTableViewer_ = new TableViewer(installComposite, SWT.BORDER | SWT.FULL_SELECTION);
        installsTable_ = installsTableViewer_.getTable();
        installsTable_.addMouseListener(new MouseAdapter() {
            @Override
            public void mouseDown(MouseEvent e)
            {
                updateInterface(getSelectedInstall());
            }
        });
        installsTable_.setHeaderVisible(true);
        installsTable_.setLayoutData(new GridData(SWT.FILL, SWT.FILL, true, true, 1, 1));

        TableColumn tblclmnName = new TableColumn(installsTable_, SWT.NONE);
        tblclmnName.setWidth(150);
        tblclmnName.setText("Install Name");

        TableColumn tblclmnWesnothVersion = new TableColumn(installsTable_, SWT.NONE);
        tblclmnWesnothVersion.setWidth(70);
        tblclmnWesnothVersion.setText("Version");

        TableColumn tblclmnIsDefault = new TableColumn(installsTable_, SWT.NONE);
        tblclmnIsDefault.setWidth(70);
        tblclmnIsDefault.setText("Default?");

        installsTableViewer_.setContentProvider(new ContentProvider());
        installsTableViewer_.setLabelProvider(new TableLabelProvider());
        installsTableViewer_.setInput(installs_.values());

        Composite composite = new Composite(installComposite, SWT.NONE);
        FillLayout fl_composite = new FillLayout(SWT.VERTICAL);
        fl_composite.spacing = 10;
        fl_composite.marginHeight = 10;
        composite.setLayout(fl_composite);
        GridData gd_composite = new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1);
        gd_composite.widthHint = 80;
        composite.setLayoutData(gd_composite);

        Button btnNew = new Button(composite, SWT.NONE);
        btnNew.addSelectionListener(new SelectionAdapter() {
            @Override
            public void widgetSelected(SelectionEvent e) {
                newInstall();
            }
        });
        btnNew.setText("New");

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

        Label lblInstallName = new Label(parent, SWT.NONE);
        lblInstallName.setText("Install name:");

        txtInstallName_ = new Text(parent, SWT.SINGLE);
        txtInstallName_.setText( "Default" );
        txtInstallName_.setEditable( false );
        txtInstallName_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
        txtInstallName_.addVerifyListener(new VerifyListener() {

            private boolean isCharOk( char character )
            {
                return  ( character >= 'a' && character <= 'z' ) ||
                        ( character >= 'A' && character <= 'Z' ) ||
                        ( character >= '0' && character <= '9' );
            }

            @Override
            public void verifyText(VerifyEvent e)
            {
                if ( e.character == 0 )
                {
                    // we got a text copied. Check for invalid chars.
                    for ( int index = e.text.length() - 1; index >= 0; --index ) {
                        if ( isCharOk( e.text.charAt(index) ) == false ) {
                            e.doit = false;
                            break;
                        }
                    }

                } else {
                    e.doit = isCharOk( e.character ) ||
                             e.keyCode == SWT.BS ||
                             e.keyCode == SWT.ARROW_LEFT ||
                             e.keyCode == SWT.ARROW_RIGHT ||
                             e.keyCode == SWT.DEL;
                }
            }
        });

        Label lblVersion = new Label(parent, SWT.NONE);
        lblVersion.setText("Wesnoth Version:");

        cmbVersion_ = new Combo(parent, SWT.READ_ONLY);
        cmbVersion_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));

        cmbVersion_.add( "" ); //$NON-NLS-1$ // for default install
        cmbVersion_.add( "1.9.x" ); //$NON-NLS-1$
        cmbVersion_.add( "trunk" ); //$NON-NLS-1$

        // create fields
        parentComposite_ = (Composite) super.createContents(parent);

        return parentComposite_;
    }

    protected void newInstall()
    {
        updateInterface(null);
    }

    protected void setInstallAsDefault()
    {
        WesnothInstall install = getSelectedInstall();
        if (install != null) {
            Preferences.getPreferences().setValue(Constants.P_INST_DEFAULT_INSTALL, install.Name);
            installsTableViewer_.refresh();
        }
    }

    protected void removeInstall()
    {
        WesnothInstall install = getSelectedInstall();
        if (install != null && install.Name.equalsIgnoreCase( "default" ) == false){
            installs_.remove( install.Name );
            installsTableViewer_.refresh();

            // clear the current info
            newInstall( );
        }
    }

    private WesnothInstall getSelectedInstall()
    {
        if (installsTable_.getSelectionIndex() == -1)
            return null;
        return installs_.get(installsTable_.getSelection()[0].getText(0));
    }

    private void updateInterface(WesnothInstall install)
    {
        txtInstallName_.setText( install == null ? "" : install.Name ); //$NON-NLS-1$
        txtInstallName_.setEditable( install == null ? true : false );

        cmbVersion_.setText( install == null ? "" : install.Version ); //$NON-NLS-1$

        String installPrefix = Preferences.getInstallPrefix( install == null ? null : install.Name );

        wesnothExecutableField_.setPreferenceName( installPrefix + Constants.P_WESNOTH_EXEC_PATH );
        wesnothExecutableField_.load();

        wesnothUserDirField_.setPreferenceName( installPrefix + Constants.P_WESNOTH_USER_DIR );
        wesnothUserDirField_.load();

        wesnothWorkingDirField_.setPreferenceName( installPrefix + Constants.P_WESNOTH_WORKING_DIR );
        wesnothWorkingDirField_.load();

        wmlToolsField_.setPreferenceName( installPrefix + Constants.P_WESNOTH_WMLTOOLS_DIR );
        wmlToolsField_.load();
    }

    @Override
    protected void checkState()
    {
        super.checkState();
        setValid(true);

        String wesnothExec = wesnothExecutableField_.getStringValue();
        if ( new File( wesnothExec ).exists() ){
            String wesnothExecName = new File( wesnothExec ).getName();

            if (wesnothWorkingDirField_.getStringValue().isEmpty() &&
                !wesnothExec.isEmpty() &&
                new File(wesnothExec.substring(0,
                        wesnothExec.lastIndexOf(wesnothExecName))).exists())
            {
                wesnothWorkingDirField_.setStringValue(wesnothExec.substring(0,
                        wesnothExec.lastIndexOf(wesnothExecName)));
            }
        }

        testWMLToolsPath(wmlToolsField_.getStringValue());
        setErrorMessage(null);
    }

    /**
     * Tries the list of available paths for current os
     */
    private void guessDefaultPaths()
    {
        String os = "linux"; //$NON-NLS-1$
        if (Constants.IS_MAC_MACHINE)
            os = "mac"; //$NON-NLS-1$
        else if (Constants.IS_WINDOWS_MACHINE)
            os = "windows"; //$NON-NLS-1$

        List<ReplaceableParameter> params = new ArrayList<ReplaceableParameter>();
        params.add(new ReplaceableParameter("$$home_path", System.getProperty("user.home"))); //$NON-NLS-1$ //$NON-NLS-2$

        testPaths(StringUtils.getLines(
                TemplateProvider.getInstance().getProcessedTemplate(os + "_exec", params)), //$NON-NLS-1$
                wesnothExecutableField_);
        testPaths(StringUtils.getLines(
                TemplateProvider.getInstance().getProcessedTemplate(os + "_data", params)), //$NON-NLS-1$
                wesnothWorkingDirField_);
        testPaths(StringUtils.getLines(
                TemplateProvider.getInstance().getProcessedTemplate(os + "_user", params)), //$NON-NLS-1$
                wesnothUserDirField_);

        // guess the working dir based on executable's path
        Text textControl = wesnothWorkingDirField_.getTextControl(
                getFieldEditorParent());

        String wesnothExec = wesnothExecutableField_.getStringValue();
        if (wesnothWorkingDirField_.getStringValue().isEmpty() &&
            !wesnothExec.isEmpty() &&
            new File(wesnothExec.substring(0,
                    wesnothExec.lastIndexOf(new File(wesnothExec).getName()))).exists())
        {
            textControl.setText(wesnothExec.substring(0,
                    wesnothExec.lastIndexOf(new File(wesnothExec).getName()))
            );
        }

        // guess the wmltools path
        if (wmlToolsField_.getStringValue().isEmpty() &&
            !wesnothWorkingDirField_.getStringValue().isEmpty())
        {
            String path = wesnothWorkingDirField_.getStringValue() + "/data/tools"; //$NON-NLS-1$
            if (testWMLToolsPath(path))
                wmlToolsField_.setStringValue(path);
        }

        // guess the userdata path
        if (wesnothUserDirField_.getStringValue().isEmpty() &&
            !wesnothWorkingDirField_.getStringValue().isEmpty())
        {
            String path = wesnothWorkingDirField_.getStringValue() + "/userdata"; //$NON-NLS-1$
            testPaths(new String[] { path },wesnothUserDirField_);
        }

        checkState();
    }

    /**
     * Tests for wmltools in the specified path
     * @param path
     * @return
     */
    private boolean testWMLToolsPath(String path)
    {
        for(String tool : wmlToolsList_)
        {
            if (!(new File(path + Path.SEPARATOR + tool).exists()))
            {
                setErrorMessage(String.format(Messages.WesnothPreferencesPage_24,
                        tool));
                return false;
            }
        }
        return true;
    }

    /**
     * Tests the list of paths and if any path exists it will
     * set it as the string value to the field editor
     * if the field editor value is empty
     * @param list The list to search in
     * @param field The field to put the path in
     */
    private void testPaths(String[] list, StringFieldEditor field)
    {
        if (!(field.getStringValue().isEmpty()))
            return;

        for(String path : list)
        {
            if (new File(path).exists())
            {
                field.setStringValue(path);
                return;
            }
        }
    }

    private void saveInstall()
    {
        String installName = txtInstallName_.getText();

        // we are creating a new install. Clear the editable
        // flag after we save it, to prevent renaming.
        if ( txtInstallName_.getEditable() &&
             installName.isEmpty() == false ) {

            // do some checks first
            if ( installName.equalsIgnoreCase( "default" ) ){
                GUIUtils.showInfoMessageBox( "Cannot create an install with the 'Default' name." );
                return;
            }

            if ( cmbVersion_.getText().isEmpty() == true ) {
                GUIUtils.showInfoMessageBox(
                        "Please select a version before creating a new install." );
               return;
            }

            WesnothInstall newInstall = new WesnothInstall(installName,
                    cmbVersion_.getText());

            installs_.put( installName, newInstall );
            installsTableViewer_.refresh();

            txtInstallName_.setEditable( false );
        }
    }

    /**
     * This method will unset invalid properties's values,
     * and saving only valid ones.
     */
    private void savePreferences()
    {
        if (!wesnothExecutableField_.isValid())
            wesnothExecutableField_.setStringValue(""); //$NON-NLS-1$
        if (!wesnothUserDirField_.isValid())
            wesnothUserDirField_.setStringValue(""); //$NON-NLS-1$
        if (!wesnothWorkingDirField_.isValid())
            wesnothWorkingDirField_.setStringValue(""); //$NON-NLS-1$
        if (!wmlToolsField_.isValid())
            wmlToolsField_.setStringValue(""); //$NON-NLS-1$

        saveInstall();
        WesnothInstallsUtils.setInstalls( installs_.values( ) );
    }

    @Override
    public boolean performOk()
    {
        savePreferences();
        return super.performOk();
    }

    @Override
    public void propertyChange(PropertyChangeEvent event)
    {
        super.propertyChange(event);
        if (event.getProperty().equals(FieldEditor.VALUE))
            checkState();
    }

    private static class ContentProvider extends ArrayContentProvider {
        @Override
        public Object[] getElements(Object inputElement)
        {
            return super.getElements(inputElement);
        }
    }

    private static class TableLabelProvider extends LabelProvider implements ITableLabelProvider {
        public Image getColumnImage(Object element, int columnIndex) {
            return null;
        }
        public String getColumnText(Object element, int columnIndex) {
            if (element instanceof WesnothInstall){
                WesnothInstall install = ( WesnothInstall ) element;

                if (columnIndex == 0) { // name
                   return install.Name;
                } else if (columnIndex == 1) { // version
                    return install.Version;
                } else if ( columnIndex == 2 ) { // is Default ?

                    if ( install.Name.equals( Preferences.getString( Constants.P_INST_DEFAULT_INSTALL ) ) )
                        return "Yes";

                    return "";
                }
            }
            return ""; //$NON-NLS-1$
        }
    }
}
