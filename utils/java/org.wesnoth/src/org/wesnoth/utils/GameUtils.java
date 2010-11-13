/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.utils;

import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;

import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IResource;
import org.eclipse.ui.console.MessageConsole;
import org.wesnoth.Constants;
import org.wesnoth.Logger;
import org.wesnoth.Messages;
import org.wesnoth.preferences.Preferences;


public class GameUtils
{
	/**
	 * Runs campaign from the selected project
	 */
	public static void runCampaign()
	{
		Thread gameThread = new Thread(new Runnable() {
			@Override
			public void run()
			{
				runCampaignScenario(false);
			}
		});
		gameThread.start();
	}

	/**
	 * Runs a scenario from the selected file
	 */
	public static void runScenario()
	{
		Thread gameThread = new Thread(new Runnable() {
			@Override
			public void run()
			{
				runCampaignScenario(true);
			}
		});
		gameThread.start();
	}

	protected static void runCampaignScenario(boolean scenario)
	{
		if (WorkspaceUtils.getSelectedResource() == null)
		{
			GUIUtils.showWarnMessageBox(Messages.GameUtils_0 +
					Messages.GameUtils_1);
			return;
		}

		IResource selectedResource = WorkspaceUtils.getSelectedResource();

		try
		{
			String campaignId = null;
			String scenarioId = null;

			campaignId = ResourceUtils.getCampaignID(selectedResource);

			if (scenario == true && selectedResource instanceof IFile)
				scenarioId = ResourceUtils.getScenarioID((IFile)selectedResource);

			if (campaignId == null)
			{
				GUIUtils.showErrorMessageBox(Messages.GameUtils_2 +
						Messages.GameUtils_3);
				return;
			}

			if (scenario == true && scenarioId == null)
			{
				GUIUtils.showErrorMessageBox(Messages.GameUtils_4 +
						Messages.GameUtils_5);
				return;
			}

			List<String> args = new ArrayList<String>();
			args.add("-c"); //$NON-NLS-1$
			args.add(campaignId);
			if (scenario == true)
				args.add(scenarioId);
			startGame(args);
		} catch (Exception e)
		{
			Logger.getInstance().logException(e);
		}
	}

	/**
	 * Starts the game
	 */
	public static void startGame()
	{
		startGame(null);
	}

	/**
	 * Starts the wesnoth game with the specified extraArguments
	 * @param extraArgs Extra arguments given to the game, or null.
	 */
	public static void startGame(List<String> extraArgs)
	{
		List<String> args = new ArrayList<String>();
		String wesnothExec = Preferences.getString(Constants.P_WESNOTH_EXEC_PATH);
		String workingDir = Preferences.getString(Constants.P_WESNOTH_WORKING_DIR);
		if (wesnothExec.isEmpty() || workingDir.isEmpty())
		{
			GUIUtils.showErrorMessageBox(Messages.GameUtils_7);
			return;
		}

		if (extraArgs != null)
			args.addAll(extraArgs);

		// add the user's data directory path
		args.add("--config-dir"); //$NON-NLS-1$
		args.add(Preferences.getString(Constants.P_WESNOTH_USER_DIR));

		// we need to add the working dir (backward compatibility)
		args.add(workingDir);

		MessageConsole console = GUIUtils.createConsole(Messages.GameUtils_9, null, true);
		ExternalToolInvoker.launchTool(wesnothExec, args,
				new OutputStream[] { console.newMessageStream() },
				new OutputStream[] { console.newMessageStream() });
	}

	/**
	 * Starts editor
	 */
	public static void startEditor()
	{
		startEditor(""); //$NON-NLS-1$
	}

	/**
	 * Starts the game editor on the specified file
	 * @param file The file to be edited
	 */
	public static void startEditor(IFile file)
	{
		if (file == null || !file.exists())
		{
			Logger.getInstance().log(Messages.GameUtils_11,
					Messages.GameUtils_12);
			return;
		}

		startEditor(file.getLocation().toOSString());
	}

	/**
	 * Starts the editor
	 * @param mapName
	 */
	public static void startEditor(String mapName)
	{
		startGame(getEditorLaunchArguments(mapName));
	}

	/**
	 * Gets a list of parameters for the game editor
	 * @param mapName the map to launch
	 * @return
	 */
	public static List<String> getEditorLaunchArguments(String mapName)
	{

		List<String> args = new ArrayList<String>(3);

		args.add("-e"); //$NON-NLS-1$
		if (mapName != null && !(mapName.isEmpty()))
			args.add(mapName);

		return args;
	}
}
