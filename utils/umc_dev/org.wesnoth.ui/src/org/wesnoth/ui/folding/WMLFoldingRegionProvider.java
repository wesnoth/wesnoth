/*******************************************************************************
 * Copyright (c) 2010 - 2011 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.folding;

import org.eclipse.emf.ecore.EObject;
import org.eclipse.xtext.ui.editor.folding.DefaultFoldingRegionProvider;
import org.wesnoth.wml.WMLKey;
import org.wesnoth.wml.WMLTextdomain;

public class WMLFoldingRegionProvider extends DefaultFoldingRegionProvider
{
    @Override
    protected boolean isHandled( EObject eObject )
    {
        if ( eObject instanceof WMLTextdomain )
            return false;
        else if ( eObject instanceof WMLKey ) {
            WMLKey key = ( WMLKey ) eObject;
            if ( !key.getEol( ).isEmpty( ) )
                return false;
        }

        return super.isHandled( eObject );
    }
}
