/*******************************************************************************
 * Copyright (c) 2010 - 2011 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.preferences;

import org.eclipse.core.runtime.preferences.AbstractPreferenceInitializer;
import org.eclipse.jface.preference.IPreferenceStore;
import org.wesnoth.Constants;
import org.wesnoth.WesnothPlugin;
import org.wesnoth.utils.StringUtils;

/**
 * Class used to initialize default preference values.
 */
public class Preferences extends AbstractPreferenceInitializer
{
	@Override
	public void initializeDefaultPreferences()
	{
		IPreferenceStore store = WesnothPlugin.getDefault().getPreferenceStore();
		// general settings
		store.setDefault(Constants.P_WESNOTH_EXEC_PATH, ""); //$NON-NLS-1$
		store.setDefault(Constants.P_WESNOTH_WORKING_DIR, ""); //$NON-NLS-1$
		store.setDefault(Constants.P_WESNOTH_USER_DIR, ""); //$NON-NLS-1$
		store.setDefault(Constants.P_WESNOTH_WMLTOOLS_DIR, ""); //$NON-NLS-1$
		store.setDefault(Constants.P_PYTHON_PATH, ""); //$NON-NLS-1$

		// wml tools
		store.setDefault(Constants.P_WMLINDENT_VERBOSE, true);
		store.setDefault(Constants.P_WMLINDENT_DRYRUN, true);

		store.setDefault(Constants.P_WMLLINT_DRYRUN, true);
		store.setDefault(Constants.P_WMLLINT_SPELL_CHECK, false);
		store.setDefault(Constants.P_WMLLINT_VERBOSE_LEVEL, 0);

		store.setDefault(Constants.P_WMLSCOPE_VERBOSE_LEVEL, 0);
		store.setDefault(Constants.P_WMLSCOPE_COLLISIONS, false);

		// upload manager
		store.setDefault(Constants.P_WAU_PASSWORD, ""); //$NON-NLS-1$
		store.setDefault(Constants.P_WAU_VERBOSE, false);
		store.setDefault(Constants.P_WAU_ADDRESS, "add-ons.wesnoth.org"); //$NON-NLS-1$
		store.setDefault(Constants.P_WAU_PORT, 15002);

		// advanced
		store.setDefault(Constants.P_ADV_NO_TERRAIN_GFX, true);

		// installs
		store.setDefault(Constants.P_INST_DEFAULT_INSTALL, ""); // $NON-NLS-1$
		store.setDefault(Constants.P_INST_INSTALL_LIST, ""); // $NON-NLS-1$
	}

	/**
 	* @return The preferences store of the plugin
 	*/
	public static IPreferenceStore getPreferences()
	{
		return WesnothPlugin.getDefault().getPreferenceStore();
	}

	/**
	 * Returns the contents of the specified preference as a string
	 * or empty string ("") if there is no such preference set
	 */
	public static String getString(String prefName)
	{
		return getPreferences().getString(prefName);
	}

	/**
	 * Returns the contents of the specified preference as an int
	 * or zero (0) if there is no such preference set
	 */
	public static int getInt(String prefName)
	{
		return getPreferences().getInt(prefName);
	}
	/**
	 * Returns the contents of the specified preference as a boolean
	 * or false if there is no such preference set
	 */
	public static boolean getBool(String prefName)
	{
		return getPreferences().getBoolean(prefName);
	}

	/**
     * Gets the install preference prefix for the default Installation
     * found in the @see {@link Constants#P_INST_DEFAULT_INSTALL} <b
     * @param installName The name of the install
     */
	public static String getInstallPrefix()
	{
	    return getInstallPrefix(getString(Constants.P_INST_DEFAULT_INSTALL));
	}

	/**
	 * Gets the install preference prefix for the specified install name
	 * @param installName The name of the install
	 */
	public static String getInstallPrefix(String installName)
	{
        if (StringUtils.isNullOrEmpty(installName) ||
            installName.equalsIgnoreCase("default"))
            return "";

        return "inst_" + installName + "_";
	}
}
