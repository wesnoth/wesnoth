/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.validation;

import org.eclipse.xtext.parsetree.CompositeNode;
import org.eclipse.xtext.parsetree.LeafNode;
import org.eclipse.xtext.parsetree.NodeUtil;
import org.eclipse.xtext.validation.Check;
import org.wesnoth.wML.WMLPackage;
import org.wesnoth.wML.WMLTag;

import wesnoth_eclipse_plugin.schema.SchemaParser;
import wesnoth_eclipse_plugin.schema.Tag;

public class WMLJavaValidator extends AbstractWMLJavaValidator
{
	@Check
	public void checkTagName(WMLTag tag)
	{
		CompositeNode node = NodeUtil.getNodeAdapter(tag).getParserNode();
		if (node != null)
		{
			LeafNode parentNode = (LeafNode)
						NodeUtil.findLeafNodeAtOffset(node.getParent(),
								node.getParent().getOffset() + 2);

			boolean found = false;
			String searchName = parentNode.getText();
			if (node.getParent().eContainer() == null) // root node
			{
				searchName = "root";
			}
			if (SchemaParser.getInstance().getTags().get(searchName) != null)
			{
				for(Tag childTag : SchemaParser.getInstance().getTags().get(searchName).
						getTagChildren())
				{
					if (childTag.getName().equals(tag.getName()))
					{
						found = true;
						break;
					}
				}
				if (found == false)
					warning("There is no such wml tag name.", WMLPackage.WML_TAG__NAME);
			}
		}
		if (!tag.getName().equals(tag.getEndName()))
			error("Incorrect closing tag name.", WMLPackage.WML_TAG__END_NAME);
	}
//	@Check
//	public void checkGreetingStartsWithCapital(Attributes attr) {
		//System.out.println("Bla: ");
//		for (String itm : attr.getName())
//		{
//			System.out.println(itm);
//		}
//		if (!Character.isUpperCase(attr.getName().charAt(0))) {
//			warning("Name should start with a capital", MyDslPackage.GREETING__NAME);
//		}
//	}

}
