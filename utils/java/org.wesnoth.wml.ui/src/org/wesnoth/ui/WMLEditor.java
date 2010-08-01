/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui;

import org.apache.log4j.Level;
import org.eclipse.jface.text.source.ISourceViewer;
import org.eclipse.swt.custom.StyledText;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.xtext.parsetree.LeafNode;
import org.eclipse.xtext.ui.editor.XtextEditor;
import org.eclipse.xtext.ui.editor.syntaxcoloring.IHighlightingHelper;
import org.wesnoth.ui.syntax.WMLHighlightingHelper;

import wesnoth_eclipse_plugin.Activator;

public class WMLEditor extends XtextEditor
{
	protected IHighlightingHelper highlightingHelper_;
	protected LeafNode currentHighlightedNode_;

	public WMLEditor()
	{
		super();
		if (WMLUiModule.DEBUG)
			org.apache.log4j.Logger.getLogger(XtextEditor.class).setLevel(Level.DEBUG);
		// activate the wesnoth plugin
		Activator.getDefault();
	}

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
