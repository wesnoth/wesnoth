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
 *   <li>{@link org.wesnoth.wml.WMLExpression#is_LuaBased <em>Lua Based</em>}</li>
 *   <li>{@link org.wesnoth.wml.WMLExpression#get_DefinitionLocation <em>Definition Location</em>}</li>
 *   <li>{@link org.wesnoth.wml.WMLExpression#get_DefinitionOffset <em>Definition Offset</em>}</li>
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
   * Returns the value of the '<em><b>Lua Based</b></em>' attribute.
   * The default value is <code>"false"</code>.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Lua Based</em>' attribute isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Lua Based</em>' attribute.
   * @see #set_LuaBased(boolean)
   * @see org.wesnoth.wml.WmlPackage#getWMLExpression__LuaBased()
   * @model default="false"
   * @generated
   */
  boolean is_LuaBased();

  /**
   * Sets the value of the '{@link org.wesnoth.wml.WMLExpression#is_LuaBased <em>Lua Based</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>Lua Based</em>' attribute.
   * @see #is_LuaBased()
   * @generated
   */
  void set_LuaBased(boolean value);

  /**
   * Returns the value of the '<em><b>Definition Location</b></em>' attribute.
   * The default value is <code>""</code>.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Definition Location</em>' attribute isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Definition Location</em>' attribute.
   * @see #set_DefinitionLocation(String)
   * @see org.wesnoth.wml.WmlPackage#getWMLExpression__DefinitionLocation()
   * @model default=""
   * @generated
   */
  String get_DefinitionLocation();

  /**
   * Sets the value of the '{@link org.wesnoth.wml.WMLExpression#get_DefinitionLocation <em>Definition Location</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>Definition Location</em>' attribute.
   * @see #get_DefinitionLocation()
   * @generated
   */
  void set_DefinitionLocation(String value);

  /**
   * Returns the value of the '<em><b>Definition Offset</b></em>' attribute.
   * The default value is <code>"0"</code>.
   * <!-- begin-user-doc -->
   * <p>
   * If the meaning of the '<em>Definition Offset</em>' attribute isn't clear,
   * there really should be more of a description here...
   * </p>
   * <!-- end-user-doc -->
   * @return the value of the '<em>Definition Offset</em>' attribute.
   * @see #set_DefinitionOffset(int)
   * @see org.wesnoth.wml.WmlPackage#getWMLExpression__DefinitionOffset()
   * @model default="0"
   * @generated
   */
  int get_DefinitionOffset();

  /**
   * Sets the value of the '{@link org.wesnoth.wml.WMLExpression#get_DefinitionOffset <em>Definition Offset</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @param value the new value of the '<em>Definition Offset</em>' attribute.
   * @see #get_DefinitionOffset()
   * @generated
   */
  void set_DefinitionOffset(int value);

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
   * @model annotation="http://www.eclipse.org/emf/2002/GenModel body='return _Cardinality == \'1\';'"
   * @generated
   */
  boolean is_Required();

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @model annotation="http://www.eclipse.org/emf/2002/GenModel body='return _Cardinality == \'-\';'"
   * @generated
   */
  boolean is_Forbidden();

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @model annotation="http://www.eclipse.org/emf/2002/GenModel body='return _Cardinality == \'?\';'"
   * @generated
   */
  boolean is_Optional();

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @model annotation="http://www.eclipse.org/emf/2002/GenModel body='return _Cardinality == \'*\';'"
   * @generated
   */
  boolean is_Repeatable();

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @model kind="operation"
   *        annotation="http://www.eclipse.org/emf/2002/GenModel body='switch( _Cardinality ) {\n                case \'-\': return 0;\n                case \'?\': case \'1\':  return 1;\n            }\n            // by default let it be infinite times\n            return Integer.MAX_VALUE;'"
   * @generated
   */
  int getAllowedCount();

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
