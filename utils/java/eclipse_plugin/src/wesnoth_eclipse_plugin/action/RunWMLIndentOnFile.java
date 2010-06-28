/**
 * @author Timotei Dolean
 */
package wesnoth_eclipse_plugin.action;

import org.eclipse.jface.action.IAction;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.ui.IObjectActionDelegate;
import org.eclipse.ui.IWorkbenchPart;

import wesnoth_eclipse_plugin.utils.WMLTools;
import wesnoth_eclipse_plugin.utils.WorkspaceUtils;

public class RunWMLIndentOnFile  implements IObjectActionDelegate
{

	public RunWMLIndentOnFile() { }

	@Override
	public void setActivePart(IAction action, IWorkbenchPart targetPart){
	}

	@Override
	public void run(IAction action)
	{
		WMLTools.runWMLIndent(WorkspaceUtils.getPathRelativeToUserDir(WorkspaceUtils.getSelectedFile()),
				true, true,false);
	}

	@Override
	public void selectionChanged(IAction action, ISelection selection){
	}
}
