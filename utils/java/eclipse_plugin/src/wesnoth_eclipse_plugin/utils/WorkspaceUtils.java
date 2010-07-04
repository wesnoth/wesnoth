/**
 * @author Timotei Dolean
 */
package wesnoth_eclipse_plugin.utils;

import java.io.File;
import java.util.ArrayList;

import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IFolder;
import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IProjectDescription;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.Path;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.ui.IWorkbenchWindow;

import wesnoth_eclipse_plugin.Activator;
import wesnoth_eclipse_plugin.builder.WesnothProjectNature;
import wesnoth_eclipse_plugin.preferences.PreferenceConstants;
import wesnoth_eclipse_plugin.preferences.PreferenceInitializer;
import wesnoth_eclipse_plugin.wizards.ReplaceableParameter;
import wesnoth_eclipse_plugin.wizards.TemplateProvider;

public class WorkspaceUtils
{
	private static String	temporaryFolder_	= "";

	public static IProject getSelectedProject(IWorkbenchWindow window)
	{
		IStructuredSelection selection = getSelectedStructuredSelection(window);
		if (selection == null || !(selection.getFirstElement() instanceof IProject))
			return null;

		return (IProject) selection.getFirstElement();
	}

	public static IFolder getSelectedFolder(IWorkbenchWindow window)
	{
		IStructuredSelection selection = getSelectedStructuredSelection(window);
		if (selection == null || !(selection.getFirstElement() instanceof IFolder))
			return null;

		return (IFolder) selection.getFirstElement();
	}

	public static IFile getSelectedFile(IWorkbenchWindow window)
	{
		IStructuredSelection selection = getSelectedStructuredSelection(window);
		if (selection == null || !(selection.getFirstElement() instanceof IFile))
			return null;

		return (IFile) selection.getFirstElement();
	}

	public static IStructuredSelection getSelectedStructuredSelection(IWorkbenchWindow window)
	{
		if (window == null)
			return null;

		if (!(window.getSelectionService().getSelection() instanceof IStructuredSelection))
			return null;
		return (IStructuredSelection) window.getSelectionService().getSelection();
	}

	public static IProject getSelectedProject()
	{
		return getSelectedProject(WorkspaceUtils.getWorkbenchWindow());
	}

	public static IFolder getSelectedFolder()
	{
		return getSelectedFolder(WorkspaceUtils.getWorkbenchWindow());
	}

	public static IFile getSelectedFile()
	{
		return getSelectedFile(WorkspaceUtils.getWorkbenchWindow());
	}

	public static IStructuredSelection getSelectedStructuredSelection()
	{
		return getSelectedStructuredSelection(WorkspaceUtils.getWorkbenchWindow());
	}

	/**
	 * Returns the first WorkbenchWindow available. This is not always the same
	 * with ActiveWorkbecnWindow
	 * @return
	 */
	public static IWorkbenchWindow getWorkbenchWindow()
	{
		if (Activator.getDefault().getWorkbench().getWorkbenchWindowCount() == 0)
			return null;
		return Activator.getDefault().getWorkbench().getWorkbenchWindows()[0];
	}

	/**
	 * Returns the temporary folder where the plugin can write resources
	 * @return
	 */
	public static String getTemporaryFolder()
	{
		if (temporaryFolder_.isEmpty())
		{
			temporaryFolder_ = System.getProperty("java.io.tmpdir") + Path.SEPARATOR + "wesnoth_plugin" + Path.SEPARATOR;

			File tmpFile = new File(temporaryFolder_);
			if (!tmpFile.exists())
				tmpFile.mkdirs();
		}
		return temporaryFolder_;
	}

	/**
	 * Returns the resource path relative to the user directory
	 * @param resource the resource to be computed
	 * @return
	 */
	public static String getPathRelativeToUserDir(IResource resource)
	{
		return PreferenceInitializer.getString(PreferenceConstants.P_WESNOTH_USER_DIR) + Path.SEPARATOR + "data/add-ons/" + resource.getProject().getName()
				+ Path.SEPARATOR + resource.getProjectRelativePath().toOSString();
	}

	public static void setupWorkspace()
	{
		// automatically import "WesnothUserDir/data/add-ons as a project
		// container
		String userDir = PreferenceInitializer.getString(PreferenceConstants.P_WESNOTH_USER_DIR);
		if (userDir.isEmpty() || !new File(userDir).exists())
		{
			GUIUtils.showMessageBox(WorkspaceUtils.getWorkbenchWindow(),
					"Please set all plugin's preferences before using it.");
			return;
		}

		try
		{
			IProject proj = ResourcesPlugin.getWorkspace().getRoot().getProject("User Addons");
			if (!proj.exists())
			{
				IProjectDescription description = ResourcesPlugin.getWorkspace().newProjectDescription("User Addons");

				// cleanup any strictly-project related files if any.
				if (new File(userDir + Path.SEPARATOR + "data/add-ons/.project").exists())
					new File(userDir + Path.SEPARATOR + "data/add-ons/.project").delete();

				if (new File(userDir + Path.SEPARATOR + "data/add-ons/.ignore").exists())
					new File(userDir + Path.SEPARATOR + "data/add-ons/.ignore").delete();

				description.setLocation(new Path(userDir + Path.SEPARATOR + "data/add-ons/"));
				proj.create(description, null);
				proj.open(null);

				// the nature isn't set on creation so the nature adds the builder aswell
				description.setNatureIds(new String[] { WesnothProjectNature.NATURE_ID });
				proj.setDescription(description, null);

				// add the build.xml file
				ArrayList<ReplaceableParameter> param = new ArrayList<ReplaceableParameter>();
				param.add(new ReplaceableParameter("$$project_name", "User Addons"));
				ResourceUtils.createFile(proj, "build.xml",
						TemplateProvider.getInstance().getProcessedTemplate("build_xml", param));


				// we need to skip the already created projects (if any) in the addons directory
				String skipList = "";
				for (IProject project : ResourcesPlugin.getWorkspace().getRoot().getProjects())
				{
					if (project.getName().equals("User Addons"))
						continue;
					skipList += (project.getLocation().toOSString() + "\n");
				}
				ResourceUtils.createFile(proj, ".ignore",skipList);
			}
			GUIUtils.showMessageBox(WorkspaceUtils.getWorkbenchWindow(),
					"Workspace was set up successfully.");
		} catch (Exception e)
		{
			e.printStackTrace();
		}
	}
}
