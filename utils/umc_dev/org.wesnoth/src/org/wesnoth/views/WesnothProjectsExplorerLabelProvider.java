package org.wesnoth.views;

import org.eclipse.core.resources.IContainer;
import org.eclipse.core.resources.IResource;
import org.eclipse.jface.viewers.LabelProvider;
import org.eclipse.swt.graphics.Image;

import org.wesnoth.WesnothPlugin;

/**
 * Label provider for the WesnothProjectsExplorer
 */
public class WesnothProjectsExplorerLabelProvider extends LabelProvider
{
    private static Image CORE_LIBRARY_IMAGE = null;

    @Override
    public String getText( Object element )
    {
        if( element instanceof IResource ) {
            return ( ( IResource ) element ).getName( );
        }
        return super.getText( element );
    }

    @Override
    public Image getImage( Object element )
    {
        if( element instanceof IContainer ) {
            IContainer container = ( ( IContainer ) element );

            if( container.getName( ).equals(
                WesnothProjectsExplorer.CORE_LIBRARY_NAME ) ) {
                return getCoreLibraryImage( );
            }
        }

        return super.getImage( element );
    }

    private Image getCoreLibraryImage( )
    {
        if( CORE_LIBRARY_IMAGE == null ) {
            CORE_LIBRARY_IMAGE = WesnothPlugin.getImageDescriptor(
                "icons/closed_book_16.png" ).createImage( );
        }

        return CORE_LIBRARY_IMAGE;
    }
}
