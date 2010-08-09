/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.labeling.wmldoc;

import java.util.ArrayList;
import java.util.List;

import org.eclipse.swt.SWT;
import org.eclipse.swt.custom.StyleRange;
import org.eclipse.swt.widgets.Display;

import wesnoth_eclipse_plugin.preprocessor.Define;

/**
 * Provides WMLDoc info for a macro
 */
public class WMLDocMacro implements IWMLDocProvider
{
	private Define macro_;
	private String title_;
	private String contents_;
	private List<StyleRange> styleRanges_;

	public WMLDocMacro(Define macro)
	{
		macro_ = macro;
	}

	/**
	 * A method used for lazly generating the documentation
	 */
	private void generateDoc()
	{
		if (title_ != null && contents_ != null && styleRanges_ != null)
			return;

		styleRanges_ = new ArrayList<StyleRange>();
		title_ = "Macro: " + macro_.getName();
		StringBuilder content = new StringBuilder();

		content.append("Definition:\n");
		addStyleRange(0, content.length() - 1, SWT.BOLD);
		content.append(macro_.getValue());

		content.append('\n');
		if (macro_.getArguments().isEmpty() == false)
		{
			int len = content.length() - 1;
			content.append("Arguments:\n");

			addStyleRange(len, content.length() - len - 1, SWT.BOLD);

			len = content.length() - 1;
			for(String arg : macro_.getArguments())
			{
				content.append('\t' + arg + '\n');
			}
			addStyleRange(len, content.length() - len - 1, SWT.ITALIC);
		}

		contents_ = content.toString();
	}

	/**
	 * Adds a style range to current list
	 * @param offset
	 * @param length
	 * @param style
	 */
	private void addStyleRange(int offset, int length, int style)
	{
		styleRanges_.add(new StyleRange(offset, length,
				Display.getDefault().getSystemColor(SWT.COLOR_INFO_FOREGROUND),
				Display.getDefault().getSystemColor(SWT.COLOR_INFO_BACKGROUND),
				style));
	}

	public String getTitle()
	{
		generateDoc();
		return title_;
	}

	public String getContents()
	{
		generateDoc();
		return contents_;
	}

	public StyleRange[] getStyleRanges()
	{
		generateDoc();
		return styleRanges_.toArray(new StyleRange[styleRanges_.size()]);
	}

	/**
	 * Gets the associated macro
	 * @return
	 */
	public Define getMacro()
	{
		return macro_;
	}

	public String getInfoText()
	{
		return "";
	}
}
