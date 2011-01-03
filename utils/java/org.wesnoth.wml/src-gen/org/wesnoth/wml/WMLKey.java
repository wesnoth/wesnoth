/**
 * <copyright>
 * </copyright>
 *

 */
package org.wesnoth.wml;

import org.eclipse.emf.common.util.EList;

import org.eclipse.emf.ecore.EObject;

/**
 * <!-- begin-user-doc -->
 * A representation of the model object '<em><b>WML Key</b></em>'.
 * <!-- end-user-doc -->
 *
 * <p>
 * The following features are supported:
 * <ul>
 *   <li>{@link org.wesnoth.wml.WMLKey#getName <em>Name</em>}</li>
 *   <li>{@link org.wesnoth.wml.WMLKey#getValue <em>Value</em>}</li>
 *   <li>{@link org.wesnoth.wml.WMLKey#getEol <em>Eol</em>}</li>
 * </ul>
 * </p>
 *
 * @see org.wesnoth.wml.WmlPackage#getWMLKey()
 * @model
 * @generated
 */
public interface WMLKey extends EObject
{
  /**
   * Returns the value of the '<em><b>Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Name</em>' attribute isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Name</em>' attribute.
   * @see #setName(String)
   * @see org.wesnoth.wml.WmlPackage#getWMLKey_Name()
   * @model
   * @generated
   */
  String getName();

  /**
   * Sets the value of the '{@link org.wesnoth.wml.WMLKey#getName <em>Name</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>Name</em>' attribute.
   * @see #getName()
   * @generated
   */
  void setName(String value);

  /**
   * Returns the value of the '<em><b>Value</b></em>' containment reference list.
   * The list contents are of type {@link org.wesnoth.wml.WMLKeyValue}.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Value</em>' containment reference list isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Value</em>' containment reference list.
   * @see org.wesnoth.wml.WmlPackage#getWMLKey_Value()
   * @model containment="true"
   * @generated
   */
  EList<WMLKeyValue> getValue();

  /**
   * Returns the value of the '<em><b>Eol</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Eol</em>' attribute isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Eol</em>' attribute.
   * @see #setEol(String)
   * @see org.wesnoth.wml.WmlPackage#getWMLKey_Eol()
   * @model
   * @generated
   */
  String getEol();

  /**
   * Sets the value of the '{@link org.wesnoth.wml.WMLKey#getEol <em>Eol</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>Eol</em>' attribute.
   * @see #getEol()
   * @generated
   */
  void setEol(String value);

} // WMLKey
