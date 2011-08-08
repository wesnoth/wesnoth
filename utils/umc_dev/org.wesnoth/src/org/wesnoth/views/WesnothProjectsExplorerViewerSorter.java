/*******************************************************************************
 * Copyright (c) 2011 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.views;

import java.text.Collator;

import org.eclipse.core.resources.IContainer;
import org.eclipse.jface.viewers.Viewer;
import org.eclipse.jface.viewers.ViewerSorter;

public class WesnothProjectsExplorerViewerSorter extends ViewerSorter
{
    public WesnothProjectsExplorerViewerSorter( )
    {
    }

    public WesnothProjectsExplorerViewerSorter( Collator collator )
    {
        super( collator );
    }

    @Override
    public int compare( Viewer viewer, Object e1, Object e2 )
    {
        // The core library container should be first everytime
        int result = super.compare( viewer, e1, e2 );

        if( result != 0
            && e1 instanceof IContainer
            && ( ( IContainer ) e1 ).getName( ).equals(
                WesnothProjectsExplorer.CORE_LIBRARY_NAME ) ) {
            return - 1;
        }

        if( result != 0
            && e2 instanceof IContainer
            && ( ( IContainer ) e2 ).getName( ).equals(
                WesnothProjectsExplorer.CORE_LIBRARY_NAME ) ) {
            return 1;
        }

        return result;
    }
}
