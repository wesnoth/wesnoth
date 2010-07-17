/*******************************************************************************
 * Copyright (c) 2009 itemis AG (http://www.itemis.eu) and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.syntax;

import org.wesnoth.ui.WMLEditor;

import org.eclipse.xtext.ui.editor.XtextEditor;
import org.eclipse.xtext.ui.editor.XtextSourceViewer;
import org.eclipse.xtext.ui.editor.syntaxcoloring.HighlightingHelper;

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
