/*******************************************************************************
 * Copyright (c) 2010 - 2011 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.validation;

import org.eclipse.core.resources.IResource;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.Path;
import org.eclipse.emf.ecore.resource.Resource;
import org.eclipse.xtext.nodemodel.ICompositeNode;
import org.eclipse.xtext.nodemodel.impl.LeafNode;
import org.eclipse.xtext.validation.Check;
import org.eclipse.xtext.validation.CheckType;
import org.wesnoth.Messages;
import org.wesnoth.installs.WesnothInstallsUtils;
import org.wesnoth.schema.SchemaParser;
import org.wesnoth.schema.Tag;
import org.wesnoth.wml.WMLKey;
import org.wesnoth.wml.WMLMacroCall;
import org.wesnoth.wml.WMLRoot;
import org.wesnoth.wml.WMLTag;
import org.wesnoth.wml.WmlPackage;

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
//	@Check(CheckType.FAST)
	public void checkFastTagName(WMLTag tag)
	{
		if (!tag.getName().equals(tag.getEndName()))
			warning( Messages.WMLJavaValidator_0, WmlPackage.Literals.WML_TAG__END_NAME);
	}

//	@Check(CheckType.NORMAL)
	public void checkNormalTagName(WMLTag tag)
	{
	    ICompositeNode node = NodeUtil.getNodeAdapter(tag).getParserNode();
		if (node != null)
		{
			LeafNode parentNode = (LeafNode)
						NodeUtil.findLeafNodeAtOffset(node.getParent(),
								node.getParent().getOffset() + 2);

			boolean found = false;
			String searchName = parentNode.getText();
			if (node.getParent().eContainer() == null) // root node
			{
				searchName = "root"; //$NON-NLS-1$
			}

	        Resource resource =  getCurrentObject( ).eResource( );
	        IResource file = ResourcesPlugin.getWorkspace( ).getRoot( ).
	            getFile( new Path( resource.getURI( ).toPlatformString( true ) ) );

			Tag schemaTag = SchemaParser.getInstance(
			        WesnothInstallsUtils.getInstallNameForResource( file ) ).getTags().get(searchName);

			if ( schemaTag != null )
			{
				for(Tag childTag : schemaTag.getTagChildren())
				{
					if (childTag.getName().equals(tag.getName()))
					{
						found = true;
						break;
					}
				}
				if (found == false)
					warning( Messages.WMLJavaValidator_1, WmlPackage.Literals.WML_EXPRESSION__NAME  );
			}
		}
	}

	@Check(CheckType.EXPENSIVE)
	public void checkExpensiveKeyValue(WMLKey key)
	{
		//TODO: add regex checking here
		System.out.println(Messages.WMLJavaValidator_2);
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
}
