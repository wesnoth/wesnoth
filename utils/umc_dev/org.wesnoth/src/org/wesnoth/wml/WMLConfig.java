/*******************************************************************************
 * Copyright (c) 2010 - 2011 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.wml;

import java.io.Serializable;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;


/**
 * A class that stores WML config file specific information
 */
public class WMLConfig implements Serializable
{
    private static final long     serialVersionUID = - 4722231494864404935L;

    public String                 ScenarioId;
    /**
     * True if there was a [scenario] tag present in the file.
     * 
     * However The {@link WMLConfig#ScenarioId} may be null
     */
    public boolean                IsScenario;

    public String                 CampaignId;
    /**
     * True if there was a [campaign] tag present in the file.
     * 
     * However The {@link WMLConfig#CampaignId} may be null
     */
    public boolean                IsCampaign;

    private String                filename_;

    private Map< String, WMLTag > tags_;
    private Set< String >         events_;

    public WMLConfig( String filename )
    {
        filename_ = filename;
        tags_ = new HashMap< String, WMLTag >( );
        events_ = new HashSet< String >( );
    }

    public String getFilename( )
    {
        return filename_;
    }

    /**
     * Returns the parsed WML Tags from this config file
     * 
     * @return A list of Tags
     */
    public Map< String, WMLTag > getWMLTags( )
    {
        return tags_;
    }

    @Override
    public String toString( )
    {
        return filename_ + "; ScenarioId: "
            + ( ScenarioId == null ? "": ScenarioId ) + "; CampaignId: "
            + ( CampaignId == null ? "": CampaignId );
    }

    /**
     * Returns the list of the parsed event names
     * 
     * @return A set of event names
     */
    public Set< String > getEvents( )
    {
        return events_;
    }
}
