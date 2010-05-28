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
	 * @param filePath the full path of the file where "wmllint" will be runned on
	 * @param writeToConsole true to write the output of "wmllint" in console
	 * @param dryrun true to run "wmllint" in dry mode - i.e. no changes in the config file.
	 * @param useThread whether the tool should be runned in a new thread
	 */
	public static void runWMLLint(String filePath,boolean dryrun, boolean writeToConsole, boolean useThread)
	{
		File wmllintFile = new File(PreferenceInitializer.getString(PreferenceConstants.P_WESNOTH_WMLTOOLS_DIR) +
					Path.SEPARATOR + "wmllint");
		if (!wmllintFile.exists())
		{
			GUIUtils.showMessageBox(WorkspaceUtils.getWorkbenchWindow(),
					"Please set the wmltools directory in the preferences before you use this feature.");
			return;
		}

		if (filePath == null || filePath.isEmpty() || !new File(filePath).exists())
			return;

		List<String> arguments = new ArrayList<String>();

		arguments.add(wmllintFile.getAbsolutePath());
		arguments.add("--dryrun");
		arguments.add("--verbose");
		arguments.add("--nospellcheck");
		arguments.add(filePath);

		try
		{
			ExternalToolInvoker wmllint = new ExternalToolInvoker("python", arguments, useThread);

			wmllint.run();
			wmllint.waitFor();
			if (writeToConsole)
			{
				MessageConsole console = new MessageConsole("wmllint result", null);
				console.activate();
				ConsolePlugin.getDefault().getConsoleManager().addConsoles(new IConsole[]{ console });
				MessageConsoleStream stream = console.newMessageStream();
				String line = "";
				while((line = wmllint.readOutputLine())!= null)
					stream.write(line);
				while((line = wmllint.readErrorLine())!= null)
					stream.write(line);
			}
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}
}
