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
 *   <li>{@link org.wesnoth.wml.WMLTag#get_extendedTagName <em>extended Tag Name</em>}</li>
 *   <li>{@link org.wesnoth.wml.WMLTag#get_cardinality <em>cardinality</em>}</li>
 *   <li>{@link org.wesnoth.wml.WMLTag#is_needsExpanding <em>needs Expanding</em>}</li>
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
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>End Name</em>' attribute isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>End Name</em>' attribute.
   * @see #setEndName(String)
   * @see org.wesnoth.wml.WmlPackage#getWMLTag_EndName()
   * @model
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
   * Returns the value of the '<em><b>extended Tag Name</b></em>' attribute.
   * The default value is <code>""</code>.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>extended Tag Name</em>' attribute isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>extended Tag Name</em>' attribute.
   * @see #set_extendedTagName(String)
   * @see org.wesnoth.wml.WmlPackage#getWMLTag__extendedTagName()
   * @model default=""
   * @generated
   */
  String get_extendedTagName();

  /**
   * Sets the value of the '{@link org.wesnoth.wml.WMLTag#get_extendedTagName <em>extended Tag Name</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>extended Tag Name</em>' attribute.
   * @see #get_extendedTagName()
   * @generated
   */
  void set_extendedTagName(String value);

  /**
   * Returns the value of the '<em><b>cardinality</b></em>' attribute.
   * The default value is <code>" "</code>.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>cardinality</em>' attribute isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>cardinality</em>' attribute.
   * @see #set_cardinality(char)
   * @see org.wesnoth.wml.WmlPackage#getWMLTag__cardinality()
   * @model default=" "
   * @generated
   */
  char get_cardinality();

  /**
   * Sets the value of the '{@link org.wesnoth.wml.WMLTag#get_cardinality <em>cardinality</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>cardinality</em>' attribute.
   * @see #get_cardinality()
   * @generated
   */
  void set_cardinality(char value);

  /**
   * Returns the value of the '<em><b>needs Expanding</b></em>' attribute.
   * The default value is <code>"false"</code>.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>needs Expanding</em>' attribute isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>needs Expanding</em>' attribute.
   * @see #set_needsExpanding(boolean)
   * @see org.wesnoth.wml.WmlPackage#getWMLTag__needsExpanding()
   * @model default="false"
   * @generated
   */
  boolean is_needsExpanding();

  /**
   * Sets the value of the '{@link org.wesnoth.wml.WMLTag#is_needsExpanding <em>needs Expanding</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>needs Expanding</em>' attribute.
   * @see #is_needsExpanding()
   * @generated
   */
  void set_needsExpanding(boolean value);

} // WMLTag
