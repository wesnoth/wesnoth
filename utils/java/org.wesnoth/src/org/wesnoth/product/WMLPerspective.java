/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
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

public class WMLPerspective implements IPerspectiveFactory
{
	private static final String WESNOTH_PROJ_EXPLORER_ID = "wesnoth_eclipse_plugin.views.projects";

	@Override
	public void createInitialLayout(IPageLayout layout)
	{
		 // Add "new wizards".
        layout.addNewWizardShortcut("org.eclipse.ui.wizards.new.folder");
        layout.addNewWizardShortcut("org.eclipse.ui.wizards.new.file");

        // Add "show views".
        layout.addShowViewShortcut(IPageLayout.ID_PROJECT_EXPLORER);
        layout.addShowViewShortcut(IPageLayout.ID_OUTLINE);
        layout.addShowViewShortcut(IPageLayout.ID_PROBLEM_VIEW);

        // Editors are placed for free.
        String editorArea = layout.getEditorArea();
        layout.setEditorAreaVisible(true);

        // Place navigator and outline to left of editor area.
        IFolderLayout left = layout.createFolder("left", IPageLayout.LEFT, (float) 0.26, editorArea);
        left.addView(WESNOTH_PROJ_EXPLORER_ID);

        IFolderLayout bottom = layout.createFolder("bottom", IPageLayout.BOTTOM, 0.76f, editorArea);
        bottom.addView(IPageLayout.ID_PROBLEM_VIEW);
        bottom.addView(IPageLayout.ID_PROGRESS_VIEW);

        IFolderLayout right = layout.createFolder("right", IPageLayout.RIGHT, 0.68f, editorArea);
        right.addView(IPageLayout.ID_OUTLINE);
	}
}
