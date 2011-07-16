/*******************************************************************************
 * Copyright (c) 2010 - 2011 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui;

import org.eclipse.core.resources.IFile;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.xtext.nodemodel.ICompositeNode;
import org.eclipse.xtext.nodemodel.ILeafNode;
import org.eclipse.xtext.nodemodel.INode;
import org.eclipse.xtext.nodemodel.util.NodeModelUtils;
import org.eclipse.xtext.resource.EObjectAtOffsetHelper;
import org.eclipse.xtext.ui.editor.XtextEditor;
import org.eclipse.xtext.ui.editor.utils.EditorUtils;

public class WMLUtil
{
    private static EObjectAtOffsetHelper eObjectAtOffsetHelper_;

    public static EObjectAtOffsetHelper EObjectUtils(){
        if ( eObjectAtOffsetHelper_ == null ) {
            eObjectAtOffsetHelper_ = new EObjectAtOffsetHelper( );
        }

        return eObjectAtOffsetHelper_;
    }

	public static String debug(EObject root)
	{
		ICompositeNode node = NodeModelUtils.getNode(root);
		Iterable<INode> contents = node.getAsTreeIterable( );
		StringBuffer text = new StringBuffer();
		for (INode abstractNode : contents)
		{
			if (abstractNode instanceof ILeafNode)
			{
				System.out.println((((ILeafNode) abstractNode).getText()));
				text.append(((ILeafNode) abstractNode).getText());
			}
		}
		return text.toString();
	}

	/**
	 * Gets current edited file
	 * @return
	 */
	public static IFile getActiveEditorFile()
	{
		return getEditorFile( EditorUtils.getActiveXtextEditor() );
	}

	public static IFile getEditorFile( XtextEditor editor )
	{
	    if ( editor == null )
	        return null;
	    return ( IFile ) editor.getEditorInput( ).getAdapter( IFile.class );
	}
}
