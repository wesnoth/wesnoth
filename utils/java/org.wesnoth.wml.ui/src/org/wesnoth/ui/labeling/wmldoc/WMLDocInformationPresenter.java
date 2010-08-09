/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
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
import org.eclipse.swt.events.MouseEvent;
import org.eclipse.swt.events.MouseListener;
import org.eclipse.swt.events.MouseMoveListener;
import org.eclipse.swt.events.MouseTrackListener;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.events.SelectionListener;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Shell;
import org.wesnoth.ui.WMLUtil;

import wesnoth_eclipse_plugin.utils.ProjectUtils;

public class WMLDocInformationPresenter extends PopupDialog implements
		MouseListener, SelectionListener, MouseTrackListener, MouseMoveListener
{
	private Point bounds_;
	private String currentMacroName_;
	private Composite panel_;

	public WMLDocInformationPresenter(Shell parent, String macroName,
			String titleText, String infoText, Point bounds)
	{
		super(parent, PopupDialog.INFOPOPUPRESIZE_SHELLSTYLE, true, true, true,
				true, false, titleText, infoText);
		bounds_ = bounds;
		currentMacroName_ = macroName;
	}

	@Override
	protected Control createInfoTextArea(Composite parent)
	{
		//Add back& forward button
		return super.createInfoTextArea(parent);
	}

	@Override
	protected Control createDialogArea(Composite parent)
	{
		panel_ = new Composite(parent, SWT.None);
		panel_.setBackground(parent.getDisplay().getSystemColor(SWT.COLOR_INFO_BACKGROUND));
		panel_.setForeground(parent.getDisplay().getSystemColor(SWT.COLOR_INFO_FOREGROUND));

		GridLayout grid = new GridLayout();
		grid.numColumns = 5;
		panel_.setLayout(grid);
		StyledText text = new StyledText(panel_, SWT.None);

		text.setText(ProjectUtils.getCacheForProject(WMLUtil.getActiveEditorFile().getProject())
    			.getDefines().get(currentMacroName_).getValue());
		text.setEditable(false);
		//text.setStyleRanges(ranges);

		text.setLayoutData(createDefaultGridData(4));
		text.addMouseListener(this);
		text.addMouseTrackListener(this);
		text.addMouseMoveListener(this);
		return panel_;
	}

	private GridData createDefaultGridData(int columnspan) {
		GridData gd = new GridData();
		gd.horizontalSpan = columnspan;
		gd.verticalAlignment = SWT.BEGINNING;
		gd.verticalIndent = 0;
		gd.horizontalIndent = 5;
		return gd;
	}
	@Override
	protected Point getInitialLocation(Point initialSize)
	{
		return bounds_;
	}

	public void mouseMove(MouseEvent e)
	{
	}

	public void mouseEnter(MouseEvent e)
	{
	}

	public void mouseExit(MouseEvent e)
	{
	}

	public void mouseHover(MouseEvent e)
	{
	}

	public void widgetSelected(SelectionEvent e)
	{
	}

	public void widgetDefaultSelected(SelectionEvent e)
	{
	}

	public void mouseDoubleClick(MouseEvent e)
	{
	}

	public void mouseDown(MouseEvent e)
	{
	}

	public void mouseUp(MouseEvent e)
	{
	}
}

