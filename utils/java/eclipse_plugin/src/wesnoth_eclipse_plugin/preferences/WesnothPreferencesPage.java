/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.preferences;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import org.eclipse.core.runtime.Path;
import org.eclipse.jface.preference.DirectoryFieldEditor;
import org.eclipse.jface.preference.FieldEditor;
import org.eclipse.jface.preference.FieldEditorPreferencePage;
import org.eclipse.jface.preference.FileFieldEditor;
import org.eclipse.jface.preference.StringFieldEditor;
import org.eclipse.jface.util.PropertyChangeEvent;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.widgets.Text;
import org.eclipse.ui.IWorkbench;
import org.eclipse.ui.IWorkbenchPreferencePage;

import wesnoth_eclipse_plugin.Activator;
import wesnoth_eclipse_plugin.Constants;
import wesnoth_eclipse_plugin.templates.ReplaceableParameter;
import wesnoth_eclipse_plugin.templates.TemplateProvider;
import wesnoth_eclipse_plugin.utils.StringUtils;

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
public class WesnothPreferencesPage extends FieldEditorPreferencePage
		implements IWorkbenchPreferencePage
{
	private DirectoryFieldEditor	wmlToolsField_;
	private DirectoryFieldEditor	wesnothWorkingDirField_;
	private DirectoryFieldEditor	wesnothUserDirField_;
	private FileFieldEditor			wesnothExecutableField_;

	private List<String> 			wmlToolsList_;

	public WesnothPreferencesPage() {
		super(GRID);

		setPreferenceStore(Activator.getDefault().getPreferenceStore());
		setDescription("Wesnoth User-Made-Content Plugin preferences");

		wmlToolsList_ = new ArrayList<String>();
		wmlToolsList_.add("wmllint");
		wmlToolsList_.add("wmlindent");
		wmlToolsList_.add("wmlscope");
		wmlToolsList_.add("wesnoth_addon_manager");
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
		ModifyListener stateChecker = new ModifyListener() {
			@Override
			public void modifyText(ModifyEvent e)
			{
				checkState();
			}
		};

		wesnothExecutableField_ = new FileFieldEditor(Constants.P_WESNOTH_EXEC_PATH,
				"Wesnoth executable path:", getFieldEditorParent());
		wesnothExecutableField_.getTextControl(getFieldEditorParent()).
			addModifyListener(stateChecker);
		addField(wesnothExecutableField_);

		wesnothWorkingDirField_ = new DirectoryFieldEditor(Constants.P_WESNOTH_WORKING_DIR,
				"Working directory:", getFieldEditorParent());
		addField(wesnothWorkingDirField_);

		wesnothUserDirField_ = new DirectoryFieldEditor(Constants.P_WESNOTH_USER_DIR,
				"User data directory:", getFieldEditorParent());
		addField(wesnothUserDirField_);

		wmlToolsField_ = new DirectoryFieldEditor(Constants.P_WESNOTH_WMLTOOLS_DIR,
				"WML* tools directory:", getFieldEditorParent());
		wmlToolsField_.getTextControl(getFieldEditorParent()).
					addModifyListener(stateChecker);
		addField(wmlToolsField_);

		guessDefaultPaths();
	}

	@Override
	public void init(IWorkbench workbench)
	{
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
		String os = "linux";
		if (Constants.IS_MAC_MACHINE)
			os = "mac";
		else if (Constants.IS_WINDOWS_MACHINE)
			os = "windows";

		List<ReplaceableParameter> params = new ArrayList<ReplaceableParameter>();
		params.add(new ReplaceableParameter("$$home_path", System.getProperty("user.home")));

		testPaths(StringUtils.getLines(
				TemplateProvider.getInstance().getProcessedTemplate(os + "_exec", params)),
				wesnothExecutableField_);
		testPaths(StringUtils.getLines(
				TemplateProvider.getInstance().getProcessedTemplate(os + "_data", params)),
				wesnothWorkingDirField_);
		testPaths(StringUtils.getLines(
				TemplateProvider.getInstance().getProcessedTemplate(os + "_user", params)),
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
			String path = wesnothWorkingDirField_.getStringValue() + "/data/tools";
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
				setErrorMessage(String.format("'%s' cannot be found in the wml tools path",
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
			wesnothExecutableField_.setStringValue("");
		if (!wesnothUserDirField_.isValid())
			wesnothUserDirField_.setStringValue("");
		if (!wesnothWorkingDirField_.isValid())
			wesnothWorkingDirField_.setStringValue("");
		if (!wmlToolsField_.isValid())
			wmlToolsField_.setStringValue("");
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