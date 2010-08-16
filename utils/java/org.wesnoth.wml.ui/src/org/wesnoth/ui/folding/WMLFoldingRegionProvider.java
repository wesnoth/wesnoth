/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.folding;

import java.util.List;

import org.eclipse.core.runtime.Assert;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.jface.text.Position;
import org.eclipse.xtext.parsetree.CompositeNode;
import org.eclipse.xtext.ui.editor.folding.DefaultFoldingRegionProvider;
import org.eclipse.xtext.ui.editor.folding.IFoldingRegion;
import org.eclipse.xtext.ui.editor.model.IXtextDocument;
import org.wesnoth.wML.WMLKey;
import org.wesnoth.wML.WMLTextdomain;

public class WMLFoldingRegionProvider extends DefaultFoldingRegionProvider
{
	@Override
	protected void addFoldingRegions(IXtextDocument xtextDocument,
			EObject eObject, List<IFoldingRegion> foldingRegions)
	{
		super.addFoldingRegions(xtextDocument, eObject, foldingRegions);
	}
	@Override
	protected Position getPosition(IXtextDocument xtextDocument,
			CompositeNode compositeNode)
	{
		Assert.isNotNull(compositeNode, "parameter 'compositeNode' must not be null");
		if (compositeNode.getElement() instanceof WMLKey &&
			((WMLKey)compositeNode.getElement()).getEol().length() > 0)
			return null;
		else if (compositeNode.getElement() instanceof WMLTextdomain)
			return null;
		return super.getPosition(xtextDocument, compositeNode);
	}
}
