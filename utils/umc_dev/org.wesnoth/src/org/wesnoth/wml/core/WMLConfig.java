/*******************************************************************************
 * Copyright (c) 2010 - 2011 by Timotei Dolean <timotei21@gmail.com>
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
public class WMLConfig
{
	public String ScenarioId;
	/**
	 * True if there was a [scenario] tag present in the file.
	 *
	 * However The {@link WMLConfig#ScenarioId} may be null
	 */
	public boolean IsScenario;

	public String CampaignId;
	/**
	 * True if there was a [campaign] tag present in the file.
	 *
	 * However The {@link WMLConfig#CampaignId} may be null
	 */
	public boolean IsCampaign;

	private List<Variable> variables_;
	private String filename_;

	public WMLConfig(String filename)
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
}