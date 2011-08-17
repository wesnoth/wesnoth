package org.wesnoth.action;

import java.util.ArrayList;
import java.util.Iterator;

import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.NullProgressMonitor;
import org.eclipse.jface.action.IAction;

import org.wesnoth.Logger;
import org.wesnoth.templates.ReplaceableParameter;
import org.wesnoth.templates.TemplateProvider;
import org.wesnoth.utils.ResourceUtils;

/**
 * Regenerates the necessary building files
 */
public class RegenerateBuildFiles extends ObjectActionDelegate
{
    @SuppressWarnings( "rawtypes" )
    @Override
    public void run( IAction action )
    {
        if( structuredSelection_ == null ) {
            return;
        }

        for( Iterator it = structuredSelection_.iterator( ); it.hasNext( ); ) {
            Object element = it.next( );
            if( element instanceof IProject ) {
                ArrayList< ReplaceableParameter > param = new ArrayList< ReplaceableParameter >( );
                param
                    .add( new ReplaceableParameter(
                        "$$project_name", ( ( ( IProject ) element ).getName( ) ) ) ); //$NON-NLS-1$
                param
                    .add( new ReplaceableParameter(
                        "$$project_dir_name", ( ( IProject ) element ).getName( ) ) ); //$NON-NLS-1$
                ResourceUtils.createFile( ( IProject ) element,
                    "build.xml", //$NON-NLS-1$
                    TemplateProvider.getInstance( ).getProcessedTemplate(
                        "build_xml", param ), true ); //$NON-NLS-1$
                try {
                    ( ( IProject ) element ).refreshLocal( IResource.DEPTH_ONE,
                        new NullProgressMonitor( ) );
                } catch( CoreException e ) {
                    Logger.getInstance( ).logException( e );
                }
            }
        }
    }
}
