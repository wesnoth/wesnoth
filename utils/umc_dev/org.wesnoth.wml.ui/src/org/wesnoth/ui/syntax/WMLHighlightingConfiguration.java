/*******************************************************************************
 * Copyright (c) 2010 - 2011 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.syntax;

import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.RGB;
import org.eclipse.xtext.ui.editor.syntaxcoloring.DefaultHighlightingConfiguration;
import org.eclipse.xtext.ui.editor.syntaxcoloring.IHighlightingConfigurationAcceptor;
import org.eclipse.xtext.ui.editor.utils.TextStyle;
import org.wesnoth.ui.Messages;

public class WMLHighlightingConfiguration extends DefaultHighlightingConfiguration
{
	public static final String RULE_WML_TAG = "wmlTag"; //$NON-NLS-1$
	public static final String RULE_WML_KEY = "wmlKey"; //$NON-NLS-1$
	public static final String RULE_WML_MACRO_CALL = "wmlMacroCall"; //$NON-NLS-1$

	public static final String RULE_WML_MACRO_DEFINE = "wmlMacroDefine"; //$NON-NLS-1$
	public static final String RULE_WML_IF = "wmlIF"; //$NON-NLS-1$
	public static final String RULE_WML_TEXTDOMAIN = "wmlTextdomain"; //$NON-NLS-1$

	public static final String RULE_START_END_TAG = "wmlStartEnd"; //$NON-NLS-1$

	@Override
	public void configure(IHighlightingConfigurationAcceptor acceptor)
	{
		super.configure(acceptor);
		acceptor.acceptDefaultHighlighting(RULE_WML_TAG, Messages.WMLHighlightingConfiguration_7, tagTextStyle());
		acceptor.acceptDefaultHighlighting(RULE_WML_KEY, Messages.WMLHighlightingConfiguration_8, keyTextStyle());
		acceptor.acceptDefaultHighlighting(RULE_WML_MACRO_CALL, Messages.WMLHighlightingConfiguration_9, macroTextStyle());

		acceptor.acceptDefaultHighlighting(RULE_START_END_TAG,
				Messages.WMLHighlightingConfiguration_10, startEndTextStyle());

		// preproc
		acceptor.acceptDefaultHighlighting(RULE_WML_MACRO_DEFINE, Messages.WMLHighlightingConfiguration_11, preprocTextStyle());
		acceptor.acceptDefaultHighlighting(RULE_WML_IF, Messages.WMLHighlightingConfiguration_12, preprocTextStyle());
		acceptor.acceptDefaultHighlighting(RULE_WML_TEXTDOMAIN, Messages.WMLHighlightingConfiguration_13, preprocTextStyle());
	}

	public TextStyle preprocTextStyle()
	{
		TextStyle textStyle = defaultTextStyle().copy();
		textStyle.setColor(new RGB(31, 209, 241));
		textStyle.setStyle(SWT.BOLD);
		return textStyle;
	}

	public TextStyle macroTextStyle()
	{
		TextStyle textStyle = defaultTextStyle().copy();
		textStyle.setColor(new RGB(197, 137, 23));
		textStyle.setStyle(SWT.ITALIC);
		return textStyle;
	}

	public TextStyle startEndTextStyle()
	{
		TextStyle textStyle = defaultTextStyle().copy();
		textStyle.setColor(new RGB(128, 128, 128));
		textStyle.setBackgroundColor(new RGB(128, 0, 0));
		textStyle.setStyle(SWT.BOLD | SWT.ITALIC);
		return textStyle;
	}

	public TextStyle tagTextStyle()
	{
		TextStyle textStyle = defaultTextStyle().copy();
		textStyle.setColor(new RGB(128, 0, 0));
		textStyle.setStyle(SWT.BOLD);
		return textStyle;
	}

	public TextStyle keyTextStyle()
	{
		TextStyle textStyle = defaultTextStyle().copy();
		textStyle.setColor(new RGB(0, 128, 128));
		textStyle.setStyle(SWT.BOLD);
		return textStyle;
	}
}
