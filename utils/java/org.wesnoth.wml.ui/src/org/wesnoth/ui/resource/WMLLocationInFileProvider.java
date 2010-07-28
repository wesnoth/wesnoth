/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.resource;

import org.eclipse.emf.ecore.EObject;
import org.eclipse.emf.ecore.EStructuralFeature;
import org.eclipse.xtext.Keyword;
import org.eclipse.xtext.resource.DefaultLocationInFileProvider;

public class WMLLocationInFileProvider extends DefaultLocationInFileProvider
{
	@Override
	protected boolean useKeyword(Keyword keyword, EObject context)
	{
		return super.useKeyword(keyword, context);
	}

	@Override
	protected EStructuralFeature getIdentifierFeature(EObject obj)
	{
		return super.getIdentifierFeature(obj);
	}
}
