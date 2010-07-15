package wesnoth_eclipse_plugin.action;

import org.eclipse.jface.action.IAction;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.ui.IObjectActionDelegate;
import org.eclipse.ui.IWorkbenchPart;

import wesnoth_eclipse_plugin.utils.GameUtils;

public class OpenCampaignInGame implements IObjectActionDelegate
{
	public OpenCampaignInGame() {
	}

	@Override
	public void setActivePart(IAction action, IWorkbenchPart targetPart)
	{
	}

	@Override
	public void run(IAction action)
	{
		GameUtils.runCampaignScenario();
	}

	@Override
	public void selectionChanged(IAction action, ISelection selection)
	{
	}
}
