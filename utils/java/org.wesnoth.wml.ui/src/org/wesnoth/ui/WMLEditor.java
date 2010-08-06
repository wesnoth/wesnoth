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
import org.eclipse.ui.IEditorInput;
import org.eclipse.ui.IEditorSite;
import org.eclipse.ui.IFileEditorInput;
import org.eclipse.ui.IURIEditorInput;
import org.eclipse.ui.PartInitException;
import org.eclipse.xtext.parsetree.LeafNode;
import org.eclipse.xtext.ui.editor.XtextEditor;
import org.eclipse.xtext.ui.editor.syntaxcoloring.IHighlightingHelper;
import org.wesnoth.ui.syntax.WMLHighlightingHelper;
import org.wesnoth.ui.xtext.EFSEditorInput;

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
	}

	/**
	 * Translates an incoming IEditorInput being an FilestoreEditorInput, or IURIEditorInput
	 * that is not also a IFileEditorInput.
	 * FilestoreEditorInput is used when opening external files in an IDE environment.
	 * The result is that the regular XtextEditor gets an IEFSEditorInput which is also an
	 * IStorageEditorInput.
	 */
	@Override
	public void init(IEditorSite site, IEditorInput input) throws PartInitException {
		// THE ISSUE HERE:
		// In the IDE, the File Open Dialog (and elsewhere) uses a FilestoreEditorInput class
		// which is an IDE specific implementation.
		// The state at this point:
		// 1. When creating a file, the IEditorInput is an IURIEditorInput
		// 2. The only (non IDE specific) interface implemented by FilestoreEditorInput is IURIEditorInput
		// 3. The creation of a file is however also an IFileEditorInput
		//
		// Remedy:
		// 1. Create an IEditorInput that can be used by the rest of Xtext
		// 2. If it is a file based URI select an impl that can handle both reading, writing and annotations
		// even if not in the workbench
		// 3. If it is a non file: uri, provide an impl that at least opens the file
		// 3.1 The EMF URIEditorInputImpl does not support writing, and does not automatically provide
		// markers.
		// 4. The XtextDocumentProvider must be specialized to handle the specialized IEditorInput
		// if it can not be made to look like one of the others (too much may be assumed)
		//
		if(input instanceof IURIEditorInput && !(input instanceof IFileEditorInput)) {
			super.init(site, new EFSEditorInput(((IURIEditorInput) input).getURI(), input.getName()));
			return;
		}
		super.init(site, input);
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
