/*******************************************************************************
 * Copyright (c) 2010 - 2011 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.product;

import org.eclipse.ui.IFolderLayout;
import org.eclipse.ui.IPageLayout;
import org.eclipse.ui.IPerspectiveFactory;
import org.eclipse.ui.console.IConsoleConstants;
import org.wesnoth.navigator.WesnothProjectsExplorer;
import org.wesnoth.views.AddonsView;

public class WMLPerspective implements IPerspectiveFactory
{
	@Override
	public void createInitialLayout(IPageLayout layout)
	{
		 // Add "new wizards".
        layout.addNewWizardShortcut("org.eclipse.ui.wizards.new.folder"); //$NON-NLS-1$
        layout.addNewWizardShortcut("org.eclipse.ui.wizards.new.file"); //$NON-NLS-1$

        // Add "show views".
        layout.addShowViewShortcut(IPageLayout.ID_PROJECT_EXPLORER);
        layout.addShowViewShortcut(IPageLayout.ID_OUTLINE);
        layout.addShowViewShortcut(IPageLayout.ID_PROBLEM_VIEW);
        layout.addShowViewShortcut( AddonsView.ID_ADDONS_VIEW );

        // Editors are placed for free.
        String editorArea = layout.getEditorArea();
        layout.setEditorAreaVisible(true);

        // Place navigator and outline to left of editor area.
        IFolderLayout left = layout.createFolder("left", IPageLayout.LEFT, (float) 0.26, editorArea); //$NON-NLS-1$
        left.addView(WesnothProjectsExplorer.ID_PROJECTS_EXPLORER);

        IFolderLayout bottom = layout.createFolder("bottom", IPageLayout.BOTTOM, 0.76f, editorArea); //$NON-NLS-1$
        bottom.addView(IPageLayout.ID_PROBLEM_VIEW);
        bottom.addView(IPageLayout.ID_PROGRESS_VIEW);
        bottom.addView(IConsoleConstants.ID_CONSOLE_VIEW);
        bottom.addView( AddonsView.ID_ADDONS_VIEW );
        bottom.addView("org.eclipse.pde.runtime.LogView"); //$NON-NLS-1$

        IFolderLayout right = layout.createFolder("right", IPageLayout.RIGHT, 0.68f, editorArea); //$NON-NLS-1$
        right.addView(IPageLayout.ID_OUTLINE);
	}
}
