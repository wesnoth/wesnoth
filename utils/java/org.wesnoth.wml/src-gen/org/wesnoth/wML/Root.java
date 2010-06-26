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
 * A representation of the model object '<em><b>Root</b></em>'.
 * <!-- end-user-doc -->
 *
 * <p>
 * The following features are supported:
 * <ul>
 *   <li>{@link org.wesnoth.wML.Root#getTextdomains <em>Textdomains</em>}</li>
 *   <li>{@link org.wesnoth.wML.Root#getPreproc <em>Preproc</em>}</li>
 *   <li>{@link org.wesnoth.wML.Root#getRoots <em>Roots</em>}</li>
 * </ul>
 * </p>
 *
 * @see org.wesnoth.wML.WMLPackage#getRoot()
 * @model
 * @generated
 */
public interface Root extends EObject
{
  /**
   * Returns the value of the '<em><b>Textdomains</b></em>' containment reference list.
   * The list contents are of type {@link org.wesnoth.wML.TextDomain}.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Textdomains</em>' containment reference list isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Textdomains</em>' containment reference list.
   * @see org.wesnoth.wML.WMLPackage#getRoot_Textdomains()
   * @model containment="true"
   * @generated
   */
  EList<TextDomain> getTextdomains();

  /**
   * Returns the value of the '<em><b>Preproc</b></em>' containment reference list.
   * The list contents are of type {@link org.wesnoth.wML.Preprocessor}.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Preproc</em>' containment reference list isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Preproc</em>' containment reference list.
   * @see org.wesnoth.wML.WMLPackage#getRoot_Preproc()
   * @model containment="true"
   * @generated
   */
  EList<Preprocessor> getPreproc();

  /**
   * Returns the value of the '<em><b>Roots</b></em>' containment reference list.
   * The list contents are of type {@link org.wesnoth.wML.RootType}.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Roots</em>' containment reference list isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Roots</em>' containment reference list.
   * @see org.wesnoth.wML.WMLPackage#getRoot_Roots()
   * @model containment="true"
   * @generated
   */
  EList<RootType> getRoots();

} // Root
