/**
 * @author Timotei Dolean
 *
 */
package org.wesnoth.ui;

import com.google.inject.Inject;

import org.eclipse.xtext.resource.XtextResource;
import org.eclipse.xtext.ui.editor.syntaxcoloring.HighlightingReconciler;
import org.eclipse.xtext.ui.editor.syntaxcoloring.IHighlightedPositionAcceptor;
import org.eclipse.xtext.ui.editor.syntaxcoloring.ISemanticHighlightingCalculator;
import org.eclipse.xtext.ui.editor.syntaxcoloring.MergingHighlightedPositionAcceptor;

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
