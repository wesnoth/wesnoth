/*******************************************************************************
 * Copyright (c) 2010 - 2011 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.product;

import java.util.ArrayList;
import java.util.List;

import org.eclipse.jface.action.GroupMarker;
import org.eclipse.jface.action.ICoolBarManager;
import org.eclipse.jface.action.IMenuManager;
import org.eclipse.jface.action.MenuManager;
import org.eclipse.ui.IWorkbenchActionConstants;
import org.eclipse.ui.IWorkbenchWindow;
import org.eclipse.ui.actions.ActionFactory;
import org.eclipse.ui.actions.ActionFactory.IWorkbenchAction;
import org.eclipse.ui.application.ActionBarAdvisor;
import org.eclipse.ui.application.IActionBarConfigurer;
import org.wesnoth.Messages;

public class ApplicationActionBarAdvisor extends ActionBarAdvisor
{
	private IWorkbenchAction	aboutAction_;
	private IWorkbenchAction	quitAction_;
	private IWorkbenchAction	prefAction_;
	private IWorkbenchAction	newWizardDropDownAction_;

	private List<IWorkbenchAction> coolBarActions_;

	public ApplicationActionBarAdvisor(IActionBarConfigurer configurer)
	{
		super(configurer);

		coolBarActions_ = new ArrayList<IWorkbenchAction>();
	}

	@Override
	protected void makeActions(IWorkbenchWindow window)
	{
		// file menu
		newWizardDropDownAction_ = ActionFactory.NEW_WIZARD_DROP_DOWN.create(window);
		register(newWizardDropDownAction_);
		quitAction_ = ActionFactory.QUIT.create(window);
		register(quitAction_);

		// window menu
		prefAction_ = ActionFactory.PREFERENCES.create(window);
		register(prefAction_);

		// help menu
		aboutAction_ = ActionFactory.ABOUT.create(window);
		register(aboutAction_);

        coolBarActions_.add(register(ActionFactory.UNDO.create(window)));
        coolBarActions_.add(register(ActionFactory.REDO.create(window)));

        coolBarActions_.add(register(ActionFactory.SAVE.create(window)));
        coolBarActions_.add(register(ActionFactory.SAVE_AS.create(window)));
        coolBarActions_.add(register(ActionFactory.SAVE_ALL.create(window)));

        coolBarActions_.add(register(ActionFactory.COPY.create(window)));
        coolBarActions_.add(register(ActionFactory.CUT.create(window)));
        coolBarActions_.add(register(ActionFactory.PASTE.create(window)));

        coolBarActions_.add(register(ActionFactory.FIND.create(window)));

        coolBarActions_.add(register(ActionFactory.BACK.create(window)));
        coolBarActions_.add(register(ActionFactory.BACKWARD_HISTORY.create(window)));
        coolBarActions_.add(register(ActionFactory.FORWARD.create(window)));
        coolBarActions_.add(register(ActionFactory.FORWARD_HISTORY.create(window)));

        coolBarActions_.add(register(ActionFactory.PRINT.create(window)));
	}

	private IWorkbenchAction register(IWorkbenchAction action)
	{
	    super.register(action);
	    return action;
	}

	@Override
	protected void fillCoolBar(ICoolBarManager coolBar)
	{
	    IActionBarConfigurer config  = getActionBarConfigurer();
	    System.out.println(config.toString());
	    for(IWorkbenchAction action : coolBarActions_){
	        coolBar.add(action);
	    }
	}

	@Override
	protected void fillMenuBar(IMenuManager menuBar)
	{
		MenuManager fileMenu = new MenuManager(Messages.ApplicationActionBarAdvisor_0, "wesnoth.file"); //$NON-NLS-1$
		fileMenu.add(newWizardDropDownAction_);
		fileMenu.add(quitAction_);
		menuBar.add(fileMenu);

		MenuManager editMenu = new MenuManager(Messages.ApplicationActionBarAdvisor_2, IWorkbenchActionConstants.M_EDIT);
		menuBar.add(editMenu);

		// wesnoth
		menuBar.add(new GroupMarker(IWorkbenchActionConstants.MB_ADDITIONS));

		MenuManager projectMenu = new MenuManager(Messages.ApplicationActionBarAdvisor_3, IWorkbenchActionConstants.M_PROJECT);
		menuBar.add(projectMenu);

		MenuManager windowMenu = new MenuManager(Messages.ApplicationActionBarAdvisor_4, IWorkbenchActionConstants.M_WINDOW);
		windowMenu.add(prefAction_);
		menuBar.add(windowMenu);

		MenuManager helpMenu = new MenuManager(Messages.ApplicationActionBarAdvisor_5, "help"); //$NON-NLS-1$
		helpMenu.add(aboutAction_);
		menuBar.add(helpMenu);
	}
}
