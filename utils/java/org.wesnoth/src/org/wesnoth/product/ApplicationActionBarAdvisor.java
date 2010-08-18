/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.product;

import org.eclipse.jface.action.GroupMarker;
import org.eclipse.jface.action.IMenuManager;
import org.eclipse.jface.action.MenuManager;
import org.eclipse.ui.IWorkbenchActionConstants;
import org.eclipse.ui.IWorkbenchWindow;
import org.eclipse.ui.actions.ActionFactory;
import org.eclipse.ui.actions.ActionFactory.IWorkbenchAction;
import org.eclipse.ui.application.ActionBarAdvisor;
import org.eclipse.ui.application.IActionBarConfigurer;

public class ApplicationActionBarAdvisor extends ActionBarAdvisor
{
    IWorkbenchAction aboutAction_;
    IWorkbenchAction quitAction_;
    IWorkbenchAction newAction_;

    public ApplicationActionBarAdvisor(IActionBarConfigurer configurer)
    {
        super(configurer);
    }
    @Override
	protected void makeActions(IWorkbenchWindow window)
    {
    	aboutAction_ = ActionFactory.ABOUT.create(window);
    	register(aboutAction_);

    	quitAction_ = ActionFactory.QUIT.create(window);
    	register(quitAction_);

    	newAction_ = ActionFactory.NEW.create(window);
    	register(newAction_);
    }

    @Override
	protected void fillMenuBar(IMenuManager menuBar)
    {
    	menuBar.add(newAction_);
    	menuBar.add(new GroupMarker(IWorkbenchActionConstants.MB_ADDITIONS));
    	MenuManager helpMenu = new MenuManager("&Help", "help");
    	helpMenu.add(aboutAction_);
    	menuBar.add(helpMenu);
    }
}
