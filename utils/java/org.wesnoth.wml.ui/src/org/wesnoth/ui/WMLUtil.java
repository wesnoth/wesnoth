/**
 * @author Timotei Dolean
 *
 */
package org.wesnoth.ui;

import org.eclipse.emf.ecore.EObject;
import org.eclipse.xtext.parsetree.AbstractNode;
import org.eclipse.xtext.parsetree.CompositeNode;
import org.eclipse.xtext.parsetree.LeafNode;
import org.eclipse.xtext.parsetree.NodeUtil;

public class WMLUtil
{
	public static String debug(EObject root)
	{
		CompositeNode node = NodeUtil.getNode(root);
		Iterable<AbstractNode> contents = NodeUtil.getAllContents(node);
		StringBuffer text = new StringBuffer();
		for (AbstractNode abstractNode : contents)
		{
			if (abstractNode instanceof LeafNode)
			{
				System.out.println((((LeafNode) abstractNode).getText()));
				text.append(((LeafNode) abstractNode).getText());
			}
		}
		return text.toString();
	}
}
