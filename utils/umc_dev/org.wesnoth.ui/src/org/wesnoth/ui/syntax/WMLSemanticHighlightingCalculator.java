/*******************************************************************************
 * Copyright (c) 2010 - 2011 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.syntax;

import java.util.Iterator;
import java.util.List;

import org.eclipse.emf.ecore.EObject;
import org.eclipse.emf.ecore.EStructuralFeature;
import org.eclipse.emf.ecore.util.EcoreUtil;
import org.eclipse.xtext.nodemodel.ICompositeNode;
import org.eclipse.xtext.nodemodel.ILeafNode;
import org.eclipse.xtext.nodemodel.INode;
import org.eclipse.xtext.nodemodel.util.NodeModelUtils;
import org.eclipse.xtext.resource.XtextResource;
import org.eclipse.xtext.ui.editor.syntaxcoloring.IHighlightedPositionAcceptor;
import org.eclipse.xtext.ui.editor.syntaxcoloring.ISemanticHighlightingCalculator;
import org.wesnoth.ui.editor.WMLEditor;
import org.wesnoth.wml.WMLKey;
import org.wesnoth.wml.WMLMacroCall;
import org.wesnoth.wml.WMLMacroDefine;
import org.wesnoth.wml.WMLPreprocIF;
import org.wesnoth.wml.WMLTag;
import org.wesnoth.wml.WMLTextdomain;
import org.wesnoth.wml.WmlPackage;

public class WMLSemanticHighlightingCalculator implements ISemanticHighlightingCalculator
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

			INode begin=null, end=null;
			String beginId = null, endId = null;
			if (current instanceof WMLTag)
			{
				begin = getFirstFeatureNode( current, WmlPackage.Literals.WML_EXPRESSION__NAME );
				beginId = WMLHighlightingConfiguration.RULE_WML_TAG;

				end = getFirstFeatureNode( current, WmlPackage.Literals.WML_TAG__END_NAME );
				endId = WMLHighlightingConfiguration.RULE_WML_TAG;
			}
			else if (current instanceof WMLKey)
			{
				begin = getFirstFeatureNode( current, WmlPackage.Literals.WML_EXPRESSION__NAME );
				beginId = WMLHighlightingConfiguration.RULE_WML_KEY;
			}
			else if (current instanceof WMLMacroCall)
			{
				begin = getFirstFeatureNode( current, WmlPackage.Literals.WML_EXPRESSION__NAME );
				beginId = WMLHighlightingConfiguration.RULE_WML_MACRO_CALL;
			}
			else if (current instanceof WMLTextdomain)
			{
				begin = getFirstFeatureNode( current, WmlPackage.Literals.WML_EXPRESSION__NAME );
				beginId = WMLHighlightingConfiguration.RULE_WML_TEXTDOMAIN;
			}
			else if (current instanceof WMLPreprocIF)
			{
				begin = getFirstFeatureNode( current, WmlPackage.Literals.WML_EXPRESSION__NAME );
				beginId = WMLHighlightingConfiguration.RULE_WML_IF;

				end = getFirstFeatureNode( current, WmlPackage.Literals.WML_EXPRESSION__NAME );
				endId = WMLHighlightingConfiguration.RULE_WML_IF;
			}
			else if (current instanceof WMLMacroDefine)
			{
				begin = getFirstFeatureNode( current, WmlPackage.Literals.WML_EXPRESSION__NAME );
				beginId = WMLHighlightingConfiguration.RULE_WML_MACRO_DEFINE;

				end = getFirstFeatureNode( current, WmlPackage.Literals.WML_MACRO_DEFINE__END_NAME );
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

	private void highlightNode(INode node, String id, IHighlightedPositionAcceptor acceptor)
	{
		if (node == null || id == null)
			return;

		if (node instanceof ILeafNode)
		{
			acceptor.addPosition(node.getOffset(), node.getLength(), id);
		}
		else
		{
			for (ILeafNode leaf : node.getLeafNodes())
			{
				if (!leaf.isHidden())
				{
					acceptor.addPosition(leaf.getOffset(), leaf.getLength(), id);
				}
			}
		}
	}

	public INode getFirstFeatureNode(EObject semantic, EStructuralFeature feature)
	{
		ICompositeNode node = NodeModelUtils.findActualNodeFor( semantic );
		if (node == null)
		    return null;

		for (INode child : node.getChildren())
		{
			if ( child instanceof ILeafNode == false)
			    continue;

		    List<INode> features = NodeModelUtils.findNodesForFeature(
		            child.getGrammarElement( ), feature );
		    if ( !features.isEmpty( ) )
		        return features.get( 0 );
		}
		return null;
	}
}