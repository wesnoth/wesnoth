/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.preferences;

import org.eclipse.core.runtime.preferences.AbstractPreferenceInitializer;
import org.eclipse.jface.preference.IPreferenceStore;

import wesnoth_eclipse_plugin.Activator;
import wesnoth_eclipse_plugin.Constants;

/**
 * Class used to initialize default preference values.
 */
public class Preferences extends AbstractPreferenceInitializer {

	@Override
	public void initializeDefaultPreferences() {
		IPreferenceStore store = Activator.getDefault().getPreferenceStore();
		store.setDefault(Constants.P_WESNOTH_EXEC_PATH, "");
		store.setDefault(Constants.P_WESNOTH_WORKING_DIR, "");
		store.setDefault(Constants.P_WESNOTH_USER_DIR, "");
		store.setDefault(Constants.P_WESNOTH_WMLTOOLS_DIR, "");
	}

	/**
 	* @return The preferences store of the plugin
 	*/
	public static IPreferenceStore getPreferences()
	{
		return Activator.getDefault().getPreferenceStore();
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
}
