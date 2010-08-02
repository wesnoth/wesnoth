/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.labeling;

import java.util.Locale;

import org.eclipse.emf.ecore.EClass;
import org.eclipse.emf.edit.ui.provider.AdapterFactoryLabelProvider;
import org.eclipse.swt.graphics.Image;
import org.eclipse.xtext.ui.label.DefaultEObjectLabelProvider;

import com.google.inject.Inject;

/**
 * Provides labels for a EObjects.
 *
 * see http://www.eclipse.org/Xtext/documentation/latest/xtext.html#labelProvider
 */
public class WMLLabelProvider extends DefaultEObjectLabelProvider
{
	private static WMLLabelProvider instance_;
	@Inject
	public WMLLabelProvider(AdapterFactoryLabelProvider delegate) {
		super(delegate);
		instance_ = this;
	}

	@Override
	protected Object doGetImage(Object element)
	{
		if (element instanceof EClass)
		{
			return ((EClass)element).getName().toLowerCase(Locale.ENGLISH) + ".png";
		}
		else if (element instanceof String)
		{
			return element;
		}
		return super.doGetImage(element);
	}

	public static Image getImageByName(String fileName)
	{
		if (instance_ == null)
			return null;
		return instance_.getImage(fileName);
	}
}
