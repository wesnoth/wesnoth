/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.preferences;

import java.util.HashMap;
import java.util.Map;
import java.util.Map.Entry;

import org.eclipse.jface.preference.BooleanFieldEditor;
import org.eclipse.jface.preference.StringFieldEditor;
import org.eclipse.xtext.ui.editor.preferences.fields.LabelFieldEditor;
import org.wesnoth.Activator;
import org.wesnoth.Constants;
import org.wesnoth.Messages;
import org.wesnoth.jface.RegexStringFieldEditor;

public class AddonUploadPreferencePage extends AbstractPreferencePage
{
	Map<String, String> ports_;

	public AddonUploadPreferencePage()
	{
		super(GRID);
		ports_ = new HashMap<String, String>();
		ports_.put("15002", "1.9.x"); //$NON-NLS-1$ //$NON-NLS-2$
		ports_.put("15001", "1.8.x"); //$NON-NLS-1$ //$NON-NLS-2$
		ports_.put("15003", "1.6.x"); //$NON-NLS-1$ //$NON-NLS-2$
		ports_.put("15005", "1.4.x"); //$NON-NLS-1$ //$NON-NLS-2$
		ports_.put("15004", "trunk"); //$NON-NLS-1$ //$NON-NLS-2$

		setPreferenceStore(Activator.getDefault().getPreferenceStore());
		setDescription(Messages.AddonUploadPreferencePage_10);
	}

	@Override
	protected void createFieldEditors()
	{
		addField(new StringFieldEditor(Constants.P_WAU_PASSWORD,
				Messages.AddonUploadPreferencePage_11, getFieldEditorParent()),
				Messages.AddonUploadPreferencePage_12);
		addField(new BooleanFieldEditor(Constants.P_WAU_VERBOSE,
				Messages.AddonUploadPreferencePage_13, 1, getFieldEditorParent()));

		addField(new RegexStringFieldEditor(Constants.P_WAU_ADDRESS,
				Messages.AddonUploadPreferencePage_14, Messages.AddonUploadPreferencePage_15,
				Messages.AddonUploadPreferencePage_16,
				getFieldEditorParent()),
				Messages.AddonUploadPreferencePage_17);

		StringBuilder ports = new StringBuilder();
		StringBuilder portsRegex = new StringBuilder();
		portsRegex.append("("); //$NON-NLS-1$
		for (Entry<String, String> item : ports_.entrySet())
		{
			portsRegex.append(item.getKey() + "|"); //$NON-NLS-1$
			ports.append(String.format("\t%s - %s\n", item.getKey(), item.getValue())); //$NON-NLS-1$
		}
		portsRegex.deleteCharAt(portsRegex.length() - 1);
		portsRegex.append(")*"); //$NON-NLS-1$

		//System.out.println(portsRegex.toString());
		addField(new RegexStringFieldEditor(Constants.P_WAU_PORT,
				Messages.AddonUploadPreferencePage_22, portsRegex.toString(),
				Messages.AddonUploadPreferencePage_23, getFieldEditorParent()),
				Messages.AddonUploadPreferencePage_24);
		addField(new LabelFieldEditor(Messages.AddonUploadPreferencePage_25 +
				ports.toString(), getFieldEditorParent()));
	}
}
