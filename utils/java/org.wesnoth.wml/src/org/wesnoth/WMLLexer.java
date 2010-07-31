/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth;

import org.antlr.runtime.CharStream;
import org.wesnoth.parser.antlr.internal.InternalWMLLexer;

public class WMLLexer extends InternalWMLLexer
{
	public WMLLexer()
	{
		super();
	}

	public WMLLexer(CharStream input)
	{
        super(input);
    }

	@Override
	public void setCharStream(CharStream arg0)
	{
		super.setCharStream(arg0);
	}
}
