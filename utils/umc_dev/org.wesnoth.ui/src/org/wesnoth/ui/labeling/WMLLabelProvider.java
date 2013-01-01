/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.labeling;

import com.google.inject.Inject;

import java.util.Locale;

import org.eclipse.emf.ecore.EClass;
import org.eclipse.emf.edit.ui.provider.AdapterFactoryLabelProvider;
import org.eclipse.xtext.ui.editor.model.XtextDocument;
import org.eclipse.xtext.ui.label.DefaultEObjectLabelProvider;

/**
 * Provides labels for a EObjects.
 * 
 * see
 * http://www.eclipse.org/Xtext/documentation/latest/xtext.html#labelProvider
 */
public class WMLLabelProvider extends DefaultEObjectLabelProvider
{
    /**
     * Creates a new {@link WMLLabelProvider}
     * 
     * @param delegate
     *        An {@link AdapterFactoryLabelProvider} instance
     */
    @Inject
    public WMLLabelProvider( AdapterFactoryLabelProvider delegate )
    {
        super( delegate );
    }

    @Override
    protected Object doGetImage( Object element )
    {
        if( element instanceof EClass ) {
            return ( ( EClass ) element ).getName( ).toLowerCase(
                Locale.ENGLISH ) + ".png"; //$NON-NLS-1$
        }
        else if( element instanceof String ) {
            return element;
        }
        return super.doGetImage( element );
    }

    @Override
    public String getText( Object element )
    {
        if( element instanceof XtextDocument ) {
            return "Document";
        }

        return super.getText( element );
    }
}
