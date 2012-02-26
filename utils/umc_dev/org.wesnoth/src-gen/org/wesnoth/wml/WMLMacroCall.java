/**
 * <copyright>
 * </copyright>
 *

 */
package org.wesnoth.wml;

import org.eclipse.emf.common.util.EList;

/**
 * <!-- begin-user-doc -->
 * A representation of the model object '<em><b>WML Macro Call</b></em>'.
 * <!-- end-user-doc -->
 *
 * <p>
 * The following features are supported:
 * <ul>
 *   <li>{@link org.wesnoth.wml.WMLMacroCall#getPoint <em>Point</em>}</li>
 *   <li>{@link org.wesnoth.wml.WMLMacroCall#getRelative <em>Relative</em>}</li>
 *   <li>{@link org.wesnoth.wml.WMLMacroCall#getParameters <em>Parameters</em>}</li>
 * </ul>
 * </p>
 *
 * @see org.wesnoth.wml.WmlPackage#getWMLMacroCall()
 * @model
 * @generated
 */
public interface WMLMacroCall extends WMLKeyValue, WMLMacroCallParameter, WMLRootExpression
{
  /**
   * Returns the value of the '<em><b>Point</b></em>' attribute.
   * The default value is <code>""</code>.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Point</em>' attribute isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Point</em>' attribute.
   * @see #setPoint(String)
   * @see org.wesnoth.wml.WmlPackage#getWMLMacroCall_Point()
   * @model default=""
   * @generated
   */
  String getPoint();

  /**
   * Sets the value of the '{@link org.wesnoth.wml.WMLMacroCall#getPoint <em>Point</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>Point</em>' attribute.
   * @see #getPoint()
   * @generated
   */
  void setPoint(String value);

  /**
   * Returns the value of the '<em><b>Relative</b></em>' attribute.
   * The default value is <code>""</code>.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Relative</em>' attribute isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Relative</em>' attribute.
   * @see #setRelative(String)
   * @see org.wesnoth.wml.WmlPackage#getWMLMacroCall_Relative()
   * @model default=""
   * @generated
   */
  String getRelative();

  /**
   * Sets the value of the '{@link org.wesnoth.wml.WMLMacroCall#getRelative <em>Relative</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>Relative</em>' attribute.
   * @see #getRelative()
   * @generated
   */
  void setRelative(String value);

  /**
   * Returns the value of the '<em><b>Parameters</b></em>' containment reference list.
   * The list contents are of type {@link org.wesnoth.wml.WMLMacroCallParameter}.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Parameters</em>' containment reference list isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Parameters</em>' containment reference list.
   * @see org.wesnoth.wml.WmlPackage#getWMLMacroCall_Parameters()
   * @model containment="true"
   * @generated
   */
  EList<WMLMacroCallParameter> getParameters();

} // WMLMacroCall
