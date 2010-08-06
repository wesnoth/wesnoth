/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.syntax;

import org.eclipse.xtext.ui.editor.XtextEditor;
import org.eclipse.xtext.ui.editor.XtextSourceViewer;
import org.eclipse.xtext.ui.editor.syntaxcoloring.HighlightingHelper;
import org.wesnoth.ui.editor.WMLEditor;

/**
 * Highlighting helper.
 * Initially copied from org.eclipse.jdt.internal.ui.javaeditor.SemanticHighlightingManager
 *
 * @author Sebastian Zarnekow
 */
public class WMLHighlightingHelper extends HighlightingHelper
{
	@Override
	public void install(XtextEditor editor, XtextSourceViewer sourceViewer)
	{
		super.install(editor, sourceViewer);
		((WMLEditor) editor).setHighlightHelper(this);
	}
}
