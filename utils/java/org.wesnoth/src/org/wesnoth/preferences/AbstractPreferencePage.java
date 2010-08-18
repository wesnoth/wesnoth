/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.preferences;

import org.eclipse.jface.preference.FieldEditor;
import org.eclipse.jface.preference.FieldEditorPreferencePage;
import org.eclipse.ui.IWorkbench;
import org.eclipse.ui.IWorkbenchPreferencePage;

public abstract class AbstractPreferencePage extends FieldEditorPreferencePage implements
			IWorkbenchPreferencePage
{
	public AbstractPreferencePage(int style)
	{
		super(style);
	}
	public AbstractPreferencePage()
	{
		super();
	}

	/**
	 * Adds the specified field editor with the specified tooltip
	 * @param editor
	 * @param tooltip
	 */
	protected void addField(FieldEditor editor, String tooltip)
	{
		editor.getLabelControl(getFieldEditorParent()).setToolTipText(tooltip);
		super.addField(editor);
	}

	@Override
	public void init(IWorkbench workbench)
	{
	}
}
