/**
 * <copyright>
 * </copyright>
 *

 */
package org.wesnoth.wml;

import org.eclipse.emf.common.util.EList;

/**
 * <!-- begin-user-doc -->
 * A representation of the model object '<em><b>WML Tag</b></em>'.
 * <!-- end-user-doc -->
 *
 * <p>
 * The following features are supported:
 * <ul>
 *   <li>{@link org.wesnoth.wml.WMLTag#getPlus <em>Plus</em>}</li>
 *   <li>{@link org.wesnoth.wml.WMLTag#getExpressions <em>Expressions</em>}</li>
 *   <li>{@link org.wesnoth.wml.WMLTag#getEndName <em>End Name</em>}</li>
 *   <li>{@link org.wesnoth.wml.WMLTag#get_InhertedTagName <em>Inherted Tag Name</em>}</li>
 *   <li>{@link org.wesnoth.wml.WMLTag#is_NeedingExpansion <em>Needing Expansion</em>}</li>
 *   <li>{@link org.wesnoth.wml.WMLTag#get_Description <em>Description</em>}</li>
 * </ul>
 * </p>
 *
 * @see org.wesnoth.wml.WmlPackage#getWMLTag()
 * @model
 * @generated
 */
public interface WMLTag extends WMLRootExpression
{
  /**
   * Returns the value of the '<em><b>Plus</b></em>' attribute.
   * The default value is <code>""</code>.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Plus</em>' attribute isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Plus</em>' attribute.
   * @see #setPlus(String)
   * @see org.wesnoth.wml.WmlPackage#getWMLTag_Plus()
   * @model default=""
   * @generated
   */
  String getPlus();

  /**
   * Sets the value of the '{@link org.wesnoth.wml.WMLTag#getPlus <em>Plus</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>Plus</em>' attribute.
   * @see #getPlus()
   * @generated
   */
  void setPlus(String value);

  /**
   * Returns the value of the '<em><b>Expressions</b></em>' containment reference list.
   * The list contents are of type {@link org.wesnoth.wml.WMLExpression}.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Expressions</em>' containment reference list isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Expressions</em>' containment reference list.
   * @see org.wesnoth.wml.WmlPackage#getWMLTag_Expressions()
   * @model containment="true"
   * @generated
   */
  EList<WMLExpression> getExpressions();

  /**
   * Returns the value of the '<em><b>End Name</b></em>' attribute.
   * The default value is <code>""</code>.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>End Name</em>' attribute isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>End Name</em>' attribute.
   * @see #setEndName(String)
   * @see org.wesnoth.wml.WmlPackage#getWMLTag_EndName()
   * @model default=""
   * @generated
   */
  String getEndName();

  /**
   * Sets the value of the '{@link org.wesnoth.wml.WMLTag#getEndName <em>End Name</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>End Name</em>' attribute.
   * @see #getEndName()
   * @generated
   */
  void setEndName(String value);

  /**
   * Returns the value of the '<em><b>Inherted Tag Name</b></em>' attribute.
   * The default value is <code>""</code>.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Inherted Tag Name</em>' attribute isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Inherted Tag Name</em>' attribute.
   * @see #set_InhertedTagName(String)
   * @see org.wesnoth.wml.WmlPackage#getWMLTag__InhertedTagName()
   * @model default=""
   * @generated
   */
  String get_InhertedTagName();

  /**
   * Sets the value of the '{@link org.wesnoth.wml.WMLTag#get_InhertedTagName <em>Inherted Tag Name</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>Inherted Tag Name</em>' attribute.
   * @see #get_InhertedTagName()
   * @generated
   */
  void set_InhertedTagName(String value);

  /**
   * Returns the value of the '<em><b>Needing Expansion</b></em>' attribute.
   * The default value is <code>"false"</code>.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Needing Expansion</em>' attribute isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Needing Expansion</em>' attribute.
   * @see #set_NeedingExpansion(boolean)
   * @see org.wesnoth.wml.WmlPackage#getWMLTag__NeedingExpansion()
   * @model default="false"
   * @generated
   */
  boolean is_NeedingExpansion();

  /**
   * Sets the value of the '{@link org.wesnoth.wml.WMLTag#is_NeedingExpansion <em>Needing Expansion</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>Needing Expansion</em>' attribute.
   * @see #is_NeedingExpansion()
   * @generated
   */
  void set_NeedingExpansion(boolean value);

  /**
   * Returns the value of the '<em><b>Description</b></em>' attribute.
   * The default value is <code>""</code>.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Description</em>' attribute isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Description</em>' attribute.
   * @see #set_Description(String)
   * @see org.wesnoth.wml.WmlPackage#getWMLTag__Description()
   * @model default=""
   * @generated
   */
  String get_Description();

  /**
   * Sets the value of the '{@link org.wesnoth.wml.WMLTag#get_Description <em>Description</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>Description</em>' attribute.
   * @see #get_Description()
   * @generated
   */
  void set_Description(String value);

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @model kind="operation" many="false"
   *        annotation="http://www.eclipse.org/emf/2002/GenModel body='EList<WMLTag> result = new org.eclipse.emf.common.util.BasicEList<WMLTag>();\n        for ( WMLExpression expression : getExpressions( ) ) {\n            if ( expression.isWMLTag( ) )\n                result.add( expression.asWMLTag( ) );\n        }\n\n        return result;'"
   * @generated
   */
  EList<WMLTag> getWMLTags();

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @model kind="operation" many="false"
   *        annotation="http://www.eclipse.org/emf/2002/GenModel body='EList<WMLKey> result = new org.eclipse.emf.common.util.BasicEList<WMLKey>();\n        for ( WMLExpression expression : getExpressions( ) ) {\n            if ( expression.isWMLKey( ) )\n                result.add( expression.asWMLKey( ) );\n        }\n\n        return result;'"
   * @generated
   */
  EList<WMLKey> getWMLKeys();

} // WMLTag
