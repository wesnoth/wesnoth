/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.utils;

import org.eclipse.core.resources.IFile;
import org.eclipse.jface.text.BadLocationException;
import org.eclipse.jface.text.IDocument;
import org.eclipse.jface.text.ITextSelection;
import org.eclipse.ui.IEditorPart;
import org.eclipse.ui.IWorkbenchPage;
import org.eclipse.ui.PartInitException;
import org.eclipse.ui.ide.IDE;
import org.eclipse.ui.texteditor.AbstractTextEditor;
import org.eclipse.ui.texteditor.IDocumentProvider;
import org.eclipse.ui.texteditor.ITextEditor;

import wesnoth_eclipse_plugin.Logger;

/**
 * An utils class that handles Eclipse's editor
 */
public class EditorUtils
{
	/**
	 * Writes the specified content in current opened editor
	 * @param content the string content to write
	 */
	public static void writeInEditor(String content)
	{
		writeInEditor(getEditedFile(), content);
	}

	/**
	 * Writes the specified content in the specified editor
	 * @param content the string content to write
	 */
	public static void writeInEditor(IEditorPart targetEditor, String content)
	{
		int offset = ((ITextSelection) getTextEditor(targetEditor).
						getSelectionProvider().getSelection()).getOffset();
		try
		{
			getEditorDocument(targetEditor).replace(offset, 0, content);
		} catch (BadLocationException e)
		{
		}
	}
	/**
	 * Replaces the text in current opened editor with the specified one
	 * @param content the string to replace the current content
	 */
	public static void replaceEditorText(String content)
	{
		replaceEditorText(getEditedFile(), content);
	}

	/**
	 * Replaces the text in the specified editor with the specified one
	 * @param content the string to replace the current content
	 */
	public static void replaceEditorText(IEditorPart targetEditor, String content)
	{
		if (targetEditor == null)
			return;
		try
		{
			getEditorDocument(targetEditor).
				replace(0, getEditorDocument(targetEditor).getLength(), content);
		} catch (BadLocationException e)
		{
		}
	}

	/**
	 * Gets the current opened editor's document
	 * @return
	 */
	public static IDocument getEditorDocument()
	{
		return getEditorDocument(getEditedFile());
	}

	/**
	 * Gets the specified editor's document
	 * @return
	 */
	public static IDocument getEditorDocument(IEditorPart targetEditor)
	{
		if (targetEditor == null)
			return null;

		IDocumentProvider dp = getTextEditor(targetEditor).getDocumentProvider();
		return dp.getDocument(targetEditor.getEditorInput());
	}

	/**
	 * Gets the text editor of the current opened editor
	 * @return
	 */
	public static ITextEditor getTextEditor()
	{
		return getTextEditor(getEditedFile());
	}

	/**
	 * Gets the text editor of the specified editor
	 * @return
	 */
	public static ITextEditor getTextEditor(IEditorPart targetEditor)
	{
		if (targetEditor == null)
			return null;

		IEditorPart part = targetEditor;
		if (!(part instanceof AbstractTextEditor))
			return null;
		return (ITextEditor) part;
	}

	/**
	 * Gets the editor part of the current edited file
	 * @return
	 */
	public static IEditorPart getEditedFile()
	{
		return WorkspaceUtils.getWorkbenchWindow().getPages()[0].getActiveEditor();
	}

	/**
	 * Opens the editor on the specified file
	 * @param file The file to open
	 * @param activatePage True to activate the opened file
	 */
	public static void openEditor(IFile file, boolean activatePage)
	{
		IWorkbenchPage page = WorkspaceUtils.getWorkbenchWindow().getActivePage();
		try
		{
			IDE.openEditor(page, file, activatePage);
		} catch (PartInitException e)
		{
			Logger.getInstance().logException(e);
		}
	}
}
