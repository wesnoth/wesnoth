/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.wizards.generator;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.Map.Entry;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.List;

import org.wesnoth.Messages;
import org.wesnoth.utils.GUIUtils;
import org.wesnoth.utils.ListUtils;
import org.wesnoth.utils.StringUtils;
import org.wesnoth.utils.WizardUtils;
import org.wesnoth.wizards.WizardPageTemplate;
import org.wesnoth.wml.WMLTag;

/**
 * A page that generates a new tag
 */
public class WizardGeneratorTagPage extends WizardPageTemplate
{
    private java.util.List< WMLTag >               tags_;
    private Map< String, java.util.List< String >> content_;
    private int                                    startIndex_, endIndex_;
    private Composite                              container_;
    private int                                    indent_;

    /**
     * Creates a new {@link WizardGeneratorTagPage}
     * 
     * @param tagName
     *        The name of the tag
     * @param tags
     *        The current tags list
     * @param startIndex
     *        The start index in the tags list
     * @param endIndex
     *        The end index in the tags list
     * @param indent
     *        The indent used when computing the WML Code
     */
    public WizardGeneratorTagPage( String tagName,
        java.util.List< WMLTag > tags, int startIndex, int endIndex,
        int indent )
    {
        super( "wizardPageTag" + startIndex ); //$NON-NLS-1$
        setTitle( tagName + Messages.WizardGeneratorPageTag_1 );
        // setDescription(String.format("page %d to %d out of %d", startIndex,
        // endIndex, tags.size()));

        indent_ = indent;

        startIndex_ = startIndex;
        endIndex_ = endIndex;
        tags_ = tags;
        content_ = new HashMap< String, java.util.List< String >>( );
    }

    @Override
    public void createControl( Composite parent )
    {
        super.createControl( parent );
        container_ = new Composite( parent, SWT.NULL );
        setControl( container_ );
        container_.setLayout( new GridLayout( 2, false ) );

        for( int i = startIndex_; i <= endIndex_; i++ ) {
            final WMLTag tag = tags_.get( i );
            if( tag.is_Forbidden( ) ) {
                continue;
            }

            Group tagGroup = new Group( container_, SWT.NONE );
            tagGroup.setLayoutData( new GridData( SWT.FILL, SWT.CENTER, false,
                false, 1, 1 ) );
            tagGroup.setText( "[" + tag.getName( ) + "]" ); //$NON-NLS-1$ //$NON-NLS-2$
            tagGroup.setLayout( new GridLayout( 2, false ) );

            List list = new List( tagGroup, SWT.BORDER );
            GridData gd_list = new GridData( SWT.FILL, SWT.CENTER, false,
                false, 1, 2 );
            gd_list.heightHint = 81;
            gd_list.widthHint = 150;
            list.setLayoutData( gd_list );

            Button btnAdd = new Button( tagGroup, SWT.NONE );
            GridData gd_btnAdd = new GridData( SWT.FILL, SWT.FILL, false,
                false, 1, 1 );
            gd_btnAdd.heightHint = 40;
            btnAdd.setLayoutData( gd_btnAdd );
            btnAdd.setText( Messages.WizardGeneratorPageTag_4 );
            btnAdd.setData( "list", list ); //$NON-NLS-1$
            btnAdd.addSelectionListener( new SelectionAdapter( ) {
                @Override
                public void widgetSelected( SelectionEvent e )
                {
                    if( ! ( e.getSource( ) instanceof Button ) ) {
                        return;
                    }
                    addNewItem( ( List ) ( ( Button ) e.getSource( ) )
                        .getData( "list" ), tag ); //$NON-NLS-1$
                }
            } );

            Button btnRemove = new Button( tagGroup, SWT.NONE );
            btnRemove.setLayoutData( new GridData( SWT.FILL, SWT.FILL, false,
                false, 1, 1 ) );
            btnRemove.setText( Messages.WizardGeneratorPageTag_7 );
            btnRemove.setData( "list", list ); //$NON-NLS-1$
            btnRemove.addSelectionListener( new SelectionAdapter( ) {
                @Override
                public void widgetSelected( SelectionEvent e )
                {
                    if( ! ( e.getSource( ) instanceof Button ) ) {
                        return;
                    }
                    removeItem( ( List ) ( ( Button ) e.getSource( ) )
                        .getData( "list" ), tag.getName( ) ); //$NON-NLS-1$
                }
            } );

            tagGroup.setData( "list", list ); //$NON-NLS-1$
            tagGroup.setData( "tag", tag ); //$NON-NLS-1$
            content_.put( tag.getName( ), new ArrayList< String >( ) );
        }
        updatePageIsComplete( );
    }

    private void addNewItem( List targetList, WMLTag tag )
    {
        if( ( tag.is_Optional( ) || tag.is_Required( ) )
            && targetList.getItemCount( ) == 1 ) {
            GUIUtils.showWarnMessageBox( Messages.WizardGeneratorPageTag_12 );
            return;
        }

        WizardGenerator wizard = new WizardGenerator(
            Messages.WizardGeneratorPageTag_13 + tag.getName( ),
            tag.getName( ), ( byte ) ( indent_ + 1 ) );
        WizardUtils.launchWizard( wizard, getShell( ), null );
        if( wizard.isFinished( ) ) {
            targetList
                .add( wizard.getObjectName( ) + targetList.getItemCount( ) );
            content_.get( tag.getName( ) ).add( wizard.getData( ).toString( ) );
        }
        updatePageIsComplete( );
    }

    private void removeItem( List targetList, String tagName )
    {
        if( targetList.getSelectionCount( ) == 0
            || targetList.getItemCount( ) == 0 ) {
            GUIUtils.showWarnMessageBox( Messages.WizardGeneratorPageTag_14 );
            return;
        }

        content_.get( tagName ).remove( targetList.getSelectionIndex( ) );
        targetList.remove( targetList.getSelectionIndex( ) );
        updatePageIsComplete( );
    }

    private void updatePageIsComplete( )
    {
        setPageComplete( false );
        for( Control control: container_.getChildren( ) ) {
            if( ! ( control instanceof Group ) ) {
                continue;
            }

            int cnt = ( ( List ) control.getData( "list" ) ).getItemCount( ); //$NON-NLS-1$
            WMLTag tag = ( WMLTag ) control.getData( "tag" ); //$NON-NLS-1$
            if( cnt == 0 && tag.is_Required( ) ) {
                setErrorMessage( String.format(
                    Messages.WizardGeneratorPageTag_0, tag.getName( ) ) );
                return;
            }
        }

        setPageComplete( true );
        setErrorMessage( null );
    }

    /**
     * @return The WML Code generated by this page
     */
    public String getContent( )
    {
        StringBuilder result = new StringBuilder( );
        for( Entry< String, java.util.List< String >> tag: content_.entrySet( ) ) {
            result
                .append( StringUtils.multiples( "\t", indent_ ) + "[" + tag.getKey( ) + "]\n" ); //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$
            result
                .append( ListUtils.concatenateList( tag.getValue( ), "\n\t" ) ); //$NON-NLS-1$
            result
                .append( StringUtils.multiples( "\t", indent_ ) + "[/" + tag.getKey( ) + "]\n" ); //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$
        }
        return result.toString( );
    }
}
