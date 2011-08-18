/*******************************************************************************
 * Copyright (c) 2011 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.hover;

import org.eclipse.emf.ecore.EObject;
import org.eclipse.jface.text.IRegion;
import org.eclipse.jface.text.ITextViewer;
import org.eclipse.xtext.ui.editor.hover.html.DefaultEObjectHoverProvider;

/**
 * Class that prevents the Quick Hover Info to be presented to the user.
 * WMLDoc is enough for now
 */
public class WMLEObjectHoverProvider extends DefaultEObjectHoverProvider
{
    @Override
    public IInformationControlCreatorProvider getHoverInfo( EObject object,
        ITextViewer viewer, IRegion region )
    {
        return null;
    }
}
