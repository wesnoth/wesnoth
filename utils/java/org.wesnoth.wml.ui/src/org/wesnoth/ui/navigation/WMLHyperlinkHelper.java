/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.navigation;

import org.eclipse.core.resources.IFile;
import org.eclipse.emf.ecore.util.EcoreUtil;
import org.eclipse.jface.text.Region;
import org.eclipse.xtext.parsetree.CompositeNode;
import org.eclipse.xtext.parsetree.LeafNode;
import org.eclipse.xtext.parsetree.NodeUtil;
import org.eclipse.xtext.resource.XtextResource;
import org.eclipse.xtext.ui.editor.hyperlinking.HyperlinkHelper;
import org.eclipse.xtext.ui.editor.hyperlinking.IHyperlinkAcceptor;
import org.wesnoth.ui.WMLUtil;
import org.wesnoth.ui.emf.ObjectStorageAdapter;
import org.wesnoth.wML.WMLMacroCall;

import wesnoth_eclipse_plugin.Constants;
import wesnoth_eclipse_plugin.Logger;
import wesnoth_eclipse_plugin.preferences.Preferences;
import wesnoth_eclipse_plugin.preprocessor.Define;
import wesnoth_eclipse_plugin.utils.ProjectUtils;

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
		if (node.eContainer() == null ||
			(node.eContainer() instanceof CompositeNode) == false)
			return;

		CompositeNode container = (CompositeNode)node.eContainer();
		if ((container.getElement() instanceof WMLMacroCall) == false)
			return;

		WMLMacroCall macro = (WMLMacroCall)container.getElement();


		IFile file = WMLUtil.getActiveEditorFile();
		if (file == null)
		{
			Logger.getInstance().logError("FATAL! file is null (and it shouldn't)");
			return;
		}

		// get the define for the macro
		Define define = ProjectUtils.getCacheForProject(file.getProject()).getDefines().get(macro.getName());
		if (define == null)
		{
			//TODO: handle macro to file case
			//Logger.getInstance().log("No macro with that name found.");
			return;
		}

		if (define.getLocation().isEmpty() == true)
			return;

		String filePath = define.getLocation().split(" ")[0];

		if (filePath.startsWith("~")) // user addon relative location
			filePath = filePath.replaceFirst("~", Preferences.getString(Constants.P_WESNOTH_USER_DIR) + "/data/");
		else if (filePath.startsWith("core")) // data/core relative location
			filePath = filePath.replace("core", Preferences.getString(Constants.P_WESNOTH_WORKING_DIR) + "/data/core/");

		FileLocationOpenerHyperlink macroTarget = new FileLocationOpenerHyperlink();
		macroTarget.setHyperlinkRegion(new Region(container.getOffset(), container.getLength()));
		macroTarget.setFilePath(filePath);
		macroTarget.setLinenumber(define.getLineNum());
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
		mapLocation = mapLocation.replaceFirst("~",
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
