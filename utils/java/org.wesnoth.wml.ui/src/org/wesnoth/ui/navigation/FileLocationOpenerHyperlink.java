/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.navigation;

import org.eclipse.jface.text.BadLocationException;
import org.eclipse.jface.text.IDocument;
import org.eclipse.ui.IEditorPart;
import org.eclipse.ui.texteditor.ITextEditor;
import org.eclipse.xtext.ui.editor.hyperlinking.XtextHyperlink;

import wesnoth_eclipse_plugin.utils.EditorUtils;

/**
 * This is a hyperlink that opens a specified file
 * and puts the cursor to specified location
 */
public class FileLocationOpenerHyperlink extends XtextHyperlink
{
	private String filePath_;
	private int linenum_;

	@Override
	public void open()
	{
		IEditorPart part = EditorUtils.openEditor(filePath_);
		if (part != null)
		{

			ITextEditor editor = EditorUtils.getTextEditor(part);
			IDocument doc = editor.getDocumentProvider().
								getDocument(editor.getEditorInput());
			int offset = 0;
			try
			{
				// compute offset based on linenum_
				offset = doc.getLineOffset(linenum_);
			}
			catch (BadLocationException e) {
				//ignore
			}
			EditorUtils.getTextEditor(part).selectAndReveal(offset, 0);
		}
	}

	public void setLinenumber(int linenum)
	{
		linenum_ = linenum;
	}
	public int getLinenumber()
	{
		return linenum_;
	}
	public void setFilePath(String filePath)
	{
		filePath_ = filePath;
	}
	public String getFilePath()
	{
		return filePath_;
	}
}
