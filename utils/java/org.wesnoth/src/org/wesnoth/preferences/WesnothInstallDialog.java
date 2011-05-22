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
import java.util.List;

import org.eclipse.core.runtime.Path;
import org.eclipse.jface.preference.DirectoryFieldEditor;
import org.eclipse.jface.preference.FileFieldEditor;
import org.eclipse.jface.preference.StringFieldEditor;
import org.eclipse.swt.events.FocusEvent;
import org.eclipse.swt.events.FocusListener;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Combo;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.Text;
import org.eclipse.ui.dialogs.SelectionDialog;
import org.eclipse.xtext.ui.editor.preferences.fields.LabelFieldEditor;
import org.wesnoth.Constants;
import org.wesnoth.Messages;
import org.wesnoth.templates.ReplaceableParameter;
import org.wesnoth.templates.TemplateProvider;
import org.wesnoth.utils.StringUtils;

public class WesnothInstallDialog extends SelectionDialog
{
    private Text txtInstallName_;
    private Combo cmbVersion_;

    private DirectoryFieldEditor    wmlToolsField_;
    private DirectoryFieldEditor    wesnothWorkingDirField_;
    private DirectoryFieldEditor    wesnothUserDirField_;
    private FileFieldEditor         wesnothExecutableField_;

    private List<String>            wmlToolsList_;

    private Composite               compositeParent_;

    /**
     * @wbp.parser.constructor
     */
    protected WesnothInstallDialog(Shell parentShell)
    {
        super(parentShell);

        wmlToolsList_ = new ArrayList<String>();
        wmlToolsList_.add("wmllint"); //$NON-NLS-1$
        wmlToolsList_.add("wmlindent"); //$NON-NLS-1$
        wmlToolsList_.add("wmlscope"); //$NON-NLS-1$
        wmlToolsList_.add("wesnoth_addon_manager"); //$NON-NLS-1$
    }

    public WesnothInstallDialog(Shell parentShell, String data)
    {
        this(parentShell);
    }

    @Override
    protected Control createDialogArea(Composite parent)
    {
        Composite composite = new Composite(parent, 0);
        compositeParent_ = composite;

        composite.setLayout(new GridLayout(10, false));
        composite.setLayoutData(new GridData(GridData.FILL_BOTH));
        ModifyListener listener = new ModifyListener() {

            @Override
            public void modifyText(ModifyEvent e)
            {
                checkState();
                guessDefaultPaths();
            }
        };

        wesnothExecutableField_ = new FileFieldEditor(Constants.P_WESNOTH_EXEC_PATH,
                Messages.WesnothPreferencesPage_5, composite);
        wesnothExecutableField_.getTextControl(composite).
            addFocusListener(new FocusListener() {
                @Override
                public void focusLost(FocusEvent e)
                {
                    checkState();
                    String wesnothExec = wesnothExecutableField_.getStringValue();
                    if (wesnothWorkingDirField_.getStringValue().isEmpty() &&
                        !wesnothExec.isEmpty() &&
                        new File(wesnothExec.substring(0,
                                wesnothExec.lastIndexOf(new File(wesnothExec).getName()))).exists())
                    {
                        wesnothWorkingDirField_.setStringValue(wesnothExec.substring(0,
                                wesnothExec.lastIndexOf(new File(wesnothExec).getName()))
                        );
                    }
                }
                @Override
                public void focusGained(FocusEvent e)
                {
                }
            });
        wesnothExecutableField_.getLabelControl(composite).setToolTipText(Messages.WesnothPreferencesPage_6);

        wesnothWorkingDirField_ = new DirectoryFieldEditor(Constants.P_WESNOTH_WORKING_DIR,
                Messages.WesnothPreferencesPage_7, composite);
        wesnothWorkingDirField_.getTextControl(composite).
            addModifyListener(listener);
        wesnothWorkingDirField_.getLabelControl(composite).setToolTipText(Messages.WesnothPreferencesPage_8);

        wesnothUserDirField_ = new DirectoryFieldEditor(Constants.P_WESNOTH_USER_DIR,
                Messages.WesnothPreferencesPage_9, composite);
        wesnothUserDirField_.getLabelControl(composite).setToolTipText(Messages.WesnothPreferencesPage_10);

        wmlToolsField_ = new DirectoryFieldEditor(Constants.P_WESNOTH_WMLTOOLS_DIR,
                Messages.WesnothPreferencesPage_11, composite);
        wmlToolsField_.getLabelControl(composite).setToolTipText(Messages.WesnothPreferencesPage_12);

        new FileFieldEditor(Constants.P_PYTHON_PATH, Messages.WesnothPreferencesPage_13, composite);

        new LabelFieldEditor(Messages.WesnothPreferencesPage_14, composite);
        guessDefaultPaths();

        return composite;
    }

    protected void checkState()
    {
        testWMLToolsPath(wmlToolsField_.getStringValue());
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
        Text textControl = wesnothWorkingDirField_.getTextControl(compositeParent_);

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
                wmlToolsField_.setErrorMessage(String.format(Messages.WesnothPreferencesPage_24,
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

    /**
     * This method will unsert invalid properties's values,
     * thus saving only valid ones.
     */
    private void unsetInvalidProperties()
    {
        if (!wesnothExecutableField_.isValid())
            wesnothExecutableField_.setStringValue(""); //$NON-NLS-1$
        if (!wesnothUserDirField_.isValid())
            wesnothUserDirField_.setStringValue(""); //$NON-NLS-1$
        if (!wesnothWorkingDirField_.isValid())
            wesnothWorkingDirField_.setStringValue(""); //$NON-NLS-1$
        if (!wmlToolsField_.isValid())
            wmlToolsField_.setStringValue(""); //$NON-NLS-1$
    }

    @Override
    protected void okPressed()
    {
        super.okPressed();
        unsetInvalidProperties();
    }
}
