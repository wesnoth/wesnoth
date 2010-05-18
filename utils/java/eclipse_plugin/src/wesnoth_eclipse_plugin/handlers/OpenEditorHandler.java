package wesnoth_eclipse_plugin.handlers;

import java.io.File;

import org.eclipse.core.commands.AbstractHandler;
import org.eclipse.core.commands.ExecutionEvent;
import org.eclipse.core.commands.ExecutionException;
import org.eclipse.swt.widgets.MessageBox;
import org.eclipse.ui.IWorkbenchWindow;
import org.eclipse.ui.handlers.HandlerUtil;

import wesnoth_eclipse_plugin.builder.ExternalToolInvoker;
import wesnoth_eclipse_plugin.preferences.PreferenceConstants;
import wesnoth_eclipse_plugin.preferences.PreferenceInitializer;

public class OpenEditorHandler extends AbstractHandler
{
	/**
	 *  the arguments to launch the editor
	 *  %s - the filename of the file to be edited
	 *  %s - the data directory
	 */
	public final static String EDITOR_LAUNCH_ARGS = "-e %s";// %s";;
	@Override
	public Object execute(ExecutionEvent event) throws ExecutionException
	{
		IWorkbenchWindow window = HandlerUtil.getActiveWorkbenchWindowChecked(event);
		String editorPath = PreferenceInitializer.getString(PreferenceConstants.P_WESNOTH_EXEC_PATH);
		String workingDir = PreferenceInitializer.getString(PreferenceConstants.P_WESNOTH_WORKING_DIR);

		if (workingDir == "")
			workingDir = editorPath.substring(0,editorPath.lastIndexOf(new File(editorPath).getName()));

		if (editorPath == "")
		{
			MessageBox box = new MessageBox(window.getShell());
			box.setMessage(String.format("Please set the wesnoth's executable path first."));
			box.open();
			return null;
		}

		System.out.printf("Running: [%s] with args: [%s]\n", editorPath,getLaunchEditorArguments("", workingDir));
		ExternalToolInvoker.launchTool(editorPath, getLaunchEditorArguments("", workingDir),true, true);
		return null;
	}

	public static String getLaunchEditorArguments(String mapName, String workingDir)
	{
		String args = String.format(EDITOR_LAUNCH_ARGS, mapName,workingDir);
		return args;
	}
}
