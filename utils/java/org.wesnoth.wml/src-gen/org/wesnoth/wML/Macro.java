/**
 * <copyright>
 * </copyright>
 *
 */
package org.wesnoth.wML;

import org.eclipse.emf.common.util.EList;

/**
 * <!-- begin-user-doc -->
 * A representation of the model object '<em><b>Macro</b></em>'.
 * <!-- end-user-doc -->
 *
 * <p>
 * The following features are supported:
 * <ul>
 *   <li>{@link org.wesnoth.wML.Macro#getMacroContent <em>Macro Content</em>}</li>
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
   * Returns the value of the '<em><b>Macro Content</b></em>' attribute list.
   * The list contents are of type {@link java.lang.String}.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Macro Content</em>' attribute list isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Macro Content</em>' attribute list.
   * @see org.wesnoth.wML.WMLPackage#getMacro_MacroContent()
   * @model unique="false"
   * @generated
   */
  EList<String> getMacroContent();

} // Macro
