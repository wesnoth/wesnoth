/*******************************************************************************
 * Copyright (c) 2011 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.tests.grammar;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;

import org.eclipse.xtext.parser.IParseResult;
import org.junit.Ignore;
import org.wesnoth.tests.WMLTests;
import org.wesnoth.utils.StringUtils;

/**
 * This tests test the data/core directory and each campaign
 */
public class WMLFilesTests extends WMLTests
{
    protected String dataPath_ = "";

    @Override
    protected void setUp() throws Exception
    {
        super.setUp( );

        // get the wesnoth data path from the user
        dataPath_ = System.getProperty( "wesnothDataDir" );
        if ( StringUtils.isNullOrEmpty( dataPath_ ) || ! new File( dataPath_ ).exists( ) )
            throw new Exception( "Please set the wesnoth data dir before testing!." );
    }

    @Override
    protected void tearDown() throws Exception
    {
        super.tearDown( );
        dataPath_ = null;
    }

    public void testCoreFiles() throws FileNotFoundException
    {
        testPath( dataPath_  + "/core/" );
    }

    public void testCampaignAOI( ) throws FileNotFoundException
    {
        testPath( dataPath_ + "/campaigns/An_Orcish_Incursion/" );
    }

    public void testCampaignDW( ) throws FileNotFoundException
    {
        testPath( dataPath_ + "/campaigns/Dead_Water/" );
    }

    public void testCampaignDM( ) throws FileNotFoundException
    {
        testPath( dataPath_ + "/campaigns/Delfadors_Memoirs/" );
    }

    public void testCampaignDID( ) throws FileNotFoundException
    {
        testPath( dataPath_ + "/campaigns/Descent_Into_Darkness/" );
    }

    public void testCampaignEI( ) throws FileNotFoundException
    {
        testPath( dataPath_ + "/campaigns/Eastern_Invasion/" );
    }

    public void testCampaignHttT( ) throws FileNotFoundException
    {
        testPath( dataPath_ + "/campaigns/Heir_To_The_Throne/" );
    }

    public void testCampaignLoW( ) throws FileNotFoundException
    {
        testPath( dataPath_ + "/campaigns/Legend_of_Wesmere/" );
    }

    public void testCampaignLiberty( ) throws FileNotFoundException
    {
        testPath( dataPath_ + "/campaigns/Liberty/" );
    }

    public void testCampaignNR( ) throws FileNotFoundException
    {
        testPath( dataPath_ + "/campaigns/Northern_Rebirth/" );
    }

    public void testCampaignSoF( ) throws FileNotFoundException
    {
        testPath( dataPath_ + "/campaigns/Sceptre_of_Fire/" );
    }

    public void testCampaignSotBT( ) throws FileNotFoundException
    {
        testPath( dataPath_ + "/campaigns/Son_Of_The_Black_Eye/" );
    }

    public void testCampaignTest( ) throws FileNotFoundException
    {
        testPath( dataPath_ + "/campaigns/test_campaign/" );
    }

    public void testCampaignTHT( ) throws FileNotFoundException
    {
        testPath( dataPath_ + "/campaigns/The_Hammer_of_Thursagan/" );
    }

    public void testCampaignTRoW( ) throws FileNotFoundException
    {
        testPath( dataPath_ + "/campaigns/The_Rise_Of_Wesnoth/" );
    }

    public void testCampaignTSG( ) throws FileNotFoundException
    {
        testPath( dataPath_ + "/campaigns/The_South_Guard/" );
    }

    public void testCampaignTutorial( ) throws FileNotFoundException
    {
        testPath( dataPath_ + "/campaigns/tutorial/" );
    }

    public void testCampaignTB( ) throws FileNotFoundException
    {
        testPath( dataPath_ + "/campaigns/Two_Brothers/" );
    }

    public void testCampaignUtBS( ) throws FileNotFoundException
    {
        testPath( dataPath_ + "/campaigns/Under_the_Burning_Suns/" );
    }

    @Ignore
    public void testPath( String path )
    {
        File theFile = new File( path );

        if ( ! theFile.exists( ) ) {
            System.out.println( "Skipping non-existent path:" + path );
            return;
        }

        if ( theFile.isFile( ) )
            testFile( path );
        else {
            for ( File file : theFile.listFiles( ) ) {
                testPath( file.getAbsolutePath( ) );
            }
        }
    }

    @Ignore
    public void testFile( String path )
    {
        // just config files
        if ( ! path.endsWith( ".cfg" ) )
            return;

        System.out.print( "\nTesting file: " + path + "..." );

        IParseResult res;
        try {
            res = getParser( ).parse( new FileReader( path ) );
            assertEquals( false, res.hasSyntaxErrors( ) );
            System.out.print( " OK" );
        }
        catch ( FileNotFoundException e ) {
            e.printStackTrace();
            //should not reach here.
            assertEquals( true, false );
        }
    }
}
