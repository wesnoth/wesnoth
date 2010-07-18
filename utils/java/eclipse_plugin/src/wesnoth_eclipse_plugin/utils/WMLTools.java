/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.utils;

import java.io.BufferedWriter;
import java.io.File;
import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.util.ArrayList;
import java.util.List;

import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.WorkspaceJob;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.runtime.IStatus;
import org.eclipse.core.runtime.Path;
import org.eclipse.core.runtime.Status;
import org.eclipse.ui.IEditorReference;
import org.eclipse.ui.console.MessageConsole;

import wesnoth_eclipse_plugin.Constants;
import wesnoth_eclipse_plugin.Logger;
import wesnoth_eclipse_plugin.preferences.Preferences;

public class WMLTools
{
	/**
	 * Runs "wmlindent" on the specified resource (directory/file)
	 *
	 * @param resourcePath the full path of the target where "wmlindent" will be runned on
	 * @param writeToConsole true to write the output of "wmlindent" in user's console
	 * @param stdin the standard input string to feed "wmlindent"
	 * @param dryrun true to run "wmlindent" in dry mode - i.e. no changes in the config file.
	 * @param useThread whether the tool should be runned in a new thread or not
	 */
	public static ExternalToolInvoker runWMLIndent(String resourcePath, String stdin,
			boolean dryrun, OutputStream[] stdout, OutputStream[] stderr)
	{
		if (!checkPrerequisites(null, "wmlindent")) // wmlindent only check first
			return null;

		File wmllintFile = new File(Preferences.getString(Constants.P_WESNOTH_WMLTOOLS_DIR) + "/wmlindent");
		List<String> arguments = new ArrayList<String>();
		arguments.add(wmllintFile.getAbsolutePath());

		if (resourcePath != null)
		{
			if (!checkPrerequisites(resourcePath, null))
				return null;

			if (dryrun)
				arguments.add("--dryrun");
			arguments.add("--verbose");
			arguments.add(resourcePath);
		}
		return runPythonScript(arguments, stdin,stdout,stderr);
	}

	/**
	 * Runs "wmllint" on the specified resource (directory/file)
	 *
	 * @param resourcePath the full path of the target where "wmllint" will be runned on
	 * @param writeToConsole true to write the output of "wmllint" in user's console
	 * @param dryrun true to run "wmllint" in dry mode - i.e. no changes in the config file.
	 * @param useThread whether the tool should be runned in a new thread or not
	 */
	public static ExternalToolInvoker runWMLLint(String resourcePath, boolean dryrun,
				OutputStream[] stdout, OutputStream[] stderr)
	{
		if (!checkPrerequisites(resourcePath, "wmllint"))
			return null;

		File wmllintFile = new File(Preferences.getString(Constants.P_WESNOTH_WMLTOOLS_DIR) + "wmllint");

		List<String> arguments = new ArrayList<String>();

		arguments.add(wmllintFile.getAbsolutePath());
		if (dryrun)
			arguments.add("--dryrun");
		arguments.add("--verbose");
		//arguments.add("-v");
		//arguments.add("-v");
		arguments.add("--nospellcheck");
		// add default core directory
		arguments.add(Preferences.getString(Constants.P_WESNOTH_WORKING_DIR) +
				Path.SEPARATOR + "data/core");
		arguments.add(resourcePath);

		return runPythonScript(arguments, null, stdout,stderr);
	}

	/**
	 * Runs "wmlscope" on the specified resource (directory/file)
	 *
	 * @param resourcePath the full path of the target where "wmlindent" will be runned on
	 * @param writeToConsole true to write the output of "wmlindent" in user's console
	 * @param useThread whether the tool should be runned in a new thread or not
	 * @return
	 */
	public static ExternalToolInvoker runWMLScope(String resourcePath,
			OutputStream[] stdout, OutputStream[] stderr)
	{
		if (!checkPrerequisites(resourcePath, "wmlscope"))
			return null;

		File wmlscopeFile = new File(Preferences.getString(Constants.P_WESNOTH_WMLTOOLS_DIR) + "/wmlscope");

		List<String> arguments = new ArrayList<String>();

		arguments.add(wmlscopeFile.getAbsolutePath());
		arguments.add("-w");
		arguments.add("2");

		// add default core directory
		arguments.add(Preferences.getString(Constants.P_WESNOTH_WORKING_DIR) +
				Path.SEPARATOR + "data/core");
		arguments.add(resourcePath);

		return runPythonScript(arguments, null, stdout, stderr);
	}

	public static void runWMLToolAsWorkspaceJob(final Tools tool, final String targetPath)
	{
		if (tool == Tools.WESNOTH_ADDON_MANAGER)
			return;

		IEditorReference[] files = WorkspaceUtils.getWorkbenchWindow().getPages()[0].getEditorReferences();
		//TODO: do more checks and see if the files really need to be saved
		// maybe they are out of current scope

		for (IEditorReference file : files)
		{
			if (file.isDirty())
				file.getEditor(false).doSave(null);
		}

		if (targetPath == null && WorkspaceUtils.getSelectedFile() != null)
			EditorUtils.openEditor(WorkspaceUtils.getSelectedFile(), false);
		final String toolName = tool.toString();

		WorkspaceJob job = new WorkspaceJob("Running " + toolName) {
			private ExternalToolInvoker toolInvoker;
			@Override
			protected void canceling()
			{
				toolInvoker.kill(true);
				super.canceling();
			}

			@Override
			public IStatus runInWorkspace(final IProgressMonitor monitor)
			{
				try{
					monitor.beginTask(toolName, 50);
					monitor.beginTask(tool.toString(), 50);
					MessageConsole console = GUIUtils.createConsole(toolName + " result:", null, true);
					OutputStream messageStream = console.newMessageStream();
					//TODO: multiple streams? - check performance
					OutputStream[] stream = new OutputStream[]{ messageStream};

					String location;
					String stdin = EditorUtils.getEditorDocument().get();

					IFile selFile = WorkspaceUtils.getSelectedFile();
					if (targetPath != null)
						location = targetPath;
					else
					{
						if (selFile != null)
							location = selFile.getLocation().toOSString();
						else //TODO: add container instead of project?
							location = WorkspaceUtils.getSelectedProject().getLocation().toOSString();
					}

					switch(tool)
					{
						case WMLINDENT:
							if (selFile != null && targetPath == null)
								toolInvoker = WMLTools.runWMLIndent(null, stdin, false,
										null, stream);
							else
								toolInvoker = WMLTools.runWMLIndent(location, null, false,
										stream, stream);
							break;
						case WMLLINT:
							toolInvoker = WMLTools.runWMLLint(location, true, stream, stream);
							break;
						case WMLSCOPE:
							toolInvoker = WMLTools.runWMLScope(location, stream, stream);
							break;
					}
					toolInvoker.waitForTool();
					if (selFile != null && targetPath == null)
					{
						EditorUtils.replaceEditorText(toolInvoker.getOutputContent());
					}
					monitor.worked(50);
					monitor.done();
				}
				catch(Exception e)
				{
					Logger.getInstance().logException(e);
				}
				return Status.OK_STATUS;
			}
		};
		job.schedule();
	}


	public static ExternalToolInvoker runWesnothAddonManager(String containerPath,
			OutputStream[] stdout, OutputStream[] stderr)
	{
		if (!checkPrerequisites(containerPath, "wesnoth_addon_manager"))
			return null;

		File wmllintFile = new File(Preferences.getString(Constants.P_WESNOTH_WMLTOOLS_DIR)
				+ "/wesnoth_addon_manager");
		List<String> arguments = new ArrayList<String>();
		arguments.add(wmllintFile.getAbsolutePath());

		arguments.add("-u");
		arguments.add(containerPath);
		return runPythonScript(arguments, null, stdout,stderr);
	}

	/**
	 * Checks if a wmlTool (that is in the wml tools directory) and
	 * an additional file that is target of the tool exist / are valid.
	 *
	 * @param filePath the file to be processed by the wml tool
	 * @param wmlTool the wml tool file
	 * @return
	 */
	public static boolean checkPrerequisites(String filePath, String wmlTool)
	{
		if (wmlTool != null)
		{
			if (Preferences.getString(Constants.P_WESNOTH_WMLTOOLS_DIR).equals(""))
			{
				GUIUtils.showWarnMessageBox("Please set the wmltools directory in the " +
						"preferences before you use this feature.");
				return false;
			}
			File wmlToolFile = new File(Preferences.getString(Constants.P_WESNOTH_WMLTOOLS_DIR) +
					Path.SEPARATOR + wmlTool);

			if (!wmlToolFile.exists())
			{
				GUIUtils.showErrorMessageBox(String.format("The file %s was not found",
						wmlToolFile));
				return false;
			}
		}
		if (filePath != null && (filePath.isEmpty() || !new File(filePath).exists()))
		{
			GUIUtils.showErrorMessageBox("The file does not exist or is null");
			return false;
		}

		return true;
	}

	/**
	 * Runs a specified python script with the specified arguments
	 * (the call returns immediately)
	 *
	 * @param arguments the arguments of the "python" executable.
	 *        The first argument should be the script file name
	 * @param stdin A string that will be written to stdin of the python script
	 * @param stdout An array of writers where to write the stderr from the script
	 * @param stderr An array of writers where to write the stderr from the script
	 * @return
	 */
	public static ExternalToolInvoker runPythonScript(List<String> arguments, String stdin,
			final OutputStream[] stdout, final OutputStream[] stderr)
	{
		final ExternalToolInvoker pyscript = new ExternalToolInvoker("python", arguments);

		pyscript.runTool();
		pyscript.startErrorMonitor();
		pyscript.startOutputMonitor();
		if (stdin != null)
		{
			try
			{
				BufferedWriter stdinStream = new BufferedWriter(new OutputStreamWriter(pyscript.getStdin()));
				stdinStream.write(stdin);
				stdinStream.close();
			}
			catch (IOException e) {
				Logger.getInstance().logException(e);
			}
		}

		Thread outputThread = new Thread(new Runnable() {
			@Override
			public void run()
			{
				try{
					String line = "";
					while ((line = pyscript.readOutputLine()) != null)
					{
						if (stdout != null)
						{
							for(OutputStream stream : stdout)
								stream.write((line + "\n").getBytes());
						}
					}
				}
				catch (IOException e) {
					Logger.getInstance().logException(e);
				}
			}
		});
		Thread errorThread = new Thread(new Runnable() {
			@Override
			public void run()
			{
				try
				{
					String line = "";
					while ((line = pyscript.readErrorLine()) != null)
					{
						if (stderr != null)
						{
							for(OutputStream stream : stderr)
								stream.write((line + "\n").getBytes());
						}
					}
				} catch (IOException e)
				{
					Logger.getInstance().logException(e);
				}
			}
		});
		outputThread.start();
		errorThread.start();
		return pyscript;
	}


	public enum Tools
	{
		WMLLINT, WMLINDENT, WMLSCOPE, WESNOTH_ADDON_MANAGER;
	}
}
