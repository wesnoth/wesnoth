/*******************************************************************************
 * Copyright (c) 2011 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.builder;

import org.eclipse.emf.common.util.URI;
import org.eclipse.xtext.resource.impl.DefaultResourceServiceProvider;

import org.wesnoth.views.WesnothProjectsExplorer;

public class WMLResourceServiceProvider extends DefaultResourceServiceProvider
{
    @Override
    public boolean canHandle( URI uri )
    {
        if( uri.segmentCount( ) > 2
            && uri.segment( 2 ).equals(
                WesnothProjectsExplorer.CORE_LIBRARY_NAME_ENCODED ) ) {
            return false;
        }
        return super.canHandle( uri );
    }
}
