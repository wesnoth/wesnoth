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
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Properties;

import org.eclipse.core.resources.IProject;

import wesnoth_eclipse_plugin.Logger;
import wesnoth_eclipse_plugin.preprocessor.Define;

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

	private Properties properties_;

	private File wesnothFile_;
	private File definesFile_;

	private Map<String, String> scenarios_;
	private List<Define> defines_;

	public ProjectCache(IProject project)
	{
		scenarios_ = new HashMap<String, String>();
		defines_ = new ArrayList<Define>();
		propertiesTimetamp_ = 0;
		properties_ = new Properties();

		wesnothFile_ = new File(project.getLocation().toOSString()  +
							"/.wesnoth");
		definesFile_ = new File (PreprocessorUtils.getTemporaryLocation(
				project.getFile("_main.cfg"))  + "/_MACROS_.cfg");

		ResourceUtils.createWesnothFile(wesnothFile_.getAbsolutePath());
		readProperties(true);
		readDefines(true);
	}

	public ProjectCache(IProject project, Properties defaults)
	{
		this(project);
		properties_ = new Properties(defaults);
		saveCache();
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
			if (wesnothFile_.exists() == false)
				ResourceUtils.createWesnothFile(wesnothFile_.getAbsolutePath());
			properties_.loadFromXML(new FileInputStream(wesnothFile_));
			propertiesTimetamp_ = wesnothFile_.lastModified();

			// parse scenario ids
			scenarios_.clear();
			String[] fileNames = properties_.getProperty("scen_fns", "").split(",");
			String[] scenarioIds = properties_.getProperty("scen_ids", "").split(",");
			if (fileNames.length == scenarioIds.length)
			{
				for(int index = 0; index < fileNames.length;index++)
				{
					if (scenarioIds[index].isEmpty())
						continue;
					scenarios_.put(fileNames[index], scenarioIds[index]);
				}
			}
			else
				Logger.getInstance().logError("incorrect scenarios data.!!");
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
	public Properties getProperties()
	{
		readProperties(false);
		return properties_;
	}

	/**
	 * Gets the map with the scenarios
	 * The key represent the filenames of the files
	 * and the value the scenarioId from that file
	 * @return
	 */
	public Map<String, String> getScenarios()
	{
		return scenarios_;
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
		if (wesnothFile_.exists() == false)
			ResourceUtils.createWesnothFile(wesnothFile_.getAbsolutePath());
		try
		{
			// store scenario ids
			StringBuilder fileNames = new StringBuilder(scenarios_.size());
			StringBuilder scenarioIds = new StringBuilder(scenarios_.size());
			for(Entry<String, String> scenario : scenarios_.entrySet())
			{
				if (fileNames.length() > 0)
				{
					fileNames.append(',');
					scenarioIds.append(',');
				}
				fileNames.append(scenario.getKey());
				scenarioIds.append(scenario.getValue());
			}

			properties_.setProperty("scen_fns", fileNames.toString());
			properties_.setProperty("scen_ids", scenarioIds.toString());

			// store properties
			properties_.storeToXML(new FileOutputStream(wesnothFile_), null);
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

	public void setDefines(List<Define> defines)
	{
		defines_ = defines;
	}

	/**
	 * Returns the defines associated with this project
	 * @return
	 */
	public List<Define> getDefines()
	{
		return defines_;
	}
}
