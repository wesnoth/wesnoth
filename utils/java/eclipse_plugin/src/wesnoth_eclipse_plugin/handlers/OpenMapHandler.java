package wesnoth_eclipse_plugin.handlers;

import org.eclipse.core.commands.AbstractHandler;
import org.eclipse.core.commands.ExecutionEvent;
import org.eclipse.core.commands.ExecutionException;
import org.eclipse.core.resources.IFile;
import org.eclipse.swt.widgets.MessageBox;

import wesnoth_eclipse_plugin.Activator;
import wesnoth_eclipse_plugin.globalactions.EditorActions;
import wesnoth_eclipse_plugin.utils.WorkspaceUtils;

public class OpenMapHandler extends AbstractHandler
{
	@Override
	public Object execute(ExecutionEvent event) throws ExecutionException
	{
		IFile selectedFile = WorkspaceUtils.getSelectedFile(WorkspaceUtils.getWorkbenchWindow());
		if (!selectedFile.getName().endsWith(".map"))
		{
			MessageBox box = new MessageBox(Activator.getShell());
			box.setMessage("Please select a .map file");
			box.open();
			return null;
		}
		EditorActions.startEditor(selectedFile.getLocation().toOSString());
		return null;
	}
}
