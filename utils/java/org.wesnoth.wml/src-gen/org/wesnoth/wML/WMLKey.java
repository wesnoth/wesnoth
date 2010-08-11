/**
 * <copyright>
 * </copyright>
 *

 */
package org.wesnoth.wML;

import org.eclipse.emf.common.util.EList;

import org.eclipse.emf.ecore.EObject;

/**
 * <!-- begin-user-doc -->
 * A representation of the model object '<em><b>Key</b></em>'.
 * <!-- end-user-doc -->
 *
 * <p>
 * The following features are supported:
 * <ul>
 *   <li>{@link org.wesnoth.wML.WMLKey#getName <em>Name</em>}</li>
 *   <li>{@link org.wesnoth.wML.WMLKey#getValue <em>Value</em>}</li>
 *   <li>{@link org.wesnoth.wML.WMLKey#getExtraArgs <em>Extra Args</em>}</li>
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
   * Returns the value of the '<em><b>Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Name</em>' attribute isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Name</em>' attribute.
   * @see #setName(String)
   * @see org.wesnoth.wML.WMLPackage#getWMLKey_Name()
   * @model
   * @generated
   */
  String getName();

  /**
   * Sets the value of the '{@link org.wesnoth.wML.WMLKey#getName <em>Name</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>Name</em>' attribute.
   * @see #getName()
   * @generated
   */
  void setName(String value);

  /**
   * Returns the value of the '<em><b>Value</b></em>' containment reference.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Value</em>' containment reference isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Value</em>' containment reference.
   * @see #setValue(WMLKeyValue)
   * @see org.wesnoth.wML.WMLPackage#getWMLKey_Value()
   * @model containment="true"
   * @generated
   */
  WMLKeyValue getValue();

  /**
   * Sets the value of the '{@link org.wesnoth.wML.WMLKey#getValue <em>Value</em>}' containment reference.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>Value</em>' containment reference.
   * @see #getValue()
   * @generated
   */
  void setValue(WMLKeyValue value);

  /**
   * Returns the value of the '<em><b>Extra Args</b></em>' containment reference list.
   * The list contents are of type {@link org.wesnoth.wML.WMLKeyExtraArgs}.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Extra Args</em>' containment reference list isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Extra Args</em>' containment reference list.
   * @see org.wesnoth.wML.WMLPackage#getWMLKey_ExtraArgs()
   * @model containment="true"
   * @generated
   */
  EList<WMLKeyExtraArgs> getExtraArgs();

} // WMLKey
