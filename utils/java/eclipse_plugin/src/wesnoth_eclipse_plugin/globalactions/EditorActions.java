/**
 * @author Timotei Dolean
 */
package wesnoth_eclipse_plugin.globalactions;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import wesnoth_eclipse_plugin.builder.ExternalToolInvoker;
import wesnoth_eclipse_plugin.preferences.PreferenceConstants;
import wesnoth_eclipse_plugin.preferences.PreferenceInitializer;
import wesnoth_eclipse_plugin.utils.GUIUtils;
import wesnoth_eclipse_plugin.utils.WorkspaceUtils;

public class EditorActions
{
	public static void startEditor(String mapName)
	{
		String editorPath = PreferenceInitializer.getString(PreferenceConstants.P_WESNOTH_EXEC_PATH);
		String workingDir = PreferenceInitializer.getString(PreferenceConstants.P_WESNOTH_WORKING_DIR);

		if (workingDir.isEmpty())
			workingDir = editorPath.substring(0, editorPath.lastIndexOf(new File(editorPath).getName()));

		if (editorPath.isEmpty())
		{
			GUIUtils.showMessageBox(WorkspaceUtils.getWorkbenchWindow(), "Please set the wesnoth's executable path first.");
			return;
		}

		System.out.printf("Running: [%s] with args: %s\n", editorPath, getLaunchEditorArguments(mapName, workingDir));
		ExternalToolInvoker.launchTool(editorPath, getLaunchEditorArguments(mapName, workingDir), true, false, true,
									WorkspaceUtils.getWorkbenchWindow());
	}

	public static List<String> getLaunchEditorArguments(String mapName, String workingDir)
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
