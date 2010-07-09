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

public class RunWMLLintOnFile implements IObjectActionDelegate
{
	public RunWMLLintOnFile() {
	}

	@Override
	public void setActivePart(IAction action, IWorkbenchPart targetPart)
	{
	}

	@Override
	public void run(IAction action)
	{
		WMLTools.runWMLLint(WorkspaceUtils.getPathRelativeToUserDir(WorkspaceUtils.getSelectedFile()),
				true, true, true);
	}

	@Override
	public void selectionChanged(IAction action, ISelection selection)
	{
	}
}
