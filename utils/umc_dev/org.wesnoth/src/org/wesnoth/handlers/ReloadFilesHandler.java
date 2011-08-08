/*******************************************************************************
 * Copyright (c) 2010 - 2011 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.handlers;

import java.util.Map.Entry;

import org.eclipse.core.commands.AbstractHandler;
import org.eclipse.core.commands.ExecutionEvent;
import org.eclipse.core.resources.IProject;

import org.wesnoth.Messages;
import org.wesnoth.projects.ProjectCache;
import org.wesnoth.projects.ProjectUtils;
import org.wesnoth.schema.SchemaParser;
import org.wesnoth.templates.TemplateProvider;
import org.wesnoth.utils.GUIUtils;

public class ReloadFilesHandler extends AbstractHandler
{
    @Override
    public Object execute( ExecutionEvent event )
    {
        SchemaParser.reloadSchemas( true );

        TemplateProvider.getInstance( ).loadTemplates( );

        // reload the cache only for already loaded files
        for( Entry< IProject, ProjectCache > cache: ProjectUtils
                .getProjectCaches( ).entrySet( ) ) {
            cache.getValue( ).readDefines( true );
        }

        GUIUtils.showInfoMessageBox( Messages.ReloadFilesHandler_0 );
        return null;
    }
}
