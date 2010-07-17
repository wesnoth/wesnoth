/**
 * @author Timotei Dolean
 *
 */
package org.wesnoth.ui;

import org.wesnoth.ui.syntax.WMLHighlightingHelper;

import org.eclipse.jface.text.source.ISourceViewer;
import org.eclipse.swt.custom.StyledText;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.xtext.parsetree.LeafNode;
import org.eclipse.xtext.ui.editor.XtextEditor;
import org.eclipse.xtext.ui.editor.syntaxcoloring.IHighlightingHelper;

public class WMLEditor extends XtextEditor
{
	protected IHighlightingHelper highlightingHelper_;
	protected LeafNode currentHighlightedNode_;

	@Override
	public void createPartControl(Composite parent)
	{
		super.createPartControl(parent);
		//		listener_ = new WMLSelectionChangeListener(DefaultUiModule.class.getTypeParameters());
		//		listener_.install(getSelectionProvider());
	}

	public StyledText getTextWidget()
	{
		return getSourceViewer().getTextWidget();
	}

	public ISourceViewer getSourceViewer_()
	{
		return getSourceViewer();
	}

	public WMLHighlightingHelper getHighlightingHelper()
	{
		return (WMLHighlightingHelper) highlightingHelper_;
	}

	public void setHighlightHelper(IHighlightingHelper helper)
	{
		highlightingHelper_ = helper;
	}

	public LeafNode getCurrentHighlightedNode()
	{
		return currentHighlightedNode_;
	}

	public void setCurrentHighlightedNode(LeafNode leaf)
	{
		currentHighlightedNode_ = leaf;
	}
}
