/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.syntax;

import org.eclipse.xtext.resource.XtextResource;
import org.eclipse.xtext.ui.editor.XtextEditor;
import org.eclipse.xtext.ui.editor.XtextSourceViewer;
import org.eclipse.xtext.ui.editor.syntaxcoloring.HighlightingPresenter;
import org.eclipse.xtext.ui.editor.syntaxcoloring.HighlightingReconciler;
import org.wesnoth.ui.WMLEditor;

public class WMLHighlightingReconciler extends HighlightingReconciler
{
	protected WMLEditor editor;

	@Override
	public void install(XtextEditor editor, XtextSourceViewer sourceViewer, HighlightingPresenter presenter)
	{
		super.install(editor, sourceViewer, presenter);
		this.editor = (WMLEditor) editor;
	}

	public WMLEditor getEditor()
	{
		return editor;
	}

	@Override
	public void modelChanged(XtextResource resource)
	{
		super.modelChanged(resource);
		// add our own acceptor
		WMLMergingHighlightedPositionAcceptor acceptor =
				new WMLMergingHighlightedPositionAcceptor(getCalculator());
		acceptor.provideHighlightingFor(resource, this);
	}
}
