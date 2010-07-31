/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth;

import org.eclipse.xtext.parser.antlr.Lexer;

/**
 * Use this class to register components to be used at runtime / without the Equinox extension registry.
 */
public class WMLRuntimeModule extends org.wesnoth.AbstractWMLRuntimeModule {
	@Override
	public Class<? extends Lexer> bindLexer()
	{
		return WMLLexer.class;
	}
}
