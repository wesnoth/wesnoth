/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.product;

import org.eclipse.ui.IWorkbenchWindow;
import org.eclipse.ui.actions.ActionFactory;
import org.eclipse.ui.application.IActionBarConfigurer;

/**
 * A class that creates the action bar in the product
 */
public class WesnothActionBarAdvisor extends WorkbenchActionBuilder
{
    /**
     * Creates a new instance with the specified configurer
     * 
     * @param configurer
     *        An {@link IActionBarConfigurer} instance
     * 
     */
    public WesnothActionBarAdvisor( IActionBarConfigurer configurer )
    {
        super( configurer );
    }

    @Override
    protected void makeActions( IWorkbenchWindow window )
    {
        super.makeActions( window );

        // add dynamic help hooks
        register( ActionFactory.HELP_SEARCH.create( window ) );
        register( ActionFactory.DYNAMIC_HELP.create( window ) );
    }
}
