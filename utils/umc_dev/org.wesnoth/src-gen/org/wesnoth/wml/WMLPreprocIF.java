/**
 * <copyright>
 * </copyright>
 *

 */
package org.wesnoth.wml;

import org.eclipse.emf.common.util.EList;

/**
 * <!-- begin-user-doc -->
 * A representation of the model object '<em><b>WML Preproc IF</b></em>'.
 * <!-- end-user-doc -->
 *
 * <p>
 * The following features are supported:
 * <ul>
 *   <li>{@link org.wesnoth.wml.WMLPreprocIF#getExpressions <em>Expressions</em>}</li>
 *   <li>{@link org.wesnoth.wml.WMLPreprocIF#getElses <em>Elses</em>}</li>
 *   <li>{@link org.wesnoth.wml.WMLPreprocIF#getElseExpressions <em>Else Expressions</em>}</li>
 *   <li>{@link org.wesnoth.wml.WMLPreprocIF#getEndName <em>End Name</em>}</li>
 * </ul>
 * </p>
 *
 * @see org.wesnoth.wml.WmlPackage#getWMLPreprocIF()
 * @model
 * @generated
 */
public interface WMLPreprocIF extends WMLRootExpression
{
  /**
   * Returns the value of the '<em><b>Expressions</b></em>' containment reference list.
   * The list contents are of type {@link org.wesnoth.wml.WMLValuedExpression}.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Expressions</em>' containment reference list isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Expressions</em>' containment reference list.
   * @see org.wesnoth.wml.WmlPackage#getWMLPreprocIF_Expressions()
   * @model containment="true"
   * @generated
   */
  EList<WMLValuedExpression> getExpressions();

  /**
   * Returns the value of the '<em><b>Elses</b></em>' attribute.
   * The default value is <code>""</code>.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Elses</em>' attribute isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Elses</em>' attribute.
   * @see #setElses(String)
   * @see org.wesnoth.wml.WmlPackage#getWMLPreprocIF_Elses()
   * @model default=""
   * @generated
   */
  String getElses();

  /**
   * Sets the value of the '{@link org.wesnoth.wml.WMLPreprocIF#getElses <em>Elses</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>Elses</em>' attribute.
   * @see #getElses()
   * @generated
   */
  void setElses(String value);

  /**
   * Returns the value of the '<em><b>Else Expressions</b></em>' containment reference list.
   * The list contents are of type {@link org.wesnoth.wml.WMLValuedExpression}.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Else Expressions</em>' containment reference list isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Else Expressions</em>' containment reference list.
   * @see org.wesnoth.wml.WmlPackage#getWMLPreprocIF_ElseExpressions()
   * @model containment="true"
   * @generated
   */
  EList<WMLValuedExpression> getElseExpressions();

  /**
   * Returns the value of the '<em><b>End Name</b></em>' attribute.
   * The default value is <code>""</code>.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>End Name</em>' attribute isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>End Name</em>' attribute.
   * @see #setEndName(String)
   * @see org.wesnoth.wml.WmlPackage#getWMLPreprocIF_EndName()
   * @model default=""
   * @generated
   */
  String getEndName();

  /**
   * Sets the value of the '{@link org.wesnoth.wml.WMLPreprocIF#getEndName <em>End Name</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>End Name</em>' attribute.
   * @see #getEndName()
   * @generated
   */
  void setEndName(String value);

} // WMLPreprocIF
