/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth;

import com.google.inject.Injector;

import org.wesnoth.schema.SchemaParser;

/**
 * Initialization support for running Xtext languages
 * without equinox extension registry
 */
public class WMLStandaloneSetup extends WMLStandaloneSetupGenerated
{
    private static WMLStandaloneSetup instance_;
    private static Injector           injector_;

    /**
     * Returns the instance of the WML standalone setup
     * 
     * @return Returns the instance of the WML standalone setup
     */
    public static WMLStandaloneSetup getInstance( )
    {
        if( instance_ == null ) {
            doSetup( );
        }

        return instance_;
    }

    /**
     * Returns the injector configured with the WML Module
     * 
     * @return Returns the injector configured with the WML Module
     */
    public static Injector getInjector( )
    {
        getInstance( );
        return injector_;
    }

    /**
     * Setups this setup
     */
    public static void doSetup( )
    {
        try {
            instance_ = WMLStandaloneSetup.class.newInstance( );
        } catch( InstantiationException e ) {
            Logger.getInstance( ).logException( e );
        } catch( IllegalAccessException e ) {
            Logger.getInstance( ).logException( e );
        }
        injector_ = instance_.createInjectorAndDoEMFRegistration( );

        SchemaParser.reloadSchemas( false );
    }
}
