/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.wizards.scenario;

import java.util.Map;

import org.eclipse.core.resources.IContainer;
import org.eclipse.swt.SWT;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Text;

import org.wesnoth.Messages;
import org.wesnoth.projects.ProjectUtils;
import org.wesnoth.wizards.WizardPageTemplate;

/**
 * The second page of New Scenario Wizard
 */
public class ScenarioPage1 extends WizardPageTemplate
{
    private Composite container_;

    /**
     * Creates a new {@link ScenarioPage1}
     */
    public ScenarioPage1( )
    {
        super( "scenarioPage1" ); //$NON-NLS-1$
        setTitle( Messages.ScenarioPage1_1 );
        setDescription( Messages.ScenarioPage1_2 );
    }

    @Override
    public void createControl( Composite parent )
    {
        super.createControl( parent );
        container_ = new Composite( parent, SWT.NULL );

        setControl( container_ );
        container_.setLayout( new GridLayout( 4, false ) );

        Label lblSpecifyTheGold = new Label( container_, SWT.NONE );
        lblSpecifyTheGold.setLayoutData( new GridData( SWT.LEFT, SWT.CENTER,
            false, false, 4, 1 ) );
        lblSpecifyTheGold.setText( Messages.ScenarioPage1_3 );

        IContainer selContainer = getWizard( ).getSelectionContainer( );
        if( selContainer != null ) {
            Map< String, String > prefs = ProjectUtils
                .getPropertiesForProject( selContainer.getProject( ) );

            if( prefs.get( "difficulties" ) != null ) //$NON-NLS-1$
            {
                String[] difficulties = prefs.get( "difficulties" ).split( "," ); //$NON-NLS-1$ //$NON-NLS-2$
                for( String diff: difficulties ) {
                    if( diff.isEmpty( ) ) {
                        continue;
                    }

                    Label label = new Label( container_, SWT.NONE );
                    label.setText( "    " ); //$NON-NLS-1$

                    Label lblDiff = new Label( container_, SWT.NONE );
                    lblDiff.setLayoutData( new GridData( SWT.RIGHT, SWT.CENTER,
                        false, false, 1, 1 ) );
                    lblDiff.setText( diff + ":" ); //$NON-NLS-1$

                    Text textBox = new Text( container_, SWT.BORDER );
                    GridData gd_text = new GridData( SWT.LEFT, SWT.CENTER,
                        true, false, 1, 1 );
                    gd_text.widthHint = 77;
                    textBox.setData( "diff", diff ); //$NON-NLS-1$
                    textBox.setLayoutData( gd_text );
                    new Label( container_, SWT.NONE );
                }
            }
        }
    }

    /**
     * Gets the starting gold as #ifdef based on difficulties
     * 
     * @return The WML to set the starting gold based on the difficulties levels
     */
    public String getStartingGoldByDifficulties( )
    {
        StringBuilder result = new StringBuilder( );
        for( Control control: container_.getChildren( ) ) {
            if( ! ( control instanceof Text ) ) {
                continue;
            }
            Text textBox = ( Text ) control;
            result.append( String.format( "#ifdef %s\n\tgold=%s\n#endif\n", //$NON-NLS-1$
                textBox.getData( "diff" ).toString( ), textBox.getText( ) ) ); //$NON-NLS-1$
        }
        return result.toString( );
    }
}
