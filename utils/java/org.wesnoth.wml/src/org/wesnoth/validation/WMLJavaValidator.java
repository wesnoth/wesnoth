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
import org.eclipse.xtext.validation.CheckType;
import org.wesnoth.schema.SchemaParser;
import org.wesnoth.schema.Tag;
import org.wesnoth.wML.WMLKey;
import org.wesnoth.wML.WMLMacroCall;
import org.wesnoth.wML.WMLPackage;
import org.wesnoth.wML.WMLRoot;
import org.wesnoth.wML.WMLTag;


/**
 * This represents the validator for config files
 *
 * http://wiki.eclipse.org/Xtext/Documentation/Xtext_New_and_Noteworthy#Different_validation_hooks
 * CheckType:
 * 1. during editing with a delay of 500ms, only FAST is passed
 * 2. on save NORMAL is passed
 * 3. an action, which can be optionally generated for you DSL, explicitely evaluates EXPENSIVE constraints
 */
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

	@Check(CheckType.EXPENSIVE)
	public void checkExpensiveKeyValue(WMLKey key)
	{
		//TODO: add regex checking here
		System.out.println("triggered wmlkey expensive validation.");
	}

	@Check(CheckType.NORMAL)
	public void checkNormalWMLRootCardinality(WMLRoot root)
	{
		//TODO: add check for subtags cardinality
	}

	@Check(CheckType.NORMAL)
	public void checkNormalWMLTagCardinality(WMLTag tag)
	{
		//TODO: add check for subtags/subkeys cardinality
	}

	@Check(CheckType.FAST)
	public void checkNormalWMLMacroExistance(WMLMacroCall call)
	{
		//TODO: add check for macro existance - by name
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
