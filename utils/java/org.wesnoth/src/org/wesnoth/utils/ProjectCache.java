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
import java.util.HashMap;
import java.util.Map;

import org.eclipse.core.resources.IProject;
import org.eclipse.jface.dialogs.DialogSettings;
import org.eclipse.jface.dialogs.IDialogSettings;
import org.wesnoth.Logger;
import org.wesnoth.preprocessor.Define;
import org.wesnoth.wml.core.ConfigFile;
import org.wesnoth.wml.core.Variable;

/**
 * A class that stores some project specific infos
 * for current session.
 * Some of the fields of this cache can be saved to disk
 *  @see ProjectCache#saveCache()
 */
public class ProjectCache
{
	private IProject associatedProject_;

	private long propertiesTimetamp_;
	private long definesTimetamp_;

	private DialogSettings properties_;

	private File wesnothFile_;
	private File definesFile_;

	private Map<String, ConfigFile> configFiles_;
	private Map<String, Define> defines_;

	public ProjectCache(IProject project)
	{
		configFiles_ = new HashMap<String, ConfigFile>();
		defines_ = new HashMap<String, Define>(0);
		propertiesTimetamp_ = 0;
		properties_ = new DialogSettings("project"); //$NON-NLS-1$

		wesnothFile_ = new File(project.getLocation().toOSString()  +
							"/.wesnoth"); //$NON-NLS-1$
		definesFile_ = new File (PreprocessorUtils.getInstance().getTemporaryLocation(
				project.getFile("_main.cfg"))  + "/_MACROS_.cfg"); //$NON-NLS-1$ //$NON-NLS-2$

		ResourceUtils.createWesnothFile(wesnothFile_.getAbsolutePath(), false);
		readProperties(true);
		readDefines(true);
	}

	/**
	 * Reads the properties from the file only if the
	 * file changed.
	 * @param force True to skip checking for modifications
	 */
	private void readProperties(boolean force)
	{
		if (force == false &&
			wesnothFile_.lastModified() <= propertiesTimetamp_)
			return;
		try
		{
			ResourceUtils.createWesnothFile(wesnothFile_.getAbsolutePath(), false);

			try
			{
				properties_.load(wesnothFile_.getAbsolutePath());
			}
			catch(ClassCastException ex)
			{
				// backward compatiblity
				// we already have an xml format used by Properties.
				// convert it to DialogSettings
				ResourceUtils.createWesnothFile(wesnothFile_.getAbsolutePath(), true);
				properties_.load(wesnothFile_.getAbsolutePath());
			}

			if (properties_.getSection("configs") != null) //$NON-NLS-1$
			{
				for(IDialogSettings config : properties_.getSection("configs").getSections()) //$NON-NLS-1$
				{
					if (config.getName().startsWith("config") == false) //$NON-NLS-1$
						continue;

					ConfigFile tmp = new ConfigFile(config.get("filename")); //$NON-NLS-1$
					tmp.setScenarioId(config.get("scenario_id")); //$NON-NLS-1$
					tmp.setCampaignId(config.get("campaign_id")); //$NON-NLS-1$

					for(IDialogSettings variable : config.getSection("variables").getSections()) //$NON-NLS-1$
					{
						if (variable.getName().startsWith("var") == false) //$NON-NLS-1$
							continue;
						tmp.getVariables().add(
								new Variable(variable.get("name"), //$NON-NLS-1$
										variable.get("location"), //$NON-NLS-1$
										variable.getInt("offset"))); //$NON-NLS-1$
					}
					configFiles_.put(config.get("filename"), tmp); //$NON-NLS-1$
				}
			}
			propertiesTimetamp_ = wesnothFile_.lastModified();
		}
		catch (Exception e)
		{
			Logger.getInstance().logException(e);
			propertiesTimetamp_ = 0; // force to re-read the file
		}
	}

	public IProject getAssociatedProject()
	{
		return associatedProject_;
	}

	/**
	 * Gets the properties store for the associated Project.
	 * If the store doesn't exist it will be created.
	 * This method ensures it will get the latest up-to-date '.wesnoth' file
	 * @return
	 */
	public DialogSettings getProperties()
	{
		readProperties(false);
		return properties_;
	}

	/**
	 * Gets the map with the Configs
	 * The key represent the filenames of the files
	 * and the value the scenarioId from that file
	 * @return
	 */
	public Map<String, ConfigFile> getConfigs()
	{
		return configFiles_;
	}

	/**
	 * Gets the Config by it's filename.
	 * If the Config doesn't exist it will be created
	 * @param name
	 * @return
	 */
	public ConfigFile getConfig(String name)
	{
		if (configFiles_.containsKey(name) == false)
			configFiles_.put(name, new ConfigFile(name));
		return configFiles_.get(name);
	}

	/**
	 * Saves the cache to disk.
	 * Saves:
	 * - properties
	 * - existing scenarios
	 * @return
	 */
	public boolean saveCache()
	{
		ResourceUtils.createWesnothFile(wesnothFile_.getAbsolutePath(), false);
		try
		{
			// save config files info
			int configCnt = 0;
			IDialogSettings configsSection = properties_.addNewSection("configs"); //$NON-NLS-1$
			for(ConfigFile config : configFiles_.values())
			{
				IDialogSettings configSection = configsSection.addNewSection("config" + configCnt); //$NON-NLS-1$
				configSection.put("scenario_id", config.getScenarioId()); //$NON-NLS-1$
				configSection.put("campaign_id", config.getCampaignId()); //$NON-NLS-1$
				configSection.put("filename", config.getFilename()); //$NON-NLS-1$

				IDialogSettings variablesSection = configSection.addNewSection("variables"); //$NON-NLS-1$
				int varCnt = 0;
				for(Variable var : config.getVariables())
				{
					IDialogSettings varSection = variablesSection.addNewSection("var" + varCnt); //$NON-NLS-1$
					varSection.put("name", var.getName()); //$NON-NLS-1$
					varSection.put("location", var.getLocation()); //$NON-NLS-1$
					varSection.put("offset", var.getOffset()); //$NON-NLS-1$

					++varCnt;
				}
				++configCnt;
			}

			// store properties
			properties_.save(wesnothFile_.getAbsolutePath());
			propertiesTimetamp_ = wesnothFile_.lastModified();
			return true;
		}
		catch (Exception e)
		{
			Logger.getInstance().logException(e);
			return false;
		}
	}

	/**
	 * Reads the defines files for this project
	 * @param force skip checking for last modified timestamp
	 */
	public void readDefines(boolean force)
	{
		if (force == false &&
			definesFile_.lastModified() <= definesTimetamp_)
			return;
		if (definesFile_.exists() == false)
			return;
		defines_ = Define.readDefines(definesFile_.getAbsolutePath());
	}

	public void setDefines(Map<String, Define> defines)
	{
		defines_ = defines;
	}

	/**
	 * Returns the defines associated with this project
	 * @return
	 */
	public Map<String, Define> getDefines()
	{
		return defines_;
	}
}
