/*******************************************************************************
 * Copyright (c) 2010 - 2011 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.navigation;

import org.eclipse.core.resources.IFile;
import org.eclipse.jface.text.Region;
import org.eclipse.xtext.nodemodel.ICompositeNode;
import org.eclipse.xtext.nodemodel.ILeafNode;
import org.eclipse.xtext.nodemodel.util.NodeModelUtils;
import org.eclipse.xtext.resource.XtextResource;
import org.eclipse.xtext.ui.editor.hyperlinking.HyperlinkHelper;
import org.eclipse.xtext.ui.editor.hyperlinking.IHyperlinkAcceptor;
import org.wesnoth.Logger;
import org.wesnoth.installs.WesnothInstallsUtils;
import org.wesnoth.preferences.Preferences;
import org.wesnoth.preferences.Preferences.Paths;
import org.wesnoth.preprocessor.Define;
import org.wesnoth.projects.ProjectUtils;
import org.wesnoth.ui.Messages;
import org.wesnoth.ui.WMLUtil;
import org.wesnoth.wml.WMLMacroCall;


public class WMLHyperlinkHelper extends HyperlinkHelper
{
	@Override
	public void createHyperlinksByOffset(XtextResource resource, int offset,
			IHyperlinkAcceptor acceptor)
	{
		super.createHyperlinksByOffset(resource, offset, acceptor);
		//TODO: test this ( the root node is get correct)
		ICompositeNode rootNode = resource.getParseResult( ).getRootNode( );
		if (rootNode == null)
			return;

		ILeafNode node = NodeModelUtils.findLeafNodeAtOffset(rootNode, offset);
		if (node == null)
			return;

        IFile file = WMLUtil.getActiveEditorFile();
        if (file == null)
        {
            Logger.getInstance().logError(Messages.WMLHyperlinkHelper_0);
            return;
        }
        Paths paths = Preferences.getPaths( WesnothInstallsUtils.getInstallNameForResource( file ) );

		createMacroHyperlink( file, paths, node, acceptor, resource );
		ILeafNode prevNode = NodeModelUtils.findLeafNodeAtOffset(rootNode,
									node.getOffset() - 1);
		if(prevNode == null)
			return;

		createMapHyperlink( paths, prevNode, node, acceptor, resource );
	}

	/**
	 * Creates a hyperlink for opening the macro definition
	 * @param prevNode
	 * @param node
	 * @param acceptor
	 * @param resource
	  */
	private void createMacroHyperlink( IFile file, Paths paths, ILeafNode node,
			IHyperlinkAcceptor acceptor, XtextResource resource)
	{
		if (node.eContainer() == null ||
			(node.eContainer() instanceof ICompositeNode) == false)
			return;

		ICompositeNode container = (ICompositeNode)node.eContainer();
		if ((container.getElement() instanceof WMLMacroCall) == false)
			return;

		WMLMacroCall macro = (WMLMacroCall)container.getElement();

		// get the define for the macro
		Define define = ProjectUtils.getCacheForProject(file.getProject()).getDefines().get(macro.getName());
		if (define == null)
		{
			//TODO: handle macro include call - open folder?
			//Logger.getInstance().log("No macro with that name found.");
			return;
		}

		if (define.getLocation().isEmpty() == true)
			return;

		String filePath = define.getLocation().split(" ")[0]; //$NON-NLS-1$

		if (filePath.startsWith("~")) { // user addon relative location //$NON-NLS-1$

			filePath = filePath.replaceFirst("~", //$NON-NLS-1$
			        paths.getUserDir( ).replace('\\', '/') + "/data/"); //$NON-NLS-1$
		}
		else if (filePath.startsWith("core")) { // data/core relative location //$NON-NLS-1$
			filePath = filePath.replaceFirst("core", //$NON-NLS-1$
					paths.getWorkingDir( ).replace('\\', '/') + "/data/core/"); //$NON-NLS-1$
		}

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
	private void createMapHyperlink( Paths paths, ILeafNode key, ILeafNode value,
			IHyperlinkAcceptor acceptor, XtextResource resource )
	{
		if (!(key.getText().equals("map_data"))) //$NON-NLS-1$
			return;

		// trim the " and the { (if any exist)
		String mapLocation = value.getText();
		if (mapLocation.startsWith("\"")) //$NON-NLS-1$
			mapLocation = mapLocation.substring(1, value.getLength() - 1);
		if (mapLocation.startsWith("{")) //$NON-NLS-1$
			mapLocation = mapLocation.substring(1, mapLocation.length( ) - 1);

		mapLocation = mapLocation.replaceFirst("~", //$NON-NLS-1$
				paths.getUserDir( ).replace('\\','/') + "/data/"); //$NON-NLS-1$

		MapOpenerHyperlink hyperlink = new MapOpenerHyperlink();
		hyperlink.setHyperlinkRegion(new Region(value.getOffset(), value.getLength()));
		hyperlink.setLocation(mapLocation);
		acceptor.accept(hyperlink);
	}
}
