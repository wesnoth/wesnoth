/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.action;

import org.eclipse.jface.action.IAction;

import org.wesnoth.utils.WMLTools;
import org.wesnoth.utils.WMLTools.Tools;

/**
 * Runs WMLIndent on selection
 */
public class RunWMLIndentOnSelection extends ObjectActionDelegate
{
    @Override
    public void run( IAction action )
    {
        WMLTools.runWMLToolAsWorkspaceJob( Tools.WMLINDENT, null );
    }
}
