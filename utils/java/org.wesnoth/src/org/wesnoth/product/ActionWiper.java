/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.product;

import org.eclipse.swt.widgets.Display;
import org.eclipse.ui.IPerspectiveDescriptor;
import org.eclipse.ui.IPerspectiveListener;
import org.eclipse.ui.IStartup;
import org.eclipse.ui.IWorkbenchPage;
import org.eclipse.ui.IWorkbenchWindow;
import org.eclipse.ui.PlatformUI;

public class ActionWiper implements IStartup, IPerspectiveListener {

    private static final String[] ACTIONS_2_WIPE = new String[] {
            "org.eclipse.search.searchActionSet",
            "org.eclipse.debug.ui.breakpointActionSet",
            "org.eclipse.debug.ui.debugActionSet",
            "org.eclipse.debug.ui.launchActionSet",
            "org.eclipse.debug.ui.profileActionSet"
//            "org.eclipse.ui.edit.text.actionSet.presentation",
//            "org.eclipse.ui.edit.text.actionSet.openExternalFile",
//            "org.eclipse.ui.edit.text.actionSet.annotationNavigation",
//            "org.eclipse.ui.edit.text.actionSet.navigation",
//            "org.eclipse.ui.edit.text.actionSet.convertLineDelimitersTo",
            //"org.eclipse.update.ui.softwareUpdates"
            };

    public void earlyStartup() {
        IWorkbenchWindow[] windows = PlatformUI.getWorkbench()
                .getWorkbenchWindows();
        for (int i = 0; i < windows.length; i++) {
            IWorkbenchPage page = windows[i].getActivePage();
            if (page != null) {
                wipeActions(page);
            }
            windows[i].addPerspectiveListener(this);
        }
    }

    private void wipeActions(IWorkbenchPage page) {
        for (int i = 0; i < ACTIONS_2_WIPE.length; i++) {
            wipeAction(page, ACTIONS_2_WIPE[i]);
        }

    }

    private void wipeAction(final IWorkbenchPage page, final String actionsetId) {
        Display.getDefault().syncExec(new Runnable() {
            public void run() {
                page.hideActionSet(actionsetId);
            }
        });
    }

    public void perspectiveActivated(IWorkbenchPage page,
            IPerspectiveDescriptor perspective) {
        wipeActions(page);
    }

    public void perspectiveChanged(IWorkbenchPage page,
            IPerspectiveDescriptor perspective, String changeId) {
    }
}
