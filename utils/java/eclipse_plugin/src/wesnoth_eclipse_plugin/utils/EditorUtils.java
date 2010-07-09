/**
 * @author Timotei Dolean
 *
 */
package wesnoth_eclipse_plugin.utils;

import org.eclipse.core.resources.IFile;
import org.eclipse.jface.text.BadLocationException;
import org.eclipse.jface.text.IDocument;
import org.eclipse.jface.text.ITextSelection;
import org.eclipse.ui.IEditorPart;
import org.eclipse.ui.IWorkbenchPage;
import org.eclipse.ui.PartInitException;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.ide.IDE;
import org.eclipse.ui.texteditor.AbstractTextEditor;
import org.eclipse.ui.texteditor.IDocumentProvider;
import org.eclipse.ui.texteditor.ITextEditor;

import wesnoth_eclipse_plugin.Activator;

public class EditorUtils
{
	public static void writeInEditor(String content)
	{
		writeInEditor(getEditedFile(), content);
	}

	public static void replaceEditorText(String content)
	{
		replaceEditorText(getEditedFile(), content);
	}

	public static IDocument getEditorDocument()
	{
		return getEditorDocument(getEditedFile());
	}

	public static ITextEditor getTextEditor()
	{
		return getTextEditor(getEditedFile());
	}

	public static IEditorPart getEditedFile()
	{
		return Activator.getDefault().getWorkbench().getActiveWorkbenchWindow().getPages()[0].getActiveEditor();
	}

	public static void openEditor(IFile file, boolean activatePage)
	{
		IWorkbenchPage page = PlatformUI.getWorkbench().getActiveWorkbenchWindow().getActivePage();
		try
		{
			IDE.openEditor(page, file, activatePage);
		} catch (PartInitException e)
		{
		}
	}

	public static IDocument getEditorDocument(IEditorPart targetEditor)
	{
		IDocumentProvider dp = getTextEditor(targetEditor).getDocumentProvider();
		return dp.getDocument(targetEditor.getEditorInput());
	}

	public static ITextEditor getTextEditor(IEditorPart targetEditor)
	{
		if (targetEditor == null)
			return null;

		IEditorPart part = targetEditor;
		if (!(part instanceof AbstractTextEditor))
			return null;
		return (ITextEditor) part;
	}

	public static void writeInEditor(IEditorPart targetEditor, String content)
	{

		int offset = ((ITextSelection) getTextEditor(targetEditor).getSelectionProvider().getSelection()).getOffset();
		try
		{
			getEditorDocument(targetEditor).replace(offset, 0, content);
		} catch (BadLocationException e)
		{
		}
	}

	public static void replaceEditorText(IEditorPart targetEditor, String content)
	{
		if (targetEditor == null)
			return;
		try
		{
			getEditorDocument(targetEditor).replace(0, getEditorDocument(targetEditor).getLength(), content);
		} catch (BadLocationException e)
		{
		}
	}
}
