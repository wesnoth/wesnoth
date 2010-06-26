/**
 * <copyright>
 * </copyright>
 *
 */
package org.wesnoth.wML;


/**
 * <!-- begin-user-doc -->
 * A representation of the model object '<em><b>Macro</b></em>'.
 * <!-- end-user-doc -->
 *
 * <p>
 * The following features are supported:
 * <ul>
 *   <li>{@link org.wesnoth.wML.Macro#getMacroName <em>Macro Name</em>}</li>
 * </ul>
 * </p>
 *
 * @see org.wesnoth.wML.WMLPackage#getMacro()
 * @model
 * @generated
 */
public interface Macro extends Preprocessor
{
  /**
   * Returns the value of the '<em><b>Macro Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Macro Name</em>' attribute isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Macro Name</em>' attribute.
   * @see #setMacroName(String)
   * @see org.wesnoth.wML.WMLPackage#getMacro_MacroName()
   * @model
   * @generated
   */
  String getMacroName();

  /**
   * Sets the value of the '{@link org.wesnoth.wML.Macro#getMacroName <em>Macro Name</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>Macro Name</em>' attribute.
   * @see #getMacroName()
   * @generated
   */
  void setMacroName(String value);

} // Macro
