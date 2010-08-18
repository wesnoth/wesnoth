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

public class AdvancedPreferencePage extends AbstractPreferencePage
{
	public AdvancedPreferencePage()
	{
		super(GRID);

		setPreferenceStore(Activator.getDefault().getPreferenceStore());
		setDescription("Advanced preferences");
	}

	@Override
	protected void createFieldEditors()
	{
		addField(new BooleanFieldEditor(
				Constants.P_ADV_NO_TERRAIN_GFX, "NO_TERRAIN_GFX defined", 1,
				getFieldEditorParent()),
				"If this is set the Terrain Graphics macros won't be parsed" +
				" => improved performance. Check this only if you need them.");
	}
}
