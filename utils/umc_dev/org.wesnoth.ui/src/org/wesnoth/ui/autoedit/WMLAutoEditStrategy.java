/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.autoedit;

import org.eclipse.jface.text.IDocument;
import org.eclipse.xtext.ui.editor.autoedit.DefaultAutoEditStrategyProvider;

/**
 * The auto edit strategy for the WML Editor
 */
public class WMLAutoEditStrategy extends DefaultAutoEditStrategyProvider
{
    /**
     * Creates a new {@link WMLAutoEditStrategy}
     */
    public WMLAutoEditStrategy( )
    {
        super( );
    }

    @Override
    protected void configure( IEditStrategyAcceptor acceptor )
    {
        super.configure( acceptor );
        configureStringLiteral( acceptor );
        acceptor.accept( new ClosingEndTagAutoEditStrategy( ),
            IDocument.DEFAULT_CONTENT_TYPE );
    }
}
