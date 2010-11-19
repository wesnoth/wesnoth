/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.utils;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.eclipse.core.filesystem.EFS;
import org.eclipse.core.filesystem.IFileStore;
import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.runtime.IPath;
import org.eclipse.core.runtime.Path;
import org.eclipse.jface.dialogs.DialogSettings;
import org.wesnoth.Activator;
import org.wesnoth.Constants;
import org.wesnoth.Logger;
import org.wesnoth.Messages;
import org.wesnoth.preferences.Preferences;

public class PreprocessorUtils
{
	private static PreprocessorUtils instance_;
	private Map<String, Long> filesTimeStamps_ = new HashMap<String, Long>();

	private PreprocessorUtils()
	{
		filesTimeStamps_ = new HashMap<String, Long>();
		restoreTimestamps();
	}

	public static PreprocessorUtils getInstance()
	{
		if (instance_ == null)
			instance_ = new PreprocessorUtils();
		return instance_;
	}

	/**
	 * preprocesses a file using the wesnoth's executable, only
	 * if the file was modified since last time checked.
	 * The target directory is the temporary directory + files's path relative to project
	 * @param file the file to process
	 * @param defines the list of additional defines to be added when preprocessing the file
	 * @return
	 */
	public int preprocessFile(IFile file, List<String> defines)
	{
		return preprocessFile(file, getTemporaryLocation(file),
				getTemporaryLocation(file) + "/_MACROS_.cfg", defines, true); //$NON-NLS-1$
	}

	/**
	 * preprocesses a file using the wesnoth's executable, only
	 * if the file was modified since last time checked.
	 * The target directory is the temporary directory + files's path relative to project
	 * @param file the file to process
	 * @param macrosFile The file where macros are stored
	 * @param defines the list of additional defines to be added when preprocessing the file
	 * @return
	 */
	public int preprocessFile(IFile file, String macrosFile, List<String> defines)
	{
		return preprocessFile(file, getTemporaryLocation(file), macrosFile, defines, true);
	}

	/**
	 * preprocesses a file using the wesnoth's executable, only
	 * if the file was modified since last time checked.
	 * @param file the file to process
	 * @param targetDirectory target directory where should be put the results
	 * @param macrosFile The file where macros are stored
	 * @param defines the list of additional defines to be added when preprocessing the file
	 * @param waitForIt true to wait for the preprocessing to finish
	 * @return
	 * 	-1 - we skipped preprocessing - file was already preprocessed
	 * 	 0 - preprocessed succesfully
	 *   1 - there was an error
	 */
	public int preprocessFile(IFile file, String targetDirectory,
			String macrosFile, List<String> defines, boolean waitForIt)
	{
		String filePath = file.getLocation().toOSString();
		if (filesTimeStamps_.containsKey(filePath) &&
			filesTimeStamps_.get(filePath) >= new File(filePath).lastModified())
		{
			Logger.getInstance().log(Messages.PreprocessorUtils_1 + filePath);
			return -1;
		}

		filesTimeStamps_.put(filePath, new File(filePath).lastModified());

		try{
			List<String> arguments = new ArrayList<String>();

			arguments.add("--config-dir"); //$NON-NLS-1$
			arguments.add(Preferences.getString(Constants.P_WESNOTH_USER_DIR));

			arguments.add("--data-dir"); //$NON-NLS-1$
			arguments.add(Preferences.getString(Constants.P_WESNOTH_WORKING_DIR));

			if (macrosFile != null && macrosFile.isEmpty() == false)
			{
				ResourceUtils.createNewFile(macrosFile);

				// add the _MACROS_.cfg file
				arguments.add("--preprocess-input-macros"); //$NON-NLS-1$
				arguments.add(macrosFile);


				arguments.add("--preprocess-output-macros"); //$NON-NLS-1$
				arguments.add(macrosFile);
			}

			if (Preferences.getBool(Constants.P_ADV_NO_TERRAIN_GFX))
			{
				if (defines == null)
					defines = new ArrayList<String>();
				defines.add("NO_TERRAIN_GFX"); //$NON-NLS-1$
			}

			if (defines != null && !defines.isEmpty())
			{
				String argument = "-p="; //$NON-NLS-1$
				for(int i=0;i< defines.size() - 1;i++)
				{
					argument += (defines.get(i) + ","); //$NON-NLS-1$
				}
				argument += defines.get(defines.size()-1);
				arguments.add(argument);
			}
			else
			{
				arguments.add("-p"); //$NON-NLS-1$
			}
			arguments.add(filePath);
			arguments.add(targetDirectory);

			Logger.getInstance().log(Messages.PreprocessorUtils_10 + filePath);
			ExternalToolInvoker wesnoth = new ExternalToolInvoker(
					Preferences.getString(Constants.P_WESNOTH_EXEC_PATH),
					arguments);
			wesnoth.runTool();
			if (waitForIt)
				return wesnoth.waitForTool();
			return 0;
		}
		catch (Exception e) {
			Logger.getInstance().logException(e);
			return 1;
		}
	}

	/**
	 * Opens the preprocessed version of the specified file
	 * @param file the file to show preprocessed output
	 * @param openPlain true if it should open the plain preprocessed version
	 * or false for the normal one
	 */
	public void openPreprocessedFileInEditor(IFile file, boolean openPlain)
	{
		if (file == null || !file.exists())
		{
			Logger.getInstance().log(Messages.PreprocessorUtils_11,
					Messages.PreprocessorUtils_12);
			return;
		}
		EditorUtils.openEditor(getPreprocessedFilePath(file, openPlain, true));
	}

	/**
	 * Returns the path of the preprocessed file of the specified file
	 * @param file The file whom preprocessed file to get
	 * @param plain True to return the plain version file's file
	 * @param create if this is true, if the target preprocessed file
	 * doesn't exist it will be created.
	 * @return
	 */
	public IFileStore getPreprocessedFilePath(IFile file, boolean plain,
			boolean create)
	{
		IFileStore preprocFile =
			EFS.getLocalFileSystem().getStore(new Path(getTemporaryLocation(file)));
		preprocFile = preprocFile.getChild(file.getName() + (plain == true? ".plain" : "") ); //$NON-NLS-1$ //$NON-NLS-2$
		if (create && !preprocFile.fetchInfo().exists())
			preprocessFile(file, null);
		return preprocFile;
	}

	/**
	 * Gets the temporary location where that file should be preprcessed
	 * @param file
	 * @return
	 */
	public String getTemporaryLocation(IFile file)
	{
		String targetDirectory = WorkspaceUtils.getTemporaryFolder();
		targetDirectory += file.getProject().getName() + "/"; //$NON-NLS-1$
		targetDirectory += file.getParent().getProjectRelativePath().toOSString() + "/"; //$NON-NLS-1$
		return targetDirectory;
	}

	/**
	 * Gets the location where the '_MACROS_.cfg' file is for the
	 * specified resource.
	 *
	 * Currently we store just a defines file per project.
	 * @param resource
	 * @return
	 */
	public String getDefinesLocation(IResource resource)
	{
		return WorkspaceUtils.getTemporaryFolder() +
				resource.getProject().getName() +
				"/_MACROS_.cfg"; //$NON-NLS-1$
	}

	/**
	 * Saves the current timestamps for preprocessed files
	 * to filesystem
	 */
	public void saveTimestamps()
	{
		IPath path = Activator.getDefault().getStateLocation();
		String filename = path.append("preprocessed.txt").toOSString(); //$NON-NLS-1$
		DialogSettings settings = new DialogSettings("preprocessed"); //$NON-NLS-1$
		try
		{
			settings.put("files", filesTimeStamps_.keySet().toArray(new String[0])); //$NON-NLS-1$
			List<String> timestamps = new ArrayList<String>();
			for(Long timestamp : filesTimeStamps_.values())
			{
				timestamps.add(timestamp.toString());
			}
			settings.put("timestamps", timestamps.toArray(new String[0])); //$NON-NLS-1$
			settings.save(filename);
		}
		catch (Exception e)
		{
			Logger.getInstance().logException(e);
		}
	}

	/**
	 * Restores the timestamps for preprocessed files from
	 * the filesystem
	 */
	public void restoreTimestamps()
	{
		IPath path = Activator.getDefault().getStateLocation();
		String filename = path.append("preprocessed.txt").toOSString(); //$NON-NLS-1$
		DialogSettings settings = new DialogSettings("preprocessed"); //$NON-NLS-1$
		filesTimeStamps_.clear();

		try
		{
			// ensure the creation of a valid file if it doesn't exist
			if (new File(filename).exists() == false)
				settings.save(filename);

			settings.load(filename);
			String[] timestamps = settings.getArray("timestamps"); //$NON-NLS-1$
			String[] files = settings.getArray("files"); //$NON-NLS-1$
			if (timestamps != null && files != null &&
				timestamps.length == files.length)
			{
				for(int index = 0 ;index < files.length; ++index)
				{
					filesTimeStamps_.put(files[index], Long.valueOf(timestamps[index]));
				}
			}
		}
		catch (IOException e)
		{
			Logger.getInstance().logException(e);
		}
	}
}
