/*******************************************************************************
 * Copyright (c) 2010 - 2011 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.utils;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Date;
import java.util.List;

import org.eclipse.core.resources.IContainer;
import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IFolder;
import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.resources.WorkspaceJob;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.runtime.IStatus;
import org.eclipse.core.runtime.Path;
import org.eclipse.core.runtime.Status;
import org.eclipse.jface.preference.PreferenceDialog;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.Display;
import org.eclipse.ui.IViewPart;
import org.eclipse.ui.IWorkbench;
import org.eclipse.ui.IWorkbenchWindow;
import org.eclipse.ui.IWorkingSetManager;
import org.eclipse.ui.dialogs.PreferencesUtil;
import org.wesnoth.Logger;
import org.wesnoth.Messages;
import org.wesnoth.WesnothPlugin;
import org.wesnoth.installs.WesnothInstallsUtils;
import org.wesnoth.navigator.WesnothProjectsExplorer;
import org.wesnoth.preferences.Preferences;
import org.wesnoth.projects.ProjectUtils;

public class WorkspaceUtils
{
	private static String	temporaryFolder_	= null;

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
	 * Gets the project from the workspace that has the specified name,
	 * or null if none existing
	 * @return
	 */
	public static IProject getProject(String name)
	{
		IProject proj = ResourcesPlugin.getWorkspace().getRoot().getProject(name);
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
	 * Returns the current workbench
	 * @return
	 */
	public static IWorkbench getWorkbench()
	{
	    return WesnothPlugin.getDefault().getWorkbench();
	}

	/**
	 * Returns the first WorkbenchWindow available. This is not always the same
	 * with ActiveWorkbecnWindow
	 *
	 * @return
	 */
	public static IWorkbenchWindow getWorkbenchWindow()
	{
		if (getWorkbench().getActiveWorkbenchWindow() != null)
			return getWorkbench().getActiveWorkbenchWindow();

		if (getWorkbench().getWorkbenchWindowCount() == 0)
			return null;

		return getWorkbench().getWorkbenchWindows()[0];
	}

	/**
	 * Returns the current working set manager
	 * @return
	 */
	public static IWorkingSetManager getWorkingSetManager()
	{
		return getWorkbench().getWorkingSetManager();
	}

	/**
	 * Returns the view with the specified id or null if none found
	 * @param id The id of the searched View
	 * @return
	 */
	public static IViewPart getView(String id)
	{
	    return getWorkbenchWindow().getActivePage().findView(id);
	}

	/**
	 * Returns the Wesnoth Projects Explorer view
	 * @return
	 */
	public static WesnothProjectsExplorer getProjectsExplorer()
	{
	    return (WesnothProjectsExplorer)getView(WesnothProjectsExplorer.ID_PROJECTS_EXPLORER);
	}

	/**
	 * Returns the temporary folder where the plugin can write resources
	 *
	 * @return
	 */
	public static String getTemporaryFolder()
	{
		if (temporaryFolder_ == null || temporaryFolder_.isEmpty())
		{
			temporaryFolder_ = System.getProperty("java.io.tmpdir") + //$NON-NLS-1$
						Path.SEPARATOR + "wesnoth_plugin" + Path.SEPARATOR; //$NON-NLS-1$

			File tmpFile = new File(temporaryFolder_);

            tmpFile.mkdirs();
			if (!tmpFile.exists())
				temporaryFolder_ = null;
		}
		return temporaryFolder_;
	}

	/**
	 * Gets the project's temporary folder
	 * @param project The project for which to compute the temporary folder
	 * @return
	 */
	public static String getProjectTemporaryFolder( IProject project )
	{
	    return getTemporaryFolder( ) + project.getName( );
	}

	/**
	 * Returns a random fileName generated from current time
	 * @return
	 */
	public static String getRandomFileName()
	{
		String result = ""; //$NON-NLS-1$
		SimpleDateFormat date = new SimpleDateFormat("yyyy_MM_dd_HH_mm_ss"); //$NON-NLS-1$
		result += date.format(new Date());
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

		String result = Preferences.getPaths(
		        WesnothInstallsUtils.getInstallNameForResource( resource ) ).getAddonsDir( );
		result += resource.getProject().getName() + Path.SEPARATOR;
		result += resource.getProjectRelativePath().toOSString();
		return result;
	}

	/**
	 * Setups the workspace, by checking:
	 * 1) The user has set all plugin's preferences.
	 * 	If not, the preferences window will open
	 */
	public static void setupWorkspace(final boolean guided)
	{
        if (guided)
        {
            GUIUtils.showInfoMessageBox( Messages.WorkspaceUtils_0 );
        }

        if (!checkPathsAreSet( Preferences.getDefaultInstallName( ), false ))
        {
            PreferenceDialog pref = PreferencesUtil.createPreferenceDialogOn(
                    WesnothPlugin.getShell(), "org.wesnoth.preferences.InstallsPage", null, null); //$NON-NLS-1$
            pref.open();
            if ( !checkPathsAreSet ( Preferences.getDefaultInstallName( ), true ) )
            {
                GUIUtils.showErrorMessageBox( Messages.WorkspaceUtils_7 );
                return;
            }
        }

        if (guided)
        {
            GUIUtils.showInfoMessageBox( Messages.WorkspaceUtils_9 );
        }

        WorkspaceJob job = new WorkspaceJob( Messages.WorkspaceUtils_13 ){
            @Override
            public IStatus runInWorkspace(IProgressMonitor monitor)
            {
                try
                {
                    // automatically import 'special' folders as projects
                    List<File> files = new ArrayList<File>();
                    String addonsDir = Preferences.getPaths( null ).getAddonsDir( );
                    String campaignsDir = Preferences.getPaths( null ).getCampaignDir( );

                    if (GUIUtils.showMessageBox( Messages.WorkspaceUtils_18 ,
                            SWT.ICON_QUESTION | SWT.YES | SWT.NO) == SWT.YES)
                    {
                        // useraddons/add-ons/data
                        File[] tmp = new File(addonsDir).listFiles();
                        if (tmp != null)
                            files.addAll(Arrays.asList(tmp));
                    }

                    if (GUIUtils.showMessageBox( Messages.WorkspaceUtils_20 ,
                            SWT.ICON_QUESTION | SWT.YES | SWT.NO) == SWT.YES)
                    {
                        // workingdir/data/campaigns
                        File[] tmp = new File(campaignsDir).listFiles();
                        if (tmp != null)
                            files.addAll(Arrays.asList(tmp));
                    }

                    monitor.beginTask(Messages.WorkspaceUtils_22, files.size() * 35);
                    for(File file: files)
                    {
                        if (file.isDirectory() == false ||
                            file.getName().startsWith(".")) //$NON-NLS-1$
                            continue;

                        String projectName = file.getName();
                        if (StringUtils.normalizePath(file.getAbsolutePath()).contains(
                            StringUtils.normalizePath(campaignsDir)))
                        {
                            projectName = "_Mainline_" + file.getName(); //$NON-NLS-1$
                        }

                        ProjectUtils.createWesnothProject( projectName,
                                file.getAbsolutePath( ), false, monitor );
                    }

                    if (guided)
                    {
                        GUIUtils.showInfoMessageBox( Messages.WorkspaceUtils_25 );
                    }
                    else
                    {
                        Logger.getInstance().log("setupWorkspace was successful", //$NON-NLS-1$
                                Messages.WorkspaceUtils_29);
                    }
                } catch (Exception e)
                {
                    Logger.getInstance().logException(e);
                    GUIUtils.showErrorMessageBox(Messages.WorkspaceUtils_30);
                }

                monitor.done();
                return Status.OK_STATUS;
            }
        };
        job.schedule();
	}

	/**
	 * Checks if the user has set some needed preferences and if the workspace
	 * is setup
	 *
	 * @param displayWarning true to display a messagebox warning
	 * 		  the user if conditions are not met
	 */
	public static boolean checkPathsAreSet( String installName, boolean displayWarning)
	{
		if ( !ResourceUtils.isValidFilePath( Preferences.getPaths( installName ).getWesnothExecutablePath( ) ) ||
		     !ResourceUtils.isValidFilePath( Preferences.getPaths( installName ).getUserDir( ) ) ||
			 !ResourceUtils.isValidFilePath( Preferences.getPaths( installName ).getWMLToolsDir( ) ) ||
			 !ResourceUtils.isValidFilePath( Preferences.getPaths( installName ).getWorkingDir( ) ))
		{
			if (displayWarning)
				GUIUtils.showWarnMessageBox(Messages.WorkspaceUtils_33);
			return false;
		}

		return true;
	}
}
