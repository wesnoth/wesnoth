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
import org.eclipse.xtext.ui.editor.syntaxcoloring.HighlightingReconciler;
import org.eclipse.xtext.ui.editor.syntaxcoloring.IHighlightedPositionAcceptor;
import org.eclipse.xtext.ui.editor.syntaxcoloring.ISemanticHighlightingCalculator;
import org.eclipse.xtext.ui.editor.syntaxcoloring.MergingHighlightedPositionAcceptor;

import com.google.inject.Inject;

public class WMLMergingHighlightedPositionAcceptor extends MergingHighlightedPositionAcceptor
{
	protected HighlightingReconciler reconciler_;

	@Inject
	public WMLMergingHighlightedPositionAcceptor(ISemanticHighlightingCalculator delegate) {
		super(delegate);
	}

	@Override
	public void provideHighlightingFor(XtextResource resource, IHighlightedPositionAcceptor acceptor)
	{
		reconciler_ = (HighlightingReconciler) acceptor;
		super.provideHighlightingFor(resource, acceptor);
	}

	public HighlightingReconciler getHighlightingReconciler()
	{
		return reconciler_;
	}
}
