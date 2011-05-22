/*******************************************************************************
 * Copyright (c) 2011 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.preferences;

import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.ui.IWorkbench;
import org.wesnoth.WesnothPlugin;

public class WesnothInstallsPage extends AbstractPreferencePage
{
    public WesnothInstallsPage()
    {
        setPreferenceStore(WesnothPlugin.getDefault().getPreferenceStore());
        setTitle("Wesnoth Installs Preferences");
    }

    @Override
    public void init(IWorkbench workbench)
    {
    }

    @Override
    protected Control createContents(Composite parent)
    {
        return null;
    }

    @Override
    protected void createFieldEditors()
    {

    }
}
