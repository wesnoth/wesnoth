/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.utils;

import java.io.File;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;

import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IResource;

import wesnoth_eclipse_plugin.Constants;
import wesnoth_eclipse_plugin.Logger;
import wesnoth_eclipse_plugin.preferences.Preferences;

public class GameUtils
{
	public static void runCampaign()
	{
		runCampaignScenario(false);
	}

	public static void runScenario()
	{
		runCampaignScenario(true);
	}

	protected static void runCampaignScenario(boolean scenario)
	{
		if (WorkspaceUtils.getSelectedResource() == null)
		{
			GUIUtils.showMessageBox("Please select a directory or a resource inside the " +
					"campaign project before.");
			return;
		}

		IResource selectedResource = WorkspaceUtils.getSelectedResource();

		//TODO: optimize this by checking if file really is a scenario
		if (scenario &&
			!ProjectUtils.isScenarioFile(WorkspaceUtils.getPathRelativeToUserDir(selectedResource)))
		{
			GUIUtils.showMessageBox("This is not a valid scenario file.");
			return;
		}

		try
		{
			String campaignId = ProjectUtils.getCampaignID(selectedResource);
			String scenarioId = ProjectUtils.getScenarioID(
					WorkspaceUtils.getPathRelativeToUserDir(selectedResource));

			if (campaignId == null)
			{
				GUIUtils.showMessageBox("You need to have a valid campaign file" +
						" in your directory (_main.cfg) or selected.");
				return;
			}

			List<String> args = new ArrayList<String>();
			args.add("-c");
			args.add(campaignId);
			if (scenarioId != null)
				args.add(scenarioId);
			startGame(args);
		} catch (Exception e)
		{
			Logger.getInstance().logException(e);
		}
	}


	public static void startGame(List<String> extraArgs)
	{
		List<String> args = new ArrayList<String>();
		String wesnothExec = Preferences.getString(Constants.P_WESNOTH_EXEC_PATH);
		if (wesnothExec.isEmpty())
		{
			GUIUtils.showMessageBox("Please set the wesnoth's executable path first.");
			return;
		}

		String workingDir = Preferences.getString(Constants.P_WESNOTH_WORKING_DIR);

		args.addAll(extraArgs);

		// add the user's data directory path
		args.add("--config-dir");
		args.add(Preferences.getString(Constants.P_WESNOTH_USER_DIR));

		if (workingDir.isEmpty())
			workingDir = wesnothExec.substring(0,
					wesnothExec.lastIndexOf(new File(wesnothExec).getName()));

		// we need to add the working dir (backward compatibility)
		args.add(workingDir);

		OutputStream[] stream = new OutputStream[] {
				GUIUtils.createConsole("Wesnoth game:", null, true).newMessageStream()
			};
		ExternalToolInvoker.launchTool(wesnothExec, args, stream, stream);
	}
	public static void startGame()
	{
		startGame(new ArrayList<String>());
	}

	public static void startEditor(IFile file)
	{
		if (file == null || !file.exists())
		{
			Logger.getInstance().log("non-existing map file",
					"Please select an existing map file before opening it.");
			return;
		}

		startEditor(file.getLocation().toOSString());
	}

	public static void startEditor(String mapName)
	{
		String editorPath = Preferences.getString(Constants.P_WESNOTH_EXEC_PATH);
		String workingDir = Preferences.getString(Constants.P_WESNOTH_WORKING_DIR);

		if (workingDir.isEmpty())
			workingDir = editorPath.substring(0, editorPath.lastIndexOf(new File(editorPath).getName()));

		if (editorPath.isEmpty())
		{
			Logger.getInstance().log("wesnoth executable not set (startEditor)",
					"Please set the wesnoth's executable path first.");
			return;
		}

		OutputStream[] stream = new OutputStream[] {
				GUIUtils.createConsole("Wesnoth editor:", null, true).newMessageStream()
			};
		ExternalToolInvoker.launchTool(editorPath, getEditorLaunchArguments(mapName, workingDir),
				stream, stream);
	}

	public static List<String> getEditorLaunchArguments(String mapName, String workingDir)
	{
		List<String> args = new ArrayList<String>(3);

		args.add("-e");
		args.add(mapName);

		if (!workingDir.isEmpty())
		{
			args.add("--data-dir");
			args.add(workingDir);
		}

		return args;
	}
}
