/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
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
import org.eclipse.jface.preference.FieldEditor;
import org.eclipse.jface.preference.FileFieldEditor;
import org.eclipse.jface.preference.StringFieldEditor;
import org.eclipse.jface.util.PropertyChangeEvent;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.widgets.Text;
import org.eclipse.xtext.ui.editor.preferences.fields.LabelFieldEditor;
import org.wesnoth.Activator;
import org.wesnoth.Constants;
import org.wesnoth.Messages;
import org.wesnoth.templates.ReplaceableParameter;
import org.wesnoth.templates.TemplateProvider;
import org.wesnoth.utils.StringUtils;

/**
 * This class represents a preference page that
 * is contributed to the Preferences dialog. By
 * subclassing <samp>FieldEditorPreferencePage</samp>, we
 * can use the field support built into JFace that allows
 * us to create a page that is small and knows how to
 * save, restore and apply itself.
 * <p>
 * This page is used to modify preferences only. They
 * are stored in the preference store that belongs to
 * the main plug-in class. That way, preferences can
 * be accessed directly via the preference store.
 */
public class WesnothPreferencesPage extends AbstractPreferencePage
{
	private DirectoryFieldEditor	wmlToolsField_;
	private DirectoryFieldEditor	wesnothWorkingDirField_;
	private DirectoryFieldEditor	wesnothUserDirField_;
	private FileFieldEditor			wesnothExecutableField_;

	private List<String> 			wmlToolsList_;

	public WesnothPreferencesPage() {
		super(GRID);

		setPreferenceStore(Activator.getDefault().getPreferenceStore());
		setDescription(Messages.WesnothPreferencesPage_0);

		wmlToolsList_ = new ArrayList<String>();
		wmlToolsList_.add("wmllint"); //$NON-NLS-1$
		wmlToolsList_.add("wmlindent"); //$NON-NLS-1$
		wmlToolsList_.add("wmlscope"); //$NON-NLS-1$
		wmlToolsList_.add("wesnoth_addon_manager"); //$NON-NLS-1$
	}

	/**
	 * Creates the field editors. Field editors are abstractions of
	 * the common GUI blocks needed to manipulate various types
	 * of preferences. Each field editor knows how to save and
	 * restore itself.
	 */
	@Override
	public void createFieldEditors()
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
			addModifyListener(new ModifyListener() {
				@Override
				public void modifyText(ModifyEvent e)
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
	protected void checkState()
	{
		super.checkState();
		setValid(true);
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
	 * Tests the list of paths and if any path exists will
	 * set it as the string value to the field editor
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
	protected void performApply()
	{
		unsetInvalidProperties();
		super.performApply();
	}

	@Override
	public boolean performOk()
	{
		unsetInvalidProperties();
		return super.performOk();
	}

	@Override
	public void propertyChange(PropertyChangeEvent event)
	{
		super.propertyChange(event);
		if (event.getProperty().equals(FieldEditor.VALUE))
			checkState();
	}
}