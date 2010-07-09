/**
 * @author Timotei Dolean
 */
package wesnoth_eclipse_plugin.handlers;

import org.eclipse.core.commands.AbstractHandler;
import org.eclipse.core.commands.ExecutionEvent;
import org.eclipse.core.commands.ExecutionException;
import org.eclipse.ui.IEditorReference;

import wesnoth_eclipse_plugin.Activator;

public class TestHandler extends AbstractHandler
{
	@Override
	public Object execute(ExecutionEvent event) throws ExecutionException
	{
		//String stdin = EditorUtils.getEditorDocument().get();
		//EditorUtils.replaceEditorText(WMLTools.runWMLIndent(null, stdin, false, false, false));
		IEditorReference[] files =
				Activator.getDefault().getWorkbench().getActiveWorkbenchWindow().getPages()[0].getEditorReferences();
		for (IEditorReference file : files)
		{
			if (file.isDirty())
				file.getEditor(false).doSave(null);
		}
		return null;
	}
}
