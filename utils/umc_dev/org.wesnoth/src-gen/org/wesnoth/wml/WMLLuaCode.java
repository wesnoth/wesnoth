/**
 * <copyright>
 * </copyright>
 *

 */
package org.wesnoth.wml;


/**
 * <!-- begin-user-doc -->
 * A representation of the model object '<em><b>WML Lua Code</b></em>'.
 * <!-- end-user-doc -->
 *
 * <p>
 * The following features are supported:
 * <ul>
 *   <li>{@link org.wesnoth.wml.WMLLuaCode#getValue <em>Value</em>}</li>
 * </ul>
 * </p>
 *
 * @see org.wesnoth.wml.WmlPackage#getWMLLuaCode()
 * @model
 * @generated
 */
public interface WMLLuaCode extends WMLKeyValue
{
  /**
   * Returns the value of the '<em><b>Value</b></em>' attribute.
   * The default value is <code>""</code>.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Value</em>' attribute isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Value</em>' attribute.
   * @see #setValue(String)
   * @see org.wesnoth.wml.WmlPackage#getWMLLuaCode_Value()
   * @model default=""
   * @generated
   */
  String getValue();

  /**
   * Sets the value of the '{@link org.wesnoth.wml.WMLLuaCode#getValue <em>Value</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>Value</em>' attribute.
   * @see #getValue()
   * @generated
   */
  void setValue(String value);

} // WMLLuaCode
