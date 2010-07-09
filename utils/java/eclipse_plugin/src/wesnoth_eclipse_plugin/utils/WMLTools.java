/**
 * @author Timotei Dolean
 */
package wesnoth_eclipse_plugin.utils;

import java.io.BufferedWriter;
import java.io.File;
import java.io.OutputStreamWriter;
import java.util.ArrayList;
import java.util.List;

import org.eclipse.core.runtime.Path;
import org.eclipse.ui.console.ConsolePlugin;
import org.eclipse.ui.console.IConsole;
import org.eclipse.ui.console.MessageConsole;
import org.eclipse.ui.console.MessageConsoleStream;

import wesnoth_eclipse_plugin.Constants;
import wesnoth_eclipse_plugin.builder.ExternalToolInvoker;
import wesnoth_eclipse_plugin.preferences.Preferences;

public class WMLTools
{
	/**
	 * Runs "wmllint" on the specified resource (directory/file)
	 * 
	 * @param resourcePath the full path of the target where "wmllint" will be runned on
	 * @param writeToConsole true to write the output of "wmllint" in user's console
	 * @param dryrun true to run "wmllint" in dry mode - i.e. no changes in the config file.
	 * @param useThread whether the tool should be runned in a new thread or not
	 */
	public static String runWMLLint(String resourcePath, boolean dryrun, boolean writeToConsole, boolean useThread)
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

		return runPythonScript(arguments, null, useThread, writeToConsole, "Wmllint result: ");
	}

	/**
	 * Runs "wmlindent" on the specified resource (directory/file)
	 * 
	 * @param resourcePath the full path of the target where "wmlindent" will be runned on
	 * @param writeToConsole true to write the output of "wmlindent" in user's console
	 * @param stdin the standard input string to feed "wmlindent"
	 * @param dryrun true to run "wmlindent" in dry mode - i.e. no changes in the config file.
	 * @param useThread whether the tool should be runned in a new thread or not
	 */
	public static String runWMLIndent(String resourcePath, String stdin,
						boolean dryrun, boolean writeToConsole, boolean useThread)
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
		return runPythonScript(arguments, stdin, useThread, writeToConsole, "Wmlindent result: ");
	}

	/**
	 * Runs "wmlscope" on the specified resource (directory/file)
	 * 
	 * @param resourcePath the full path of the target where "wmlindent" will be runned on
	 * @param writeToConsole true to write the output of "wmlindent" in user's console
	 * @param useThread whether the tool should be runned in a new thread or not
	 * @return
	 */
	public static String runWMLScope(String resourcePath, boolean writeToConsole, boolean useThread)
	{
		if (!checkPrerequisites(resourcePath, "wmlscope"))
			return null;

		File wmlscopeFile = new File(Preferences.getString(Constants.P_WESNOTH_WMLTOOLS_DIR) + "/wmlscope");

		List<String> arguments = new ArrayList<String>();

		arguments.add(wmlscopeFile.getAbsolutePath());
		// add default core directory
		arguments.add(Preferences.getString(Constants.P_WESNOTH_WORKING_DIR) +
				Path.SEPARATOR + "data/core");
		arguments.add(resourcePath);

		return runPythonScript(arguments, null, useThread, writeToConsole, "Wmlscope result: ");
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
			File wmlToolFile = new File(Preferences.getString(Constants.P_WESNOTH_WMLTOOLS_DIR) +
					Path.SEPARATOR + wmlTool);

			if (!wmlToolFile.exists())
			{
				GUIUtils.showMessageBox(WorkspaceUtils.getWorkbenchWindow(),
						"Please set the wmltools directory in the preferences before you use this feature.");
				return false;
			}
		}
		if (filePath != null &&
				(filePath.isEmpty() || !new File(filePath).exists()))
			return false;

		return true;
	}

	/**
	 * Runs a specified python script with the specified arguments
	 * 
	 * @param arguments the arguments of the "python" executable.
	 *        The first argument should be the script file name
	 * @param useThread
	 * @param writeToConsole true to write the script output to user's console
	 * @return
	 */
	public static String runPythonScript(List<String> arguments, String stdin, boolean useThread,
			boolean writeToConsole, String consoleTitle)
	{
		String result = "";
		try
		{
			ExternalToolInvoker pyscript = new ExternalToolInvoker("python", arguments, useThread);
			System.out.println(arguments);
			pyscript.run();
			if (stdin != null)
			{
				//pyscript.waitForThreadStart();
				BufferedWriter stdinStream = new BufferedWriter(new OutputStreamWriter(pyscript.getOutputStream()));
				stdinStream.write(stdin);
				stdinStream.close();
			}

			MessageConsoleStream stream = null;
			if (writeToConsole)
			{
				MessageConsole console = new MessageConsole(consoleTitle, null);
				console.activate();
				ConsolePlugin.getDefault().getConsoleManager().addConsoles(new IConsole[] { console });
				stream = console.newMessageStream();
			}

			String line = "";
			while ((line = pyscript.readOutputLine()) != null)
			{
				if (writeToConsole)
					stream.write(line + "\n");
				result += (line + "\n");
			}
			while ((line = pyscript.readErrorLine()) != null)
			{
				if (writeToConsole)
					stream.write(line + "\n");
				result += (line + "\n");
			}
		} catch (Exception e)
		{
			GUIUtils.showMessageBox(e.getMessage());
			e.printStackTrace();
		}
		return result;
	}
}
