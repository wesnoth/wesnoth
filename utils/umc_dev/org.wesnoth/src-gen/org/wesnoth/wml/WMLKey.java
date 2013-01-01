/**
 * <copyright>
 * </copyright>
 *

 */
package org.wesnoth.wml;

import org.eclipse.emf.common.util.EList;

/**
 * <!-- begin-user-doc -->
 * A representation of the model object '<em><b>WML Key</b></em>'.
 * <!-- end-user-doc -->
 *
 * <p>
 * The following features are supported:
 * <ul>
 *   <li>{@link org.wesnoth.wml.WMLKey#getValues <em>Values</em>}</li>
 *   <li>{@link org.wesnoth.wml.WMLKey#getEol <em>Eol</em>}</li>
 *   <li>{@link org.wesnoth.wml.WMLKey#is_Enum <em>Enum</em>}</li>
 *   <li>{@link org.wesnoth.wml.WMLKey#is_Translatable <em>Translatable</em>}</li>
 *   <li>{@link org.wesnoth.wml.WMLKey#get_DataType <em>Data Type</em>}</li>
 * </ul>
 * </p>
 *
 * @see org.wesnoth.wml.WmlPackage#getWMLKey()
 * @model
 * @generated
 */
public interface WMLKey extends WMLExpression
{
  /**
   * Returns the value of the '<em><b>Values</b></em>' containment reference list.
   * The list contents are of type {@link org.wesnoth.wml.WMLKeyValue}.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Values</em>' containment reference list isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Values</em>' containment reference list.
   * @see org.wesnoth.wml.WmlPackage#getWMLKey_Values()
   * @model containment="true"
   * @generated
   */
  EList<WMLKeyValue> getValues();

  /**
   * Returns the value of the '<em><b>Eol</b></em>' attribute list.
   * The list contents are of type {@link java.lang.String}.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Eol</em>' attribute list isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Eol</em>' attribute list.
   * @see org.wesnoth.wml.WmlPackage#getWMLKey_Eol()
   * @model default="" unique="false"
   * @generated
   */
  EList<String> getEol();

  /**
   * Returns the value of the '<em><b>Enum</b></em>' attribute.
   * The default value is <code>"false"</code>.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Enum</em>' attribute isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Enum</em>' attribute.
   * @see #set_Enum(boolean)
   * @see org.wesnoth.wml.WmlPackage#getWMLKey__Enum()
   * @model default="false"
   * @generated
   */
  boolean is_Enum();

  /**
   * Sets the value of the '{@link org.wesnoth.wml.WMLKey#is_Enum <em>Enum</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>Enum</em>' attribute.
   * @see #is_Enum()
   * @generated
   */
  void set_Enum(boolean value);

  /**
   * Returns the value of the '<em><b>Translatable</b></em>' attribute.
   * The default value is <code>"false"</code>.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Translatable</em>' attribute isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Translatable</em>' attribute.
   * @see #set_Translatable(boolean)
   * @see org.wesnoth.wml.WmlPackage#getWMLKey__Translatable()
   * @model default="false"
   * @generated
   */
  boolean is_Translatable();

  /**
   * Sets the value of the '{@link org.wesnoth.wml.WMLKey#is_Translatable <em>Translatable</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>Translatable</em>' attribute.
   * @see #is_Translatable()
   * @generated
   */
  void set_Translatable(boolean value);

  /**
   * Returns the value of the '<em><b>Data Type</b></em>' attribute.
   * The default value is <code>""</code>.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Data Type</em>' attribute isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Data Type</em>' attribute.
   * @see #set_DataType(String)
   * @see org.wesnoth.wml.WmlPackage#getWMLKey__DataType()
   * @model default=""
   * @generated
   */
  String get_DataType();

  /**
   * Sets the value of the '{@link org.wesnoth.wml.WMLKey#get_DataType <em>Data Type</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>Data Type</em>' attribute.
   * @see #get_DataType()
   * @generated
   */
  void set_DataType(String value);

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @model kind="operation"
   *        annotation="http://www.eclipse.org/emf/2002/GenModel body=' return org.wesnoth.utils.WMLUtils.getKeyValue( getValues( ) );'"
   * @generated
   */
  String getValue();

} // WMLKey
