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
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.Locale;
import java.util.Properties;
import java.util.Random;

import org.eclipse.core.resources.FileInfoMatcherDescription;
import org.eclipse.core.resources.IContainer;
import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IFolder;
import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IProjectDescription;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.resources.IResourceFilterDescription;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.NullProgressMonitor;
import org.eclipse.core.runtime.Path;
import org.eclipse.jface.preference.PreferenceDialog;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.window.Window;
import org.eclipse.swt.widgets.Display;
import org.eclipse.ui.IWorkbenchWindow;
import org.eclipse.ui.dialogs.PreferencesUtil;

import wesnoth_eclipse_plugin.Activator;
import wesnoth_eclipse_plugin.Constants;
import wesnoth_eclipse_plugin.Logger;
import wesnoth_eclipse_plugin.preferences.Preferences;
import wesnoth_eclipse_plugin.wizards.ReplaceableParameter;
import wesnoth_eclipse_plugin.wizards.TemplateProvider;

public class WorkspaceUtils
{
	private static String	temporaryFolder_	= "";

	/**
	 * Gets the selected project or or null if none selected
	 * @return
	 */
	public static IProject getSelectedProject()
	{
		return getSelectedProject(WorkspaceUtils.getWorkbenchWindow());
	}

	/**
	 * Gets the selected project or null if none selected
	 * @param window The workbench windows from where to get the current selection
	 * @return
	 */
	public static IProject getSelectedProject(IWorkbenchWindow window)
	{
		IStructuredSelection selection = getSelectedStructuredSelection(window);
		if (selection == null || !(selection.getFirstElement() instanceof IProject))
			return null;

		return (IProject) selection.getFirstElement();
	}

	/**
	 * Gets the 'user addons' project from the workspace, or null if none existing
	 * @return
	 */
	public static IProject getUserAddonsProject()
	{
		IProject proj = ResourcesPlugin.getWorkspace().getRoot().getProject("User Addons");
		if (proj.exists())
			return proj;
		return null;
	}

	/**
	 * Gets the selected folder or null if none selected
	 * @return
	 */
	public static IFolder getSelectedFolder()
	{
		return getSelectedFolder(WorkspaceUtils.getWorkbenchWindow());
	}

	/**
	 * Gets the selected project or null if none selected
	 * @param window The workbench window from where to get the current selection
	 * @return
	 */
	public static IFolder getSelectedFolder(IWorkbenchWindow window)
	{
		IStructuredSelection selection = getSelectedStructuredSelection(window);
		if (selection == null || !(selection.getFirstElement() instanceof IFolder))
			return null;

		return (IFolder) selection.getFirstElement();
	}

	/**
	 * Gets the selected file or null if none selected
	 * @return
	 */
	public static IFile getSelectedFile()
	{
		return getSelectedFile(WorkspaceUtils.getWorkbenchWindow());
	}

	/**
	 * Gets the file selected or null if none selected
	 * @param window The workbench window from where to get the current selection
	 * @return
	 */
	public static IFile getSelectedFile(IWorkbenchWindow window)
	{
		IStructuredSelection selection = getSelectedStructuredSelection(window);
		if (selection == null || !(selection.getFirstElement() instanceof IFile))
			return null;
		return (IFile) selection.getFirstElement();
	}

	/**
	 * Gets the selected StructuredSelection or null if none selected
	 * @return
	 */
	public static IStructuredSelection getSelectedStructuredSelection()
	{
		return getSelectedStructuredSelection(WorkspaceUtils.getWorkbenchWindow());
	}
	/**
	 * Gets the selected StructuredSelection or null if none selected
	 * @param window The workbench windows from where to get the current selection
	 * @return
	 */
	public static IStructuredSelection getSelectedStructuredSelection(final IWorkbenchWindow window)
	{
		if (window == null)
			return null;
		MyRunnable<IStructuredSelection> runnable = new MyRunnable<IStructuredSelection>(){
			@Override
			public void run()
			{
				try{
					runnableObject_ = null;
					if (!(window.getSelectionService().getSelection() instanceof IStructuredSelection))
						return;
					runnableObject_ = (IStructuredSelection) window.getSelectionService().getSelection();
				}
				catch (Exception e) {
					e.printStackTrace();
				}
			}
		};
		Display.getDefault().syncExec(runnable);
		return runnable.runnableObject_;
	}

	/**
	 * Gets the selected container (folder/project) or null if none selected
	 * @return
	 */
	public static IContainer getSelectedContainer()
	{
		IStructuredSelection selection = getSelectedStructuredSelection();
		if (selection == null ||
			!(selection.getFirstElement() instanceof IFolder ||
			  selection.getFirstElement() instanceof IProject))
			return null;

		return (IContainer) selection.getFirstElement();
	}

	/**
	 * Gets the selected resource(file/folder/project) or null if none selected
	 * @return
	 */
	public static IResource getSelectedResource()
	{
		IResource res = getSelectedFile();
		if (res != null)
			return res;

		res = getSelectedFolder();
		if (res != null)
			return res;

		res = getSelectedProject();
		if (res != null)
			return res;
		return null;
	}

	/**
	 * Returns the first WorkbenchWindow available. This is not always the same
	 * with ActiveWorkbecnWindow
	 *
	 * @return
	 */
	public static IWorkbenchWindow getWorkbenchWindow()
	{
		if (Activator.getDefault().getWorkbench().getActiveWorkbenchWindow() != null)
			return Activator.getDefault().getWorkbench().getActiveWorkbenchWindow();
		if (Activator.getDefault().getWorkbench().getWorkbenchWindowCount() == 0)
			return null;
		return Activator.getDefault().getWorkbench().getWorkbenchWindows()[0];
	}

	/**
	 * Returns the temporary folder where the plugin can write resources
	 *
	 * @return
	 */
	public static String getTemporaryFolder()
	{
		if (temporaryFolder_.isEmpty())
		{
			temporaryFolder_ = System.getProperty("java.io.tmpdir") +
						Path.SEPARATOR + "wesnoth_plugin" + Path.SEPARATOR;

			File tmpFile = new File(temporaryFolder_);
			if (!tmpFile.exists())
				tmpFile.mkdirs();
		}
		return temporaryFolder_;
	}

	/**
	 * Returns a random fileName generated from current time
	 * @return
	 */
	public static String getRandomFileName()
	{
		String result = "";
		SimpleDateFormat date = new SimpleDateFormat("yyyyMMddHHmm");
		result += date.format(new Date());
		result += String.valueOf(new Random().nextInt());
		return result;
	}

	/**
	 * Returns the resource path relative to the user directory
	 *
	 * @param resource the resource to be computed
	 * @return
	 */
	public static String getPathRelativeToUserDir(IResource resource)
	{
		if (resource == null)
			return null;

		String result = Preferences.getString(Constants.P_WESNOTH_USER_DIR) +
							Path.SEPARATOR + "data/add-ons/";
		if (!resource.getProject().getName().toLowerCase(new Locale("English")).equals("user addons"))
			result += (resource.getProject().getName() + Path.SEPARATOR);
		result += resource.getProjectRelativePath().toOSString();
		return result;
	}

	/**
	 * Setups the workspace, by checking:
	 * 1) The user has set all plugin's preferences.
	 * 	If not, the preferences window will open
	 * 2) The project "User addons" exists. If not, it will be created
	 */
	public static void setupWorkspace()
	{
		if (!checkConditions(false))
		{
			PreferenceDialog pref = PreferencesUtil.createPreferenceDialogOn(
					Activator.getShell(), "plugin_preferences", null, null);
			if (pref.open() == Window.CANCEL || !checkConditions(true))
			{
				GUIUtils.showErrorMessageBox("The workspace was not setup");
				return;
			}
		}

		// automatically import "WesnothUserDir/data/add-ons as a project container
		String userDir = Preferences.getString(Constants.P_WESNOTH_USER_DIR);
		IProject projectToCreate = ResourcesPlugin.getWorkspace().getRoot().getProject("User Addons");
		try
		{
			if (!projectToCreate.exists())
			{
				IProjectDescription description =
						ResourcesPlugin.getWorkspace().newProjectDescription("User Addons");

				// cleanup any strictly-project related files if any.
				if (new File(userDir + Path.SEPARATOR + "data/add-ons/.project").exists())
					new File(userDir + Path.SEPARATOR + "data/add-ons/.project").delete();

				description.setLocation(new Path(userDir + Path.SEPARATOR + "data/add-ons/"));
				projectToCreate.create(description, null);
				projectToCreate.open(null);

				// the nature isn't set on creation so the nature adds the builder aswell
				description.setNatureIds(new String[] { Constants.NATURE_WESNOTH,
						Constants.NATURE_XTEXT });
				projectToCreate.setDescription(description, null);

				// add the build.xml file
				ArrayList<ReplaceableParameter> param = new ArrayList<ReplaceableParameter>();
				param.add(new ReplaceableParameter("$$project_name", "User Addons"));
				param.add(new ReplaceableParameter("$$project_dir_name", ""));
				ResourceUtils.createFile(projectToCreate, "build.xml",
						TemplateProvider.getInstance().getProcessedTemplate("build_xml", param), true);

				// we need to skip the already created projects (if any) in the addons directory
				StringBuilder skipList = new StringBuilder();
				for (IProject project : ResourcesPlugin.getWorkspace().getRoot().getProjects())
				{
					if (project.getName().equals("User Addons"))
						continue;
					if (skipList.length() > 0)
						skipList.append(",");
					skipList.append(StringUtils.trimPathSeparators(getPathRelativeToUserDir(project)));

					// hide the existing projects
					createIgnoreFilter(projectToCreate, project.getName());
				}
				Properties props = new Properties();
				props.setProperty("ignored", skipList.toString());
				ProjectUtils.setPropertiesForProject(projectToCreate, props);
			}

			Logger.getInstance().log("setupWorkspace was successful",
					"Workspace was set up successfully.");
		} catch (Exception e)
		{
			Logger.getInstance().logException(e);
			GUIUtils.showErrorMessageBox("There was an error trying to setup the workspace.");

			// let's remove the corrupted project
			try
			{
				projectToCreate.delete(true, null);
			}
			catch (CoreException e1)
			{
			}
		}
	}

	/**
	 * Creates and adds an ignore filter on the specified project
	 * for folders matching the specified name. The specified folder won't appear
	 * in any views of the current project's structure
	 * @param project The project where to create the filter
	 * @param folderName The foldername to ignore
	 * @throws CoreException
	 */
	public static void createIgnoreFilter(IProject project, String folderName)
		throws CoreException
	{
		// For details regarding the description see:
		// 	FileInfoAttributesMatcher.encodeArguments()

		// id = org.eclipse.ui.ide.multiFilter
		// args = 1.0-name-matches-false-false-Love_to_death
		project.createFilter(IResourceFilterDescription.EXCLUDE_ALL | IResourceFilterDescription.FOLDERS,
				new FileInfoMatcherDescription("org.eclipse.ui.ide.multiFilter",
						"1.0-name-matches-false-false-" + folderName),
				IResource.BACKGROUND_REFRESH, new NullProgressMonitor());
	}

	/**
	 * Checks if the user has set some needed preferences and if the workspace
	 * is setup (there exists the "User Addons" project)
	 *
	 * @param displayWarning true to display a messagebox warning
	 * 		  the user if conditions are not met
	 */
	private static boolean checkConditions(boolean displayWarning)
	{
		String execDir = Preferences.getString(Constants.P_WESNOTH_EXEC_PATH);
		String userDir = Preferences.getString(Constants.P_WESNOTH_USER_DIR);
		String wmltoolsDir = Preferences.getString(Constants.P_WESNOTH_WMLTOOLS_DIR);
		String workingDir = Preferences.getString(Constants.P_WESNOTH_WORKING_DIR);

		if (!validPath(execDir) || !validPath(userDir) ||
			!validPath(wmltoolsDir) || !validPath(workingDir))
		{
			if (displayWarning)
				GUIUtils.showWarnMessageBox("Please set all plugin's preferences before using it.");
			return false;
		}
		return true;
	}

	/**
	 * Checks if the path is valid and the specified path's resource exists
	 * @param path the path to check
	 * @return
	 */
	public static boolean validPath(String path)
	{
		return !path.isEmpty() && new File(path).exists();
	}
}
