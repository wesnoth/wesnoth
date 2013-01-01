/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.labeling.wmldoc;

import org.eclipse.jface.dialogs.PopupDialog;
import org.eclipse.swt.SWT;
import org.eclipse.swt.custom.StyledText;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Shell;

/**
 * Presents WMLDoc to the user
 */
public class WMLDocInformationPresenter extends PopupDialog
{
    private Point           bounds_;
    private IWMLDocProvider currentDocProvider_;
    private Composite       panel_;

    /**
     * Creates a new WMLDocumentation information presenter
     * 
     * @param parent
     *        The parent shell
     * @param docProvider
     *        The WMLDoc provider
     * @param bounds
     *        The bounds of the presented dialog
     */
    public WMLDocInformationPresenter( Shell parent,
        IWMLDocProvider docProvider, Point bounds )
    {
        super( parent, PopupDialog.INFOPOPUPRESIZE_SHELLSTYLE, true, true,
            true, false, false, docProvider.getTitle( ), docProvider
                .getInfoText( ) );

        bounds_ = bounds;
        currentDocProvider_ = docProvider;
    }

    @Override
    protected Control createDialogArea( Composite parent )
    {
        panel_ = new Composite( parent, SWT.None );
        panel_.setBackground( parent.getDisplay( ).getSystemColor(
            SWT.COLOR_INFO_BACKGROUND ) );
        panel_.setForeground( parent.getDisplay( ).getSystemColor(
            SWT.COLOR_INFO_FOREGROUND ) );

        GridLayout grid = new GridLayout( );
        grid.numColumns = 5;
        panel_.setLayout( grid );

        StyledText text = new StyledText( panel_, SWT.NONE );

        text.setText( currentDocProvider_.getContents( ) );
        text.setEditable( false );
        text.setStyleRanges( currentDocProvider_.getStyleRanges( ) );

        text.setLayoutData( createDefaultGridData( 4 ) );
        return panel_;
    }

    private GridData createDefaultGridData( int columnspan )
    {
        GridData gd = new GridData( );
        gd.horizontalSpan = columnspan;
        gd.verticalAlignment = SWT.BEGINNING;
        gd.verticalIndent = 0;
        gd.horizontalIndent = 5;
        return gd;
    }

    @Override
    protected Point getInitialLocation( Point initialSize )
    {
        return bounds_;
    }
}
