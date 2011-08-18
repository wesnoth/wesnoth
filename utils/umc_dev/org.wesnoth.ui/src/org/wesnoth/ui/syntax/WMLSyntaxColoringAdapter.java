/*******************************************************************************
 * Copyright (c) 2011 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.syntax;

import org.eclipse.emf.common.notify.impl.AdapterImpl;
import org.eclipse.emf.ecore.EObject;

/**
 * A simple WML adapter that holds a coloring id on the specified resource
 */
public class WMLSyntaxColoringAdapter extends AdapterImpl
{
    /**
     * A color id from the
     * {@link org.wesnoth.ui.syntax.WMLHighlightingConfiguration}
     */
    public String  ColorId;

    /**
     * The object to color
     */
    public EObject TargetEObject;

    /**
     * Creates a new {@link WMLSyntaxColoringAdapter}
     * 
     * @param id
     *        The color id defined in WMLHighlightingConfiguration
     * @param target
     *        The {@link EObject} to color
     */
    public WMLSyntaxColoringAdapter( String id, EObject target )
    {
        ColorId = id;
        TargetEObject = target;
    }
}
