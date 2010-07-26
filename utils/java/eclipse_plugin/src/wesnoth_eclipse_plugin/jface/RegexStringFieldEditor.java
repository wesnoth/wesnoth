/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.jface;

import org.eclipse.jface.preference.StringFieldEditor;
import org.eclipse.swt.widgets.Composite;

public class RegexStringFieldEditor extends StringFieldEditor
{
	protected String regex_;
	protected String errorMessage_;

	/**
	 * An regex matcher string field editor.
	 * @param regex The regex to match this textbox's string
	 * @param errorMessage The message to show as error when field's text
	 * doesn't match the regex
	 */
	public RegexStringFieldEditor(String name, String labelText,
			String regex, String errorMessage, Composite parent)
	{
		super(name,labelText,parent);
		regex_ = regex;
		errorMessage_ = errorMessage;
	}

	@Override
	protected boolean checkState()
	{
		if (regex_ == null)
			return true;
		boolean matches = getTextControl().getText().matches(regex_);
		setErrorMessage(matches == false ? errorMessage_ : null);
		showErrorMessage();
		return matches;
	}
}
