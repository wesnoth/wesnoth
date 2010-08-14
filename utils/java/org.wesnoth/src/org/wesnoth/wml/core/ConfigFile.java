/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.wml.core;

import java.util.ArrayList;
import java.util.List;

/**
 * A class that stores WML config file specific information
 */
public class ConfigFile
{
	private String filename_;
	private String scenarioId_;
	private boolean isScenario_;
	private String campaignId_;
	private boolean isCampaign_;
	private List<Variable> variables_;

	public ConfigFile(String filename)
	{
		variables_ = new ArrayList<Variable>();
		filename_ = filename;
	}

	public String getFilename()
	{
		return filename_;
	}

	public List<Variable> getVariables()
	{
		return variables_;
	}

	public String getScenarioId()
	{
		return scenarioId_;
	}
	public void setScenarioId(String id)
	{
		scenarioId_ = id;
	}

	public String getCampaignId()
	{
		return campaignId_;
	}

	public void setCampaignId(String campaignId)
	{
		campaignId_ = campaignId;
	}

	public void setIsScenario(boolean isScenario)
	{
		isScenario_ = isScenario;
	}
	/**
	 * Returns true if there was a [scenario] tag present in the file.
	 *
	 * However the {@link ConfigFile#getScenarioId()} may return nul
	 */
	public boolean isScenario()
	{
		return isScenario_;
	}

	public void setIsCampaign(boolean isCampaign)
	{
		isCampaign_ = isCampaign;
	}

	/**
	 * Returns true if there was a [campaign] tag present in the file.
	 * However the {@link ConfigFile#getCampaignId()} may return null
	 * @return
	 */
	public boolean isCampaign()
	{
		return isCampaign_;
	}
}