/**
 * @author Timotei Dolean
 *
 */
package org.wesnoth.ui;

import org.eclipse.xtext.resource.XtextResource;
import org.eclipse.xtext.ui.editor.XtextEditor;
import org.eclipse.xtext.ui.editor.XtextSourceViewer;
import org.eclipse.xtext.ui.editor.syntaxcoloring.HighlightingPresenter;
import org.eclipse.xtext.ui.editor.syntaxcoloring.HighlightingReconciler;

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
