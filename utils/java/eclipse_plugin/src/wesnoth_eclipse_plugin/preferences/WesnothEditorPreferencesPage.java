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
import org.eclipse.jface.util.PropertyChangeEvent;
import org.eclipse.ui.IWorkbench;
import org.eclipse.ui.IWorkbenchPreferencePage;

import wesnoth_eclipse_plugin.Activator;
import wesnoth_eclipse_plugin.Constants;

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
public class WesnothEditorPreferencesPage extends FieldEditorPreferencePage
		implements IWorkbenchPreferencePage
{
	private DirectoryFieldEditor	wmlToolsField_;
	private List<String> 			wmlToolsList_;

	public WesnothEditorPreferencesPage() {
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
		addField(new FileFieldEditor(Constants.P_WESNOTH_EXEC_PATH,
				"Wesnoth executable path:", getFieldEditorParent()));
		addField(new DirectoryFieldEditor(Constants.P_WESNOTH_WORKING_DIR,
				"Working directory:", getFieldEditorParent()));
		addField(new DirectoryFieldEditor(Constants.P_WESNOTH_USER_DIR,
				"User data directory:", getFieldEditorParent()));

		wmlToolsField_ = new DirectoryFieldEditor(Constants.P_WESNOTH_WMLTOOLS_DIR,
				"WML* tools directory:", getFieldEditorParent());
		addField(wmlToolsField_);
	}

	@Override
	public void init(IWorkbench workbench)
	{
	}

	@Override
	protected void checkState()
	{
		super.checkState();
		setValid(false);

		for(String tool : wmlToolsList_)
		{
			if (!(new File(wmlToolsField_.getStringValue() + Path.SEPARATOR + tool).exists()))
			{
				setErrorMessage(String.format("'%s' cannot be found in the wml tools path",
						tool));
				return;
			}
		}

		setErrorMessage(null);
		setValid(true);
	}

	@Override
	public void propertyChange(PropertyChangeEvent event)
	{
		super.propertyChange(event);
		if (event.getProperty().equals(FieldEditor.VALUE))
			checkState();
	}
}