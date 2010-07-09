package wesnoth_eclipse_plugin.preferences;

import org.eclipse.core.runtime.preferences.AbstractPreferenceInitializer;
import org.eclipse.jface.preference.IPreferenceStore;

import wesnoth_eclipse_plugin.Activator;
import wesnoth_eclipse_plugin.Constants;

/**
 * Class used to initialize default preference values.
 */
public class Preferences extends AbstractPreferenceInitializer {

	/*
	 * (non-Javadoc)
	 *
	 * @see org.eclipse.core.runtime.preferences.AbstractPreferenceInitializer#initializeDefaultPreferences()
	 */
	@Override
	public void initializeDefaultPreferences() {
		IPreferenceStore store = Activator.getDefault().getPreferenceStore();
		store.setDefault(Constants.P_WESNOTH_EXEC_PATH, "");
		store.setDefault(Constants.P_WESNOTH_WORKING_DIR, "");
		store.setDefault(Constants.P_WESNOTH_USER_DIR, "");
	}

	/**
 	* @return The preferences store of the plugin
 	*/
	public static IPreferenceStore getPreferences()
	{
		return Activator.getDefault().getPreferenceStore();
	}

	public static String getString(String prefName)
	{
		return getPreferences().getString(prefName);
	}
	public static int getInt(String prefName)
	{
		return getPreferences().getInt(prefName);
	}
}
