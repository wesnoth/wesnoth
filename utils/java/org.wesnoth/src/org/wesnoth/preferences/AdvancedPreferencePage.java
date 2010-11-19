/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.preferences;

import org.eclipse.jface.preference.BooleanFieldEditor;
import org.wesnoth.Activator;
import org.wesnoth.Constants;
import org.wesnoth.Messages;

public class AdvancedPreferencePage extends AbstractPreferencePage
{
	public AdvancedPreferencePage()
	{
		super(GRID);

		setPreferenceStore(Activator.getDefault().getPreferenceStore());
		setDescription(Messages.AdvancedPreferencePage_0);
	}

	@Override
	protected void createFieldEditors()
	{
		addField(new BooleanFieldEditor(
				Constants.P_ADV_NO_TERRAIN_GFX, Messages.AdvancedPreferencePage_1, 1,
				getFieldEditorParent()),
				Messages.AdvancedPreferencePage_2 +
				Messages.AdvancedPreferencePage_3);
	}
}
