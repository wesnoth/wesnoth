/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.navigation;

import org.eclipse.emf.ecore.util.EcoreUtil;
import org.eclipse.jface.text.Region;
import org.eclipse.xtext.parsetree.CompositeNode;
import org.eclipse.xtext.parsetree.LeafNode;
import org.eclipse.xtext.parsetree.NodeUtil;
import org.eclipse.xtext.resource.XtextResource;
import org.eclipse.xtext.ui.editor.hyperlinking.HyperlinkHelper;
import org.eclipse.xtext.ui.editor.hyperlinking.IHyperlinkAcceptor;
import org.wesnoth.ui.emf.ObjectStorageAdapter;

import wesnoth_eclipse_plugin.Constants;
import wesnoth_eclipse_plugin.preferences.Preferences;

public class WMLHyperlinkHelper extends HyperlinkHelper
{
	@Override
	public void createHyperlinksByOffset(XtextResource resource, int offset,
			IHyperlinkAcceptor acceptor)
	{
		super.createHyperlinksByOffset(resource, offset, acceptor);
		CompositeNode rootNode = NodeUtil.getRootNode(resource.getContents().get(0));
		if (rootNode == null)
			return;

		LeafNode node = (LeafNode)NodeUtil.findLeafNodeAtOffset(rootNode, offset);
		if (node == null)
			return;

		createMacroHyperlink(node, acceptor, resource);
		LeafNode prevNode = (LeafNode)NodeUtil.findLeafNodeAtOffset(rootNode,
									node.getOffset() - 1);
		if(prevNode == null)
			return;

		createMapHyperlink(prevNode, node, acceptor, resource);
	}

	/**
	 * Creates a hyperlink for opening the macro definition
	 * @param prevNode
	 * @param node
	 * @param acceptor
	 * @param resource
	  */
	private void createMacroHyperlink(LeafNode node,
			IHyperlinkAcceptor acceptor, XtextResource resource)
	{
		FileLocationOpenerHyperlink macroTarget = new FileLocationOpenerHyperlink();
		macroTarget.setFilePath("e:\\work\\gw\\data\\campaigns\\An_Orcish_Incursion\\_main.cfg");
		macroTarget.setLinenumber(25);
		acceptor.accept(macroTarget);
	}

	/**
	 * Creates a hyperlink for opening the map.
	 * @param key The key (must me 'map_data' in this case)
	 * @param value The value of key, that is, the location of the map
	 */
	private void createMapHyperlink(LeafNode key, LeafNode value,
			IHyperlinkAcceptor acceptor, XtextResource resource)
	{
		if (!(key.getText().equals("map_data")))
			return;

		// trim the " and the {
		String mapLocation = value.getText().substring(2, value.getLength() - 2);
		mapLocation = mapLocation.replace("~",
				Preferences.getString(Constants.P_WESNOTH_USER_DIR) + "/data/");

		ObjectStorageAdapter adapter = (ObjectStorageAdapter)EcoreUtil.getAdapter(value.eAdapters(),
				ObjectStorageAdapter.class);
		if (adapter == null)
		{
			adapter = new ObjectStorageAdapter(mapLocation);
			value.eAdapters().add(adapter);
		}
		else
			adapter.setObject(mapLocation);

		MapOpenerHyperlink hyperlink = new MapOpenerHyperlink();
		hyperlink.setHyperlinkRegion(new Region(value.getOffset(), value.getLength()));
		hyperlink.setLocation(mapLocation);
		acceptor.accept(hyperlink);
	}
}
