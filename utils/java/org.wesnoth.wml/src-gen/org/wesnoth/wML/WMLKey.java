/**
 * <copyright>
 * </copyright>
 *

 */
package org.wesnoth.wML;

import org.eclipse.emf.ecore.EObject;

/**
 * <!-- begin-user-doc -->
 * A representation of the model object '<em><b>Key</b></em>'.
 * <!-- end-user-doc -->
 *
 * <p>
 * The following features are supported:
 * <ul>
 *   <li>{@link org.wesnoth.wML.WMLKey#getKeyName <em>Key Name</em>}</li>
 *   <li>{@link org.wesnoth.wML.WMLKey#getKeyValue <em>Key Value</em>}</li>
 * </ul>
 * </p>
 *
 * @see org.wesnoth.wML.WMLPackage#getWMLKey()
 * @model
 * @generated
 */
public interface WMLKey extends EObject
{
  /**
   * Returns the value of the '<em><b>Key Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Key Name</em>' attribute isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Key Name</em>' attribute.
   * @see #setKeyName(String)
   * @see org.wesnoth.wML.WMLPackage#getWMLKey_KeyName()
   * @model
   * @generated
   */
  String getKeyName();

  /**
   * Sets the value of the '{@link org.wesnoth.wML.WMLKey#getKeyName <em>Key Name</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>Key Name</em>' attribute.
   * @see #getKeyName()
   * @generated
   */
  void setKeyName(String value);

  /**
   * Returns the value of the '<em><b>Key Value</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Key Value</em>' attribute isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Key Value</em>' attribute.
   * @see #setKeyValue(String)
   * @see org.wesnoth.wML.WMLPackage#getWMLKey_KeyValue()
   * @model
   * @generated
   */
  String getKeyValue();

  /**
   * Sets the value of the '{@link org.wesnoth.wML.WMLKey#getKeyValue <em>Key Value</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>Key Value</em>' attribute.
   * @see #getKeyValue()
   * @generated
   */
  void setKeyValue(String value);

} // WMLKey
