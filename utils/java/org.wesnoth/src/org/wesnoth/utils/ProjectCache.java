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
		properties_ = new DialogSettings("project");

		wesnothFile_ = new File(project.getLocation().toOSString()  +
							"/.wesnoth");
		definesFile_ = new File (PreprocessorUtils.getInstance().getTemporaryLocation(
				project.getFile("_main.cfg"))  + "/_MACROS_.cfg");

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

			if (properties_.getSection("configs") != null)
			{
				for(IDialogSettings config : properties_.getSection("configs").getSections())
				{
					if (config.getName().startsWith("config") == false)
						continue;

					ConfigFile tmp = new ConfigFile(config.get("filename"));
					tmp.setScenarioId(config.get("scenario_id"));
					tmp.setCampaignId(config.get("campaign_id"));

					for(IDialogSettings variable : config.getSection("variables").getSections())
					{
						if (variable.getName().startsWith("var") == false)
							continue;
						tmp.getVariables().add(
								new Variable(variable.get("name"),
										variable.get("location"),
										variable.getInt("offset")));
					}
					configFiles_.put(config.get("filename"), tmp);
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
			IDialogSettings configsSection = properties_.addNewSection("configs");
			for(ConfigFile config : configFiles_.values())
			{
				IDialogSettings configSection = configsSection.addNewSection("config" + configCnt);
				configSection.put("scenario_id", config.getScenarioId());
				configSection.put("campaign_id", config.getCampaignId());
				configSection.put("filename", config.getFilename());

				IDialogSettings variablesSection = configSection.addNewSection("variables");
				int varCnt = 0;
				for(Variable var : config.getVariables())
				{
					IDialogSettings varSection = variablesSection.addNewSection("var" + varCnt);
					varSection.put("name", var.getName());
					varSection.put("location", var.getLocation());
					varSection.put("offset", var.getOffset());

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
