/*******************************************************************************
 * Copyright (c) 2011 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.editor;

import org.eclipse.xtext.ui.editor.XtextEditor;
import org.eclipse.xtext.ui.editor.XtextSourceViewer;
import org.eclipse.xtext.ui.editor.syntaxcoloring.HighlightingHelper;

/**
 * Custom helper for our editor, so we can get an instance attached to
 * our editor
 */
public class WMLHighlightingHelper extends HighlightingHelper
{
    @Override
    public void install( XtextEditor editor, XtextSourceViewer sourceViewer )
    {
        super.install( editor, sourceViewer );

        // set the highlighting helper in our editor
        // so we can access the highlightingReconcilier in order
        // to be able to refresh the semantic highlighting
        if( editor instanceof WMLEditor ) {
            ( ( WMLEditor ) editor ).highlightingHelper_ = this;
        }
    }
}
