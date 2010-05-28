package wesnoth_eclipse_plugin.action;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import org.eclipse.core.resources.IFile;
import org.eclipse.jface.action.IAction;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.ui.IObjectActionDelegate;
import org.eclipse.ui.IWorkbenchPart;

import wesnoth_eclipse_plugin.builder.ExternalToolInvoker;
import wesnoth_eclipse_plugin.preferences.PreferenceConstants;
import wesnoth_eclipse_plugin.preferences.PreferenceInitializer;
import wesnoth_eclipse_plugin.utils.GUIUtils;
import wesnoth_eclipse_plugin.utils.ProjectUtils;
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
		if (WorkspaceUtils.getSelectedProject() == null &&
				WorkspaceUtils.getSelectedFile() == null &&
				WorkspaceUtils.getSelectedFolder() == null)
		{
			GUIUtils.showMessageBox(WorkspaceUtils.getWorkbenchWindow(),
			"Please select a campaign or a resource inside the campaign project before.");
			return;
		}

		IFile selectedFile = WorkspaceUtils.getSelectedFile(WorkspaceUtils.getWorkbenchWindow());
		if (selectedFile == null)
			return;

		//TODO: optimize this by checking if file really is a scenario (PersistentProperties?)
		if (!ProjectUtils.isScenarioFile(WorkspaceUtils.getPathRelativeToUserDir(selectedFile)))
		{
			GUIUtils.showMessageBox(WorkspaceUtils.getWorkbenchWindow(), "This is not a scenario file.");
			return;
		}

		try{
			String campaignId = ProjectUtils.getCampaignID();
			String scenarioId = ProjectUtils.getScenarioID(WorkspaceUtils.getPathRelativeToUserDir(selectedFile));

			List<String> args = new ArrayList<String>();
			args.add("-c");
			args.add(campaignId);
			args.add(scenarioId);

			String wesnothExec = PreferenceInitializer.getString(PreferenceConstants.P_WESNOTH_EXEC_PATH);
			if (wesnothExec.isEmpty())
			{
				GUIUtils.showMessageBox(WorkspaceUtils.getWorkbenchWindow(), "Please set the wesnoth's executable path first.");
				return;
			}

			String workingDir = PreferenceInitializer.getString(PreferenceConstants.P_WESNOTH_WORKING_DIR);

			if (workingDir.isEmpty())
				workingDir = wesnothExec.substring(0,wesnothExec.lastIndexOf(new File(wesnothExec).getName()));

			// we need to add the working dir (backward compatibility)
			args.add(workingDir);
			System.out.printf("Launching args: %s \n", args);
			ExternalToolInvoker.launchTool(wesnothExec,	args, true, false,true, WorkspaceUtils.getWorkbenchWindow());
		}
		catch (Exception e) {
			e.printStackTrace();
		}
	}

	@Override
	public void selectionChanged(IAction action, ISelection selection){
	}
}
