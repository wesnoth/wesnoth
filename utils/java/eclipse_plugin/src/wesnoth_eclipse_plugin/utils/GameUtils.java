/**
 * @author Timotei Dolean
 *
 */
package wesnoth_eclipse_plugin.utils;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IResource;

import wesnoth_eclipse_plugin.Constants;
import wesnoth_eclipse_plugin.builder.ExternalToolInvoker;
import wesnoth_eclipse_plugin.preferences.Preferences;

public class GameUtils
{
	public static void runCampaignScenario()
	{
		if (WorkspaceUtils.getSelectedResource() == null)
		{
			GUIUtils.showMessageBox("Please select a campaign or a resource inside the " +
					"campaign project before.");
			return;
		}

		IResource selectedResource = WorkspaceUtils.getSelectedResource();

		//TODO: optimize this by checking if file really is a scenario
		if (selectedResource instanceof IFile &&
			!ProjectUtils.isScenarioFile(WorkspaceUtils.getPathRelativeToUserDir(selectedResource)))
		{
			GUIUtils.showMessageBox("This is not a valid scenario file.");
			return;
		}

		try
		{
			String campaignId = ProjectUtils.getCampaignID();
			String scenarioId = ProjectUtils.getScenarioID(
					WorkspaceUtils.getPathRelativeToUserDir(selectedResource));

			if (campaignId == null)
			{
				GUIUtils.showMessageBox("You need to have a valid _main.cfg campaign file" +
						" in your directory");
				return;
			}

			List<String> args = new ArrayList<String>();
			args.add("-c");
			args.add(campaignId);
			if (scenarioId != null)
				args.add(scenarioId);

			String wesnothExec = Preferences.getString(Constants.P_WESNOTH_EXEC_PATH);
			if (wesnothExec.isEmpty())
			{
				GUIUtils.showMessageBox("Please set the wesnoth's executable path first.");
				return;
			}

			String workingDir = Preferences.getString(Constants.P_WESNOTH_WORKING_DIR);

			args.add("--config-dir");
			// add the user's data directory path
			args.add(Preferences.getString(Constants.P_WESNOTH_USER_DIR));

			if (workingDir.isEmpty())
				workingDir = wesnothExec.substring(0,
						wesnothExec.lastIndexOf(new File(wesnothExec).getName()));

			// we need to add the working dir (backward compatibility)
			args.add(workingDir);

			System.out.printf("Launching args: %s \n", args);
			ExternalToolInvoker.launchTool(wesnothExec, args,
					Constants.TI_SHOW_OUTPUT | Constants.TI_SHOW_OUTPUT_USER, true);
		} catch (Exception e)
		{
			e.printStackTrace();
		}
	}
}
