package wesnoth_eclipse_plugin.action;

import org.eclipse.jface.action.IAction;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.ui.IObjectActionDelegate;
import org.eclipse.ui.IWorkbenchPart;

import wesnoth_eclipse_plugin.globalactions.MapActions;

public class ImportMap implements IObjectActionDelegate
{
	public ImportMap()
	{
	}

	/**
	 * This method is runned *ONLY* if the user selected a "maps" folder
	 */
	@Override
	public void run(IAction action)
	{
		MapActions.importMap();
	}

	@Override
	public void selectionChanged(IAction action, ISelection selection){
	}

	@Override
	public void setActivePart(IAction action, IWorkbenchPart targetPart) {
	}
}
