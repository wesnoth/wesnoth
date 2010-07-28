/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.outline;

import org.eclipse.xtext.ui.editor.outline.transformer.AbstractDeclarativeSemanticModelTransformer;

/**
 * customization of the default outline structure
 *
 */
public class WMLTransformer extends AbstractDeclarativeSemanticModelTransformer
{
	//	/**
	//	 * This method will be called by naming convention:
	//	 * - method name must be createNode
	//	 * - first param: subclass of EObject
	//	 * - second param: ContentOutlineNode
	//	 */
	//	public ContentOutlineNode createNode(
	//			WMLStartTag semanticNode, ContentOutlineNode parentNode)
	//	{
	//		ContentOutlineNode node = super.newOutlineNode(semanticNode, parentNode);
	//		node.setLabel("special " + node.getLabel());
	//		return node;
	//	}

	//	public ContentOutlineNode createNode(
	//			Property semanticNode, ContentOutlineNode parentNode)
	//	{
	//		ContentOutlineNode node = super.newOutlineNode(semanticNode, parentNode);
	//		node.setLabel("pimped " + node.getLabel());
	//		return node;
	//	}
	//
	//	/**
	//	 * This method will be called by naming convention:
	//	 * - method name must be getChildren
	//	 * - first param: subclass of EObject
	//	 */
	//	public List<EObject> getChildren(WMLTag attribute)
	//	{
	//		return attribute.eContents();
	//	}

	//	public boolean consumeNode(WMLKey key)
	//	{
	//		return false;
	//	}
}
