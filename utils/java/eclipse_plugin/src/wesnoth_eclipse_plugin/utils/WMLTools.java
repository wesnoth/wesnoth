/**
 * @author Timotei Dolean
 */
package wesnoth_eclipse_plugin.utils;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import org.eclipse.core.runtime.Path;
import org.eclipse.ui.console.ConsolePlugin;
import org.eclipse.ui.console.IConsole;
import org.eclipse.ui.console.MessageConsole;
import org.eclipse.ui.console.MessageConsoleStream;

import wesnoth_eclipse_plugin.builder.ExternalToolInvoker;
import wesnoth_eclipse_plugin.preferences.PreferenceConstants;
import wesnoth_eclipse_plugin.preferences.PreferenceInitializer;

public class WMLTools
{
	/**
	 * Runs "wmllint" on the specified file
	 * @param filePath the full path of the target file where "wmllint" will be runned on
	 * @param writeToConsole true to write the output of "wmllint" in console
	 * @param dryrun true to run "wmllint" in dry mode - i.e. no changes in the config file.
	 * @param useThread whether the tool should be runned in a new thread
	 */
	public static String runWMLLint(String filePath, boolean dryrun, boolean writeToConsole, boolean useThread)
	{
		if (!checkPrerequisites(filePath, "wmllint"))
			return null;

		File wmllintFile = new File(PreferenceInitializer.getString(PreferenceConstants.P_WESNOTH_WMLTOOLS_DIR) +
				Path.SEPARATOR + "wmllint");

		List<String> arguments = new ArrayList<String>();

		arguments.add(wmllintFile.getAbsolutePath());
		if (dryrun) arguments.add("--dryrun");
		arguments.add("--verbose");
		arguments.add("--nospellcheck");
		arguments.add(filePath);

		return runPythonScript(arguments, useThread, writeToConsole, "Wmllint result: ");
	}

	/**
	 * Runs "wmlindent" on the specified file
	 * @param filePath the full path of the target file where "wmlindent" will be runned on
	 * @param writeToConsole true to write the output of "wmlindent" in console
	 * @param dryrun true to run "wmlindent" in dry mode - i.e. no changes in the config file.
	 * @param useThread whether the tool should be runned in a new thread
	 */
	public static String runWMLIndent(String filePath, boolean dryrun, boolean writeToConsole, boolean useThread)
	{
		if (!checkPrerequisites(filePath, "wmlindent"))
			return null;

		File wmllintFile = new File(PreferenceInitializer.getString(PreferenceConstants.P_WESNOTH_WMLTOOLS_DIR) +
				Path.SEPARATOR + "wmlindent");
		List<String> arguments = new ArrayList<String>();

		arguments.add(wmllintFile.getAbsolutePath());
		if (dryrun) arguments.add("--dryrun");
		arguments.add("--verbose");
		arguments.add(filePath);

		return runPythonScript(arguments, useThread, writeToConsole,"Wmlindent result: ");
	}

	/**
	 * Checks if a wmlTool (that is in the wml tools directory) and
	 * an additional file that is target of the tool exist / are valid.
	 * @param filePath the file to be processed by the wml tool
	 * @param wmlTool the wml tool file
	 * @return
	 */
	private static boolean checkPrerequisites(String filePath, String wmlTool)
	{
		File wmlToolFile = new File(PreferenceInitializer.getString(PreferenceConstants.P_WESNOTH_WMLTOOLS_DIR) +
				Path.SEPARATOR + wmlTool);

		if (!wmlToolFile.exists())
		{
			GUIUtils.showMessageBox(WorkspaceUtils.getWorkbenchWindow(),
			"Please set the wmltools directory in the preferences before you use this feature.");
			return false;
		}

		if (filePath == null || filePath.isEmpty() || !new File(filePath).exists())
			return false;

		return true;
	}

	/**
	 * Runs a specified python script with the specified arguments
	 * @param arguments the arguments of the "python" executable.
	 * The first argument should be the script file name
	 * @param useThread
	 * @param writeToConsole true to write the script output to user's console
	 * @return
	 */
	private static String runPythonScript(List<String> arguments, boolean useThread,
			boolean writeToConsole, String consoleTitle)
	{
		String result = "";
		try
		{
			ExternalToolInvoker pyscript = new ExternalToolInvoker("python", arguments, useThread);

			pyscript.run();
			pyscript.waitFor();
			if (writeToConsole)
			{
				MessageConsole console = new MessageConsole(consoleTitle, null);
				console.activate();
				ConsolePlugin.getDefault().getConsoleManager().addConsoles(new IConsole[]{ console });
				MessageConsoleStream stream = console.newMessageStream();

				String line = "";
				while((line = pyscript.readOutputLine())!= null)
				{
					stream.write(line);
					result += line;
				}
				while((line = pyscript.readErrorLine())!= null)
				{
					stream.write(line);
					result += line;
				}
			}
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
		return result;
	}
}
