/*******************************************************************************
 * Copyright (c) 2010 - 2011 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.product;

import org.eclipse.ui.application.IWorkbenchConfigurer;
import org.eclipse.ui.application.IWorkbenchWindowConfigurer;
import org.eclipse.ui.application.WorkbenchWindowAdvisor;
import org.eclipse.ui.ide.IDE;
import org.wesnoth.utils.WorkspaceUtils;

public class ApplicationWorkbenchAdvisor extends WorkbenchAdvisorHack {

	private static final String PERSPECTIVE_ID = "org.wesnoth.product.WMLPerspective"; //$NON-NLS-1$

    @Override
	public WorkbenchWindowAdvisor createWorkbenchWindowAdvisor(IWorkbenchWindowConfigurer configurer) {
        return new ApplicationWorkbenchWindowAdvisor(configurer);
    }

	@Override
	public String getInitialWindowPerspectiveId() {
		return PERSPECTIVE_ID;
	}

	@Override
	public void initialize(IWorkbenchConfigurer configurer)
	{
		super.initialize(configurer);
		configurer.setSaveAndRestore(true);
	}

	@Override
	public void preStartup()
	{
		IDE.registerAdapters();
	}

	@Override
	public void postStartup()
	{
	    if (WorkspaceUtils.checkConditions(false) == false)
        {
            WorkspaceUtils.setupWorkspace(true);
        }
	}
}
