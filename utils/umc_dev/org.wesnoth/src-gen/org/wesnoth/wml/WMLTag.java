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
 *   <li>{@link org.wesnoth.wml.WMLTag#isPlus <em>Plus</em>}</li>
 *   <li>{@link org.wesnoth.wml.WMLTag#getExpressions <em>Expressions</em>}</li>
 *   <li>{@link org.wesnoth.wml.WMLTag#getEndName <em>End Name</em>}</li>
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
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Plus</em>' attribute isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Plus</em>' attribute.
   * @see #setPlus(boolean)
   * @see org.wesnoth.wml.WmlPackage#getWMLTag_Plus()
   * @model
   * @generated
   */
  boolean isPlus();

  /**
   * Sets the value of the '{@link org.wesnoth.wml.WMLTag#isPlus <em>Plus</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>Plus</em>' attribute.
   * @see #isPlus()
   * @generated
   */
  void setPlus(boolean value);

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

} // WMLTag
