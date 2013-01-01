/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.utils;

/**
 * A Runnable that contains a result of the Runnable
 * 
 * @param <T>
 *        The type of the result
 */
public abstract class RunnableWithResult< T > implements Runnable
{
    private T result_;

    /**
     * Sets the result of the runnable
     * 
     * @param result
     *        The new result
     */
    public void setResult( T result )
    {
        result_ = result;
    }

    /**
     * @return The result of the runnable
     */
    public T getResult( )
    {
        return result_;
    }
}
