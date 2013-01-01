/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.labeling.wmldoc;

import org.eclipse.swt.custom.StyleRange;

/**
 * An interface used to provide WML documentation
 */
public interface IWMLDocProvider
{
    /**
     * Gets the title of the wmldoc dialog
     * 
     * @return
     *         Gets the title of the wmldoc dialog
     */
    public String getTitle( );

    /**
     * Gets the text to be written in the info statusbar
     * 
     * @return
     *         Gets the text to be written in the info statusbar
     */
    public String getInfoText( );

    /**
     * Gets the contents to be written into the
     * styledtext control of the wmldoc dialog
     * 
     * @return
     *         Gets the contents to be written into the
     *         styledtext control of the wmldoc dialog
     */
    public String getContents( );

    /**
     * Gets an array of StyleRange used to style the contents
     * 
     * @return
     *         Gets an array of StyleRange used to style the contents
     */
    public StyleRange[] getStyleRanges( );
}
