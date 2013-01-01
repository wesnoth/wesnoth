/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.builder;

import org.eclipse.core.resources.ICommand;
import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IProjectDescription;
import org.eclipse.core.resources.IProjectNature;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.xtext.ui.XtextProjectHelper;

/**
 * The wesnoth nature
 */
public class WesnothProjectNature implements IProjectNature
{
    /**
     * The ID of the Wesnoth nature
     */
    public static final String ID_NATURE = "org.wesnoth.natures.wesnoth"; //$NON-NLS-1$

    private IProject           project_;

    /**
     * Creates a new {@link WesnothProjectNature}
     */
    public WesnothProjectNature( )
    {
        project_ = null;
    }

    @Override
    public void configure( ) throws CoreException
    {
        IProjectDescription desc = project_.getDescription( );
        ICommand[] commands = desc.getBuildSpec( );

        boolean wesnothConfigured = false;
        boolean xtextConfigured = false;
        int configured = 0;
        for( int i = 0; i < commands.length; ++i ) {
            if( commands[i].getBuilderName( ).equals(
                WesnothProjectBuilder.ID_BUIILDER ) ) {
                wesnothConfigured = true;
                configured++;
            }
            if( commands[i].getBuilderName( ).equals(
                XtextProjectHelper.BUILDER_ID ) ) {
                xtextConfigured = true;
                configured++;
            }
        }
        if( configured == 2 ) {
            return;
        }

        ICommand[] newCommands = new ICommand[commands.length
            + ( 2 - configured )];
        System.arraycopy( commands, 0, newCommands, 0, commands.length );
        if( wesnothConfigured == false ) {
            ICommand command = desc.newCommand( );
            command.setBuilderName( WesnothProjectBuilder.ID_BUIILDER );
            newCommands[newCommands.length - 1] = command;
        }
        if( xtextConfigured == false ) {
            ICommand command = desc.newCommand( );
            command.setBuilderName( XtextProjectHelper.BUILDER_ID );
            newCommands[newCommands.length - ( 2 - configured )] = command;
        }
        desc.setBuildSpec( newCommands );
        project_.setDescription( desc, null );
    }

    @Override
    public void deconfigure( ) throws CoreException
    {
        IProjectDescription description = getProject( ).getDescription( );
        ICommand[] commands = description.getBuildSpec( );
        for( int i = 0; i < commands.length; ++i ) {
            if( commands[i].getBuilderName( ).equals(
                WesnothProjectBuilder.ID_BUIILDER )
                || commands[i].getBuilderName( ).equals(
                    XtextProjectHelper.BUILDER_ID ) ) {
                ICommand[] newCommands = new ICommand[commands.length - 1];
                System.arraycopy( commands, 0, newCommands, 0, i );
                System.arraycopy( commands, i + 1, newCommands, i,
                    commands.length - i - 1 );
                description.setBuildSpec( newCommands );
                project_.setDescription( description, null );
            }
        }
    }

    @Override
    public IProject getProject( )
    {
        return project_;
    }

    @Override
    public void setProject( IProject project )
    {
        this.project_ = project;
    }

}
