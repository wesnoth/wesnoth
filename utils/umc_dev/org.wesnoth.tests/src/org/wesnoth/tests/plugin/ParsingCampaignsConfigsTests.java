/*******************************************************************************
 * Copyright (c) 2011 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.tests.plugin;

import java.io.File;
import java.io.FileNotFoundException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;

import org.junit.Ignore;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;
import org.junit.runners.Parameterized.Parameters;

import org.wesnoth.tests.WMLTests;
import org.wesnoth.utils.StringUtils;
import org.wesnoth.wml.SimpleWMLParser;
import org.wesnoth.wml.WMLConfig;

/**
 * The class tests the parsed campaigns config files
 */
@RunWith( value = Parameterized.class )
public class ParsingCampaignsConfigsTests extends WMLTests
{
    private String         campaignDir_;
    private String         campaignId;
    private List< String > scenarioIds_;

    private List< String > parsedCampaignIds_;
    private List< String > parsedScenarioIds_;

    public ParsingCampaignsConfigsTests( String[] data ) throws Exception
    {
        setUp( );

        campaignDir_ = data[0];
        campaignId = data[1];
        scenarioIds_ = new ArrayList< String >( );

        parsedCampaignIds_ = new ArrayList< String >( );
        parsedScenarioIds_ = new ArrayList< String >( );

        for( int i = data.length - 1; i > 1; --i ) {
            scenarioIds_.add( data[i] );
        }

        // gather all info
        testPath( dataPath_ + "/campaigns/" + campaignDir_ + "/" );
    }

    @Override
    @Ignore
    public void testFile( String path ) throws FileNotFoundException
    {
        // just config files
        if( ! path.endsWith( ".cfg" ) ) {
            return;
        }

        SimpleWMLParser wmlParser = new SimpleWMLParser( new File( path ),
            getParser( ) );
        wmlParser.parse( );
        WMLConfig config = wmlParser.getParsedConfig( );

        if( ! StringUtils.isNullOrEmpty( config.ScenarioId ) ) {
            parsedScenarioIds_.add( config.ScenarioId );
        }

        if( ! StringUtils.isNullOrEmpty( config.CampaignId ) ) {
            parsedCampaignIds_.add( config.CampaignId );
        }
    }

    @Test
    public void testCampaignId( )
    {
        assertTrue( parsedCampaignIds_.size( ) == 1 );
        assertEquals( campaignId, parsedCampaignIds_.get( 0 ) );
    }

    @Test
    public void testScenarioIds( )
    {
        assertEquals( scenarioIds_.size( ), parsedScenarioIds_.size( ) );

        parsedScenarioIds_.removeAll( scenarioIds_ );

        // no scenario should have remained in the list
        assertEquals( 0, parsedScenarioIds_.size( ) );
    }

    @Parameters
    public static Collection< Object[] > data( )
    {
        return Arrays.asList( new Object[][] {
            { new String[] { "An_Orcish_Incursion", "An_Orcish_Incursion",
                "01_Defend_the_Forest", "02_Assassins", "03_Wasteland",
                "04_Valley_of_Trolls", "05_Linaera_the_Quick",
                "06_A_Detour_through_the_Swamp", "07_Showdown" } },
            { new String[] { "Dead_Water", "Dead_Water", "01_Invasion",
                "02_Flight", "03_Wolf_Coast", "04_Slavers",
                "05_Tirigaz", "06_Uncharted_Islands", "07_Bilheld",
                "08_Talking_to_Tyegea", "09_The_Mage",
                "10_The_Flaming_Sword", "11_Getting_Help",
                "12_Revenge", "13_Epilogue" } } } );
    }
}
