/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin;

import java.io.File;

import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.ui.plugin.AbstractUIPlugin;
import org.osgi.framework.BundleContext;

import wesnoth_eclipse_plugin.preferences.Preferences;

/**
 * The activator class controls the plug-in life cycle
 */
public class Activator extends AbstractUIPlugin
{

	// The plug-in ID
	public static final String	PLUGIN_ID	= "Wesnoth_Eclipse_Plugin";

	// The shared instance
	private static Activator	plugin;

	/**
	 * The constructor
	 */
	public Activator() {
	}

	@Override
	public void start(BundleContext context) throws Exception
	{
		super.start(context);
		plugin = this;
		Logger.getInstance().startLogger();
		checkConditions();
	}

	@Override
	public void stop(BundleContext context) throws Exception
	{
		plugin = null;
		Logger.getInstance().stopLogger();
		super.stop(context);
	}

	/**
	 * Returns the shared instance
	 *
	 * @return the shared instance
	 */
	public static Activator getDefault()
	{
		return plugin;
	}

	/**
	 * Returns an image descriptor for the image file at the given plug-in
	 * relative path
	 *
	 * @param path the path
	 * @return the image descriptor
	 */
	public static ImageDescriptor getImageDescriptor(String path)
	{
		return imageDescriptorFromPlugin(PLUGIN_ID, path);
	}

	/**
	 * Returns the plugin's shell
	 *
	 * @return
	 */
	public static Shell getShell()
	{
		return plugin.getWorkbench().getDisplay().getActiveShell();
	}

	/**
	 * Checks if the user has set some needed preferences and if the workspace
	 * is setup (there exists the "User Addons" project)
	 */
	private static void checkConditions()
	{
		String execDir = Preferences.getString(Constants.P_WESNOTH_EXEC_PATH);
		String userDir = Preferences.getString(Constants.P_WESNOTH_USER_DIR);
		String wmltoolsDir = Preferences.getString(Constants.P_WESNOTH_WMLTOOLS_DIR);
		String workingDir = Preferences.getString(Constants.P_WESNOTH_WORKING_DIR);

		if (!validPath(execDir) || !validPath(userDir) ||
			!validPath(wmltoolsDir) || !validPath(workingDir))
		{
			Logger.getInstance().log("checkConditions: preferences not set",
					"Please set all plugin's preferences before using it.");
			return;
		}

		// check if workspace is setup - the "userdir addons" directory created
		// as a project
//		if (!ResourcesPlugin.getWorkspace().getRoot().getProject("User Addons").exists())
//		{
//			Logger.getInstance().log("checkConditions: workspace not setup",
//				"Please setup the workspace before using the plugin. Go to \"Wesnoth\" menu," +
//				" and then click on the \"Setup Workspace\" entry following the instructions on the screen.");
//			return;
//		}
	}

	/**
	 * Checks if the path is valid and the specified path's resource exists
	 * @param path the path to check
	 * @return
	 */
	private static boolean validPath(String path)
	{
		return !path.isEmpty() && new File(path).exists();
	}
}
