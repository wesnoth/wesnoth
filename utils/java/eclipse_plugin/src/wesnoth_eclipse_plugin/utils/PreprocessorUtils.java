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
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import org.eclipse.core.filesystem.EFS;
import org.eclipse.core.filesystem.IFileStore;
import org.eclipse.core.resources.IFile;
import org.eclipse.core.runtime.Path;
import org.eclipse.ui.ide.IDE;

import wesnoth_eclipse_plugin.Constants;
import wesnoth_eclipse_plugin.Logger;
import wesnoth_eclipse_plugin.preferences.Preferences;

public class PreprocessorUtils
{
	private static HashMap<String, Long> filesTimeStamps_ = new HashMap<String, Long>();

	/**
	 * preprocesses a file using the wesnoth's executable, only
	 * if the file was modified since last time checked.
	 * The target directory is the temporary directory + files's path relative to project
	 * @param file the file to process
	 * @param defines the list of additional defines to be added when preprocessing the file
	 * @return
	 */
	public static boolean preprocessFile(IFile file, List<String> defines)
	{
		return preprocessFile(file, getTemporaryLocation(file), defines, true);
	}

	/**
	 * preprocesses a file using the wesnoth's executable, only
	 * if the file was modified since last time checked.
	 * @param file the file to process
	 * @param targetDirectory target directory where should be put the results
	 * @param defines the list of additional defines to be added when preprocessing the file
	 * @param waitForIt true to wait for the preprocessing to finish
	 * @return
	 */
	public static boolean preprocessFile(IFile file, String targetDirectory,
			List<String> defines, boolean waitForIt)
	{
		String filePath = file.getLocation().toOSString();
		if (filesTimeStamps_.containsKey(filePath) &&
				filesTimeStamps_.get(filePath) >= new File(filePath).lastModified())
		{
			Logger.getInstance().log("skipped preprocessing a non-modified file: " + filePath);
			return true;
		}

		filesTimeStamps_.put(filePath, new File(filePath).lastModified());

		try{
			List<String> arguments = new ArrayList<String>();

			arguments.add("--config-dir");
			arguments.add(Preferences.getString(Constants.P_WESNOTH_USER_DIR));

			arguments.add("--data-dir");
			arguments.add(Preferences.getString(Constants.P_WESNOTH_WORKING_DIR));

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
			arguments.add(filePath);
			arguments.add(targetDirectory);

			Logger.getInstance().log("preprocessing file: " + filePath);
			ExternalToolInvoker wesnoth = new ExternalToolInvoker(
					Preferences.getString(Constants.P_WESNOTH_EXEC_PATH),
					arguments);
			wesnoth.runTool();
			if (waitForIt)
				wesnoth.waitForTool();
			return true;
		}
		catch (Exception e) {
			Logger.getInstance().logException(e);
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
		if (file == null || !file.exists())
		{
			Logger.getInstance().log("file null or non existent.",
					"The file is null or does not exist");
			return;
		}

		try
		{
			IDE.openEditorOnFileStore(WorkspaceUtils.getWorkbenchWindow().getActivePage(),
					getPreprocessedFilePath(file, openPlain, true));
		}
		catch (Exception e)
		{
			Logger.getInstance().logException(e);
		}
	}

	/**
	 * Returns the path of the preprocessed file of the specified file
	 * @param file The file whom preprocessed file to get
	 * @param plain True to return the plain version file's file
	 * @param create if this is true, if the target preprocessed file
	 * doesn't exist it will be created.
	 * @return
	 */
	public static IFileStore getPreprocessedFilePath(IFile file, boolean plain,
			boolean create)
	{
		IFileStore preprocFile =
			EFS.getLocalFileSystem().getStore(new Path(getTemporaryLocation(file)));
		preprocFile = preprocFile.getChild(file.getName() + (plain == true? ".plain" : "") );
		if (create && !preprocFile.fetchInfo().exists())
			preprocessFile(file, null);
		return preprocFile;
	}

	/**
	 * Gets the temporary location where that file should be preprcessed
	 * @param file
	 * @return
	 */
	public static String getTemporaryLocation(IFile file)
	{
		String targetDirectory = WorkspaceUtils.getTemporaryFolder();
		targetDirectory += file.getProject().getName() + "/";
		targetDirectory += file.getParent().getProjectRelativePath().toOSString() + "/";
		return targetDirectory;
	}
}
