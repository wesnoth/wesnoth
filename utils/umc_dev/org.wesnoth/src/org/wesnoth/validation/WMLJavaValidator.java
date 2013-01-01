/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.validation;

import com.google.common.collect.Iterables;

import java.util.HashMap;
import java.util.Map;
import java.util.Map.Entry;

import org.eclipse.core.resources.IResource;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.xtext.nodemodel.ICompositeNode;
import org.eclipse.xtext.nodemodel.ILeafNode;
import org.eclipse.xtext.nodemodel.util.NodeModelUtils;
import org.eclipse.xtext.validation.Check;
import org.eclipse.xtext.validation.CheckType;

import org.wesnoth.Messages;
import org.wesnoth.installs.WesnothInstallsUtils;
import org.wesnoth.preferences.Preferences;
import org.wesnoth.projects.ProjectCache;
import org.wesnoth.projects.ProjectUtils;
import org.wesnoth.schema.SchemaParser;
import org.wesnoth.utils.ResourceUtils;
import org.wesnoth.utils.WMLUtils;
import org.wesnoth.wml.WMLExpression;
import org.wesnoth.wml.WMLKey;
import org.wesnoth.wml.WMLMacroCall;
import org.wesnoth.wml.WMLRoot;
import org.wesnoth.wml.WMLTag;
import org.wesnoth.wml.WmlPackage;

/**
 * This represents the validator for config files
 * 
 * http://wiki.eclipse.org/Xtext/Documentation/Xtext_New_and_Noteworthy#
 * Different_validation_hooks
 * CheckType:
 * 1. during editing with a delay of 500ms, only FAST is passed
 * 2. on save NORMAL is passed
 * 3. an action, which can be optionally generated for you DSL, explicitely
 * evaluates EXPENSIVE constraints
 */
public class WMLJavaValidator extends AbstractWMLJavaValidator
{
    /**
     * Returns the {@link SchemaParser} from the specified {@link EObject}
     * 
     * @param object
     * @return
     */
    private SchemaParser getSchema( EObject object )
    {
        return SchemaParser.getInstance( WesnothInstallsUtils
            .getInstallNameForResource( ResourceUtils
                .getWorkspaceResource( object.eResource( ) ) ) );
    }

    private boolean isValidationEnabled( )
    {
        return Preferences.getBool( Preferences.WML_VALIDATION );
    }

    /**
     * Checks if the tag's start and end name are equal.
     * 
     * @param tag
     *        The {@link WMLTag} to check
     */
    @Check( CheckType.FAST )
    public void checkFastTagName( WMLTag tag )
    {
        if( ! isValidationEnabled( ) ) {
            return;
        }

        if( ! tag.getName( ).equals( tag.getEndName( ) ) ) {
            warning( Messages.WMLJavaValidator_0,
                WmlPackage.Literals.WML_TAG__END_NAME );
        }
    }

    /**
     * Checks if the tag name exists in the schema.
     * 
     * @param tag
     *        The {@link WMLTag} to check
     */
    @Check( CheckType.NORMAL )
    public void checkNormalTagName( WMLTag tag )
    {
        if( ! isValidationEnabled( ) ) {
            return;
        }

        ICompositeNode node = NodeModelUtils.getNode( tag );
        if( node != null ) {
            ILeafNode parentNode = NodeModelUtils.findLeafNodeAtOffset(
                node.getParent( ), node.getParent( ).getOffset( ) + 2 );

            boolean found = false;
            String searchName = parentNode.getText( );
            if( node.getParent( ) == null ) // root node
            {
                searchName = "root"; //$NON-NLS-1$
            }

            WMLTag schemaTag = getSchema( tag ).getTags( ).get( searchName );

            if( schemaTag != null ) {
                for( WMLExpression expression: schemaTag.getExpressions( ) ) {
                    if( expression.getName( ).equals( tag.getName( ) ) ) {
                        found = true;
                        break;
                    }
                }
                if( found == false ) {
                    warning( Messages.WMLJavaValidator_1,
                        WmlPackage.Literals.WML_EXPRESSION__NAME );
                }
            }
        }
    }

    private void checkTagsCardinalities( SchemaParser schema,
        Iterable< WMLTag > tags )
    {
        Map< String, Integer > ocurrences = new HashMap< String, Integer >( );

        for( WMLTag tag: tags ) {
            Integer currentValue = ocurrences.get( tag.getName( ) );
            if( currentValue == null ) {
                currentValue = 0;
            }

            ocurrences.put( tag.getName( ), currentValue + 1 );
        }

        for( Entry< String, Integer > entry: ocurrences.entrySet( ) ) {
            WMLTag schemaTag = schema.getTags( ).get( entry.getKey( ) );

            if( schemaTag == null ) {
                continue;
            }

            if( schemaTag.getAllowedCount( ) < entry.getValue( ) ) {
                warning( "Tag " + entry.getKey( ) + " cannot appear more"
                    + "than " + schemaTag.getAllowedCount( ) + " times. ",
                    WmlPackage.Literals.WML_EXPRESSION__NAME );
            }
        }
    }

    private void checkKeysCardinalities( WMLTag parentTag,
        Iterable< WMLKey > keys )
    {
        Map< String, Integer > ocurrences = new HashMap< String, Integer >( );

        for( WMLKey key: keys ) {
            Integer currentValue = ocurrences.get( key.getName( ) );
            if( currentValue == null ) {
                currentValue = 0;
            }

            ocurrences.put( key.getName( ), currentValue + 1 );
        }

        for( Entry< String, Integer > entry: ocurrences.entrySet( ) ) {
            WMLKey schemaKey = WMLUtils
                .getKeyByName( parentTag, entry.getKey( ) );

            if( schemaKey == null ) {
                continue;
            }

            if( schemaKey.getAllowedCount( ) < entry.getValue( ) ) {
                warning(
                    "Key " + entry.getKey( ) + ", in tag "
                        + parentTag.getName( )
                        + "cannot appear more than "
                        + schemaKey.getAllowedCount( ) + " times. ",
                    WmlPackage.Literals.WML_EXPRESSION__NAME );
            }
        }
    }

    /**
     * Checks the cardinality correctness of the WMLRoot
     * 
     * @param root
     *        The {@link WMLRoot} to check
     */
    @Check( CheckType.NORMAL )
    public void checkNormalWMLRootCardinality( WMLRoot root )
    {
        if( ! isValidationEnabled( ) ) {
            return;
        }

        checkTagsCardinalities( getSchema( root ),
            Iterables.filter( root.getExpressions( ), WMLTag.class ) );
    }

    /**
     * Checks the cardinality correctness of the WMLTag
     * 
     * @param tag
     *        The {@link WMLTag} to check
     */
    @Check( CheckType.NORMAL )
    public void checkNormalWMLTagCardinality( WMLTag tag )
    {
        if( ! isValidationEnabled( ) ) {
            return;
        }

        SchemaParser schema = getSchema( tag );
        checkTagsCardinalities( schema,
            Iterables.filter( tag.getExpressions( ), WMLTag.class ) );

        checkKeysCardinalities( tag,
            Iterables.filter( tag.getExpressions( ), WMLKey.class ) );
    }

    /**
     * Checks if the specified macro exists
     * 
     * @param call
     *        The {@link WMLMacroCall} to check
     */
    @Check( CheckType.NORMAL )
    public void checkNormalWMLMacroExistance( WMLMacroCall call )
    {
        if( ! isValidationEnabled( ) ) {
            return;
        }

        IResource resource = ResourceUtils.getWorkspaceResource( call
            .eResource( ) );
        ProjectCache cache = ProjectUtils.getCacheForProject( resource
            .getProject( ) );
        if( ! cache.getDefines( ).containsKey( call.getName( ) ) ) {
            warning( "Undefined macro: " + call.getName( ),
                WmlPackage.Literals.WML_EXPRESSION__NAME );
        }
    }
}
