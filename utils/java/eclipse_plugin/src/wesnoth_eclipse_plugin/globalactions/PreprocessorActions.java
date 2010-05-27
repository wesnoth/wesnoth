/**
 * @author Timotei Dolean
 */
package wesnoth_eclipse_plugin.globalactions;

import java.util.ArrayList;
import java.util.List;

import org.eclipse.core.filesystem.EFS;
import org.eclipse.core.filesystem.IFileStore;
import org.eclipse.core.resources.IFile;
import org.eclipse.core.runtime.Path;
import org.eclipse.ui.ide.IDE;

import wesnoth_eclipse_plugin.builder.ExternalToolInvoker;
import wesnoth_eclipse_plugin.preferences.PreferenceConstants;
import wesnoth_eclipse_plugin.preferences.PreferenceInitializer;
import wesnoth_eclipse_plugin.utils.WorkspaceUtils;

public class PreprocessorActions
{
	/**
	 * preprocesses a file using the wesnoth's executable
	 * @param fileName the file to process
	 * @param targetDirectory target directory where should be put the results
	 * @param defines the list of additional defines to be added when preprocessing the file
	 * @param useThread true if the preprocessing should use a thread
	 * @return
	 */
	public static boolean preprocessFile(String fileName,String targetDirectory,List<String> defines, boolean useThread)
	{
		try{
			List<String> arguments = new ArrayList<String>();

			if (defines != null && !defines.isEmpty())
			{
				String argument = "-p=";
				for(int i=0;i<defines.size()-1;i++)
				{
					argument += (defines.get(i) + ",");
				}
				argument  += defines.get(defines.size()-1);
			}
			else
			{
				arguments.add("-p");
			}
			arguments.add(fileName);
			arguments.add(targetDirectory);

			ExternalToolInvoker wesnoth = new ExternalToolInvoker(
					PreferenceInitializer.getString(PreferenceConstants.P_WESNOTH_EXEC_PATH),
					arguments, useThread);
			System.out.printf("preprocessing : %s\n", arguments);
			wesnoth.run();
			return true;
		}
		catch (Exception e) {
			e.printStackTrace();
			return false;
		}
	}

	/**
	 * Opens the preprocessed version of the specified file
	 * @param file the file to show preprocessed output
	 * @param openPlain true if it should open the plain preprocessed version
	 * or false for the normal one
	 */
	public static void openPreprocessedFileInEditor(IFile file, boolean openPlain)
	{
		if (file == null)
			return;

		IFileStore preprocFile = EFS.getLocalFileSystem().getStore(new Path(WorkspaceUtils.getTemporaryFolder()));
		preprocFile = preprocFile.getChild(file.getName() + (openPlain == true? ".plain" : "") );

		try
		{
			IDE.openEditorOnFileStore(WorkspaceUtils.getWorkbenchWindow().getActivePage(), preprocFile);
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}
}
