package wesnoth_eclipse_plugin.action;

import org.eclipse.core.resources.IFile;
import org.eclipse.jface.action.IAction;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.ui.IObjectActionDelegate;
import org.eclipse.ui.IWorkbenchPart;

import wesnoth_eclipse_plugin.utils.WorkspaceUtils;

public class OpenScenarioInGame implements IObjectActionDelegate
{
	public OpenScenarioInGame()
	{
	}

	@Override
	public void setActivePart(IAction action, IWorkbenchPart targetPart){
	}

	@Override
	public void run(IAction action)
	{
		IFile selectedFile = WorkspaceUtils.getSelectedFile(WorkspaceUtils.getWorkbenchWindow());
		//TODO: optimize this by checking if file really is a scenario (PersistentProperties)

	}

	@Override
	public void selectionChanged(IAction action, ISelection selection){
	}
}
