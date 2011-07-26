/**
 * <copyright>
 * </copyright>
 *

 */
package org.wesnoth.wml;


/**
 * <!-- begin-user-doc -->
 * A representation of the model object '<em><b>WML Expression</b></em>'.
 * <!-- end-user-doc -->
 *
 * <p>
 * The following features are supported:
 * <ul>
 *   <li>{@link org.wesnoth.wml.WMLExpression#getName <em>Name</em>}</li>
 *   <li>{@link org.wesnoth.wml.WMLExpression#get_Cardinality <em>Cardinality</em>}</li>
 * </ul>
 * </p>
 *
 * @see org.wesnoth.wml.WmlPackage#getWMLExpression()
 * @model
 * @generated
 */
public interface WMLExpression extends WMLValuedExpression
{
  /**
   * Returns the value of the '<em><b>Name</b></em>' attribute.
   * The default value is <code>""</code>.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Name</em>' attribute isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Name</em>' attribute.
   * @see #setName(String)
   * @see org.wesnoth.wml.WmlPackage#getWMLExpression_Name()
   * @model default=""
   * @generated
   */
  String getName();

  /**
   * Sets the value of the '{@link org.wesnoth.wml.WMLExpression#getName <em>Name</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>Name</em>' attribute.
   * @see #getName()
   * @generated
   */
  void setName(String value);

  /**
   * Returns the value of the '<em><b>Cardinality</b></em>' attribute.
   * The default value is <code>" "</code>.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Cardinality</em>' attribute isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Cardinality</em>' attribute.
   * @see #set_Cardinality(char)
   * @see org.wesnoth.wml.WmlPackage#getWMLExpression__Cardinality()
   * @model default=" "
   * @generated
   */
  char get_Cardinality();

  /**
   * Sets the value of the '{@link org.wesnoth.wml.WMLExpression#get_Cardinality <em>Cardinality</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>Cardinality</em>' attribute.
   * @see #get_Cardinality()
   * @generated
   */
  void set_Cardinality(char value);

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @model kind="operation"
   *        annotation="http://www.eclipse.org/emf/2002/GenModel body='return ( this instanceof WMLTag );'"
   * @generated
   */
  boolean isWMLTag();

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @model annotation="http://www.eclipse.org/emf/2002/GenModel body='if ( !( this instanceof WMLTag ) ) return null; return ( WMLTag ) this;'"
   * @generated
   */
  WMLTag asWMLTag();

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @model kind="operation"
   *        annotation="http://www.eclipse.org/emf/2002/GenModel body='return ( this instanceof WMLKey );'"
   * @generated
   */
  boolean isWMLKey();

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @model annotation="http://www.eclipse.org/emf/2002/GenModel body='if ( !( this instanceof WMLKey ) ) return null; return ( WMLKey ) this;'"
   * @generated
   */
  WMLKey asWMLKey();

} // WMLExpression
