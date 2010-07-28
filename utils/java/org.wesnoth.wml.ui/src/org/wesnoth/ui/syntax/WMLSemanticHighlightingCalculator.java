/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.syntax;

import java.util.Iterator;

import org.eclipse.emf.ecore.EObject;
import org.eclipse.emf.ecore.util.EcoreUtil;
import org.eclipse.xtext.parsetree.AbstractNode;
import org.eclipse.xtext.parsetree.CompositeNode;
import org.eclipse.xtext.parsetree.LeafNode;
import org.eclipse.xtext.parsetree.NodeAdapter;
import org.eclipse.xtext.parsetree.NodeUtil;
import org.eclipse.xtext.resource.XtextResource;
import org.eclipse.xtext.ui.editor.syntaxcoloring.IHighlightedPositionAcceptor;
import org.eclipse.xtext.xtext.ui.editor.syntaxcoloring.SemanticHighlightingCalculator;
import org.wesnoth.ui.WMLEditor;
import org.wesnoth.wML.WMLKey;
import org.wesnoth.wML.WMLMacro;
import org.wesnoth.wML.WMLPackage;
import org.wesnoth.wML.WMLTag;

public class WMLSemanticHighlightingCalculator extends SemanticHighlightingCalculator
{
	@Override
	public void provideHighlightingFor(XtextResource resource, IHighlightedPositionAcceptor acceptor)
	{
		if (resource == null)
			return;

		// we skip xtext's default acceptor since we want only ours
		if (!(acceptor instanceof WMLMergingHighlightedPositionAcceptor))
			return;

		Iterator<EObject> iter = EcoreUtil.getAllContents(resource, true);
		while (iter.hasNext())
		{
			EObject current = iter.next();
			if (skipNode(acceptor, current))
				continue;

			if (current instanceof WMLTag)
			{
				AbstractNode begin = getFirstFeatureNode(current, WMLPackage.Literals.WML_TAG__NAME.getName());
				highlightNode(begin, WMLHighlightingConfiguration.RULE_WML_TAG, acceptor);

				AbstractNode end = getFirstFeatureNode(current, WMLPackage.Literals.WML_TAG__END_NAME.getName());
				highlightNode(end, WMLHighlightingConfiguration.RULE_WML_TAG, acceptor);
			}
			else if (current instanceof WMLKey)
			{
				AbstractNode begin = getFirstFeatureNode(current, WMLPackage.Literals.WML_KEY__KEY_NAME.getName());
				highlightNode(begin, WMLHighlightingConfiguration.RULE_WML_KEY, acceptor);
			}
			else if (current instanceof WMLMacro)
			{
				AbstractNode begin = getFirstFeatureNode(current, WMLPackage.Literals.WML_MACRO__NAME.getName());
				highlightNode(begin, WMLHighlightingConfiguration.RULE_WML_MACRO, acceptor);

//				AbstractNode end = getFirstFeatureNode(current, WMLPackage.Literals.WML_MACRO__VALUE.getName());
//				highlightNode(end, WMLHighlightingConfiguration.RULE_WML_MACRO, acceptor);
			}
		}
	}

	/**
	 * We have this auxilliar function to know when to skip a node from being highlighted
	 * This is usually case of "highlighting start/end tags" and we don't want
	 * highlighting going over.
	 *
	 * @param acceptor
	 * @param node
	 * @return
	 */
	private boolean skipNode(IHighlightedPositionAcceptor acceptor, EObject node)
	{
		if (!(node instanceof WMLTag))
			return false;

		WMLTag tag = (WMLTag) node;
		WMLMergingHighlightedPositionAcceptor acceptor2 =
						(WMLMergingHighlightedPositionAcceptor) acceptor;
		WMLEditor editor = ((WMLHighlightingReconciler) acceptor2.getHighlightingReconciler()).getEditor();

		if (editor == null || editor.getCurrentHighlightedNode() == null)
			return false;
		return (tag.getName().equals(editor.getCurrentHighlightedNode().getText()));
	}

	private void highlightNode(AbstractNode node, String id, IHighlightedPositionAcceptor acceptor)
	{
		if (node == null)
			return;
		if (node instanceof LeafNode)
		{
			acceptor.addPosition(node.getOffset(), node.getLength(), id);
		}
		else
		{
			for (LeafNode leaf : node.getLeafNodes())
			{
				if (!leaf.isHidden())
				{
					acceptor.addPosition(leaf.getOffset(), leaf.getLength(), id);
				}
			}
		}
	}

	@Override
	public AbstractNode getFirstFeatureNode(EObject semantic, String feature)
	{
		NodeAdapter adapter = NodeUtil.getNodeAdapter(semantic);
		if (adapter != null)
		{
			CompositeNode node = adapter.getParserNode();
			if (node != null)
			{
				for (AbstractNode child : node.getChildren())
				{
					if (child instanceof LeafNode)
					{
						if (feature.equals(((LeafNode) child).getFeature()))
						{
							return child;
						}
					}
				}
			}
		}
		return null;
	}

}