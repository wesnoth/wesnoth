/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.autoedit;

import org.eclipse.xtext.ui.editor.autoedit.DefaultAutoEditStrategy;

public class WMLAutoEditStrategy extends DefaultAutoEditStrategy
{
	public WMLAutoEditStrategy()
	{
		super();
	}

	@Override
	protected void configure(IEditStrategyAcceptor acceptor)
	{
		configureStringLiteral(acceptor);
	}
}
