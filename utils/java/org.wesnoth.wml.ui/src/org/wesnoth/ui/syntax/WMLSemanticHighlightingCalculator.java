/**
 * @author Timotei Dolean
 *
 */
package org.wesnoth.ui.syntax;

import org.wesnoth.wML.WMLEndTag;
import org.wesnoth.wML.WMLKey;
import org.wesnoth.wML.WMLPackage;
import org.wesnoth.wML.WMLStartTag;

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

public class WMLSemanticHighlightingCalculator extends SemanticHighlightingCalculator
{
	@Override
	public void provideHighlightingFor(XtextResource resource, IHighlightedPositionAcceptor acceptor)
	{
		if (resource == null)
			return;

		Iterator<EObject> iter = EcoreUtil.getAllContents(resource, true);
		while (iter.hasNext())
		{
			EObject current = iter.next();
			if (current instanceof WMLStartTag)
			{
				AbstractNode begin = getFirstFeatureNode(current, WMLPackage.Literals.WML_START_TAG__TAGNAME.getName());
				highlightNode(begin, WMLHighlightingConfiguration.RULE_WML_TAG, acceptor);
			}
			else if (current instanceof WMLEndTag)
			{
				AbstractNode begin = getFirstFeatureNode(current, WMLPackage.Literals.WML_END_TAG__TAGNAME.getName());
				highlightNode(begin, WMLHighlightingConfiguration.RULE_WML_TAG, acceptor);
			}
			else if (current instanceof WMLKey)
			{
				AbstractNode begin = getFirstFeatureNode(current, WMLPackage.Literals.WML_KEY__KEY_NAME.getName());
				highlightNode(begin, WMLHighlightingConfiguration.RULE_WML_KEY, acceptor);
			}
		}
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