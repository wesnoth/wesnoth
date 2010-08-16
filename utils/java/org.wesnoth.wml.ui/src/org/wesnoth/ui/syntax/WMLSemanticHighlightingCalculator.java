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
import org.wesnoth.ui.editor.WMLEditor;
import org.wesnoth.wML.WMLKey;
import org.wesnoth.wML.WMLMacroCall;
import org.wesnoth.wML.WMLMacroDefine;
import org.wesnoth.wML.WMLPackage;
import org.wesnoth.wML.WMLPreprocIF;
import org.wesnoth.wML.WMLTag;
import org.wesnoth.wML.WMLTextdomain;

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

			AbstractNode begin=null, end=null;
			String beginId = null, endId = null;
			if (current instanceof WMLTag)
			{
				begin = getFirstFeatureNode(current, WMLPackage.Literals.WML_TAG__NAME.getName());
				beginId = WMLHighlightingConfiguration.RULE_WML_TAG;

				end = getFirstFeatureNode(current, WMLPackage.Literals.WML_TAG__END_NAME.getName());
				endId = WMLHighlightingConfiguration.RULE_WML_TAG;
			}
			else if (current instanceof WMLKey)
			{
				begin = getFirstFeatureNode(current, WMLPackage.Literals.WML_KEY__NAME.getName());
				beginId = WMLHighlightingConfiguration.RULE_WML_KEY;
			}
			else if (current instanceof WMLMacroCall)
			{
				begin = getFirstFeatureNode(current, WMLPackage.Literals.WML_MACRO_CALL__NAME.getName());
				beginId = WMLHighlightingConfiguration.RULE_WML_MACRO_CALL;
			}
			else if (current instanceof WMLTextdomain)
			{
				begin = getFirstFeatureNode(current, WMLPackage.Literals.WML_TEXTDOMAIN__NAME.getName());
				beginId = WMLHighlightingConfiguration.RULE_WML_TEXTDOMAIN;
			}
			else if (current instanceof WMLPreprocIF)
			{
				begin = getFirstFeatureNode(current, WMLPackage.Literals.WML_PREPROC_IF__NAME.getName());
				beginId = WMLHighlightingConfiguration.RULE_WML_IF;

				end = getFirstFeatureNode(current, WMLPackage.Literals.WML_PREPROC_IF__END_NAME.getName());
				endId = WMLHighlightingConfiguration.RULE_WML_IF;
			}
			else if (current instanceof WMLMacroDefine)
			{
				begin = getFirstFeatureNode(current, WMLPackage.Literals.WML_MACRO_DEFINE__NAME.getName());
				beginId = WMLHighlightingConfiguration.RULE_WML_MACRO_DEFINE;

				end = getFirstFeatureNode(current, WMLPackage.Literals.WML_MACRO_DEFINE__END_NAME.getName());
				endId = WMLHighlightingConfiguration.RULE_WML_MACRO_DEFINE;
			}

			highlightNode(begin, beginId, acceptor);
			highlightNode(end, endId, acceptor);
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
		WMLEditor editor = ((WMLHighlightingReconciler)
				((WMLMergingHighlightedPositionAcceptor) acceptor).
				getHighlightingReconciler()).getEditor();

		if (editor == null || editor.getCurrentHighlightedNode() == null)
			return false;
		return (tag.getName().equals(editor.getCurrentHighlightedNode().getText()));
	}

	private void highlightNode(AbstractNode node, String id, IHighlightedPositionAcceptor acceptor)
	{
		if (node == null || id == null)
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