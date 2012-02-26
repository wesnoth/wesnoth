/**
 * <copyright>
 * </copyright>
 *

 */
package org.wesnoth.wml.impl;

import org.eclipse.emf.common.notify.Notification;

import org.eclipse.emf.ecore.EClass;

import org.eclipse.emf.ecore.impl.ENotificationImpl;

import org.wesnoth.wml.WMLExpression;
import org.wesnoth.wml.WMLKey;
import org.wesnoth.wml.WMLTag;
import org.wesnoth.wml.WmlPackage;

/**
 * <!-- begin-user-doc -->
 * An implementation of the model object '<em><b>WML Expression</b></em>'.
 * <!-- end-user-doc -->
 * <p>
 * The following features are implemented:
 * <ul>
 *   <li>{@link org.wesnoth.wml.impl.WMLExpressionImpl#getName <em>Name</em>}</li>
 *   <li>{@link org.wesnoth.wml.impl.WMLExpressionImpl#is_LuaBased <em>Lua Based</em>}</li>
 *   <li>{@link org.wesnoth.wml.impl.WMLExpressionImpl#get_DefinitionLocation <em>Definition Location</em>}</li>
 *   <li>{@link org.wesnoth.wml.impl.WMLExpressionImpl#get_DefinitionOffset <em>Definition Offset</em>}</li>
 *   <li>{@link org.wesnoth.wml.impl.WMLExpressionImpl#get_Cardinality <em>Cardinality</em>}</li>
 * </ul>
 * </p>
 *
 * @generated
 */
public class WMLExpressionImpl extends WMLValuedExpressionImpl implements WMLExpression
{
  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  private static final long serialVersionUID = 1L;

  /**
   * The default value of the '{@link #getName() <em>Name</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getName()
   * @generated
   * @ordered
   */
  protected static final String NAME_EDEFAULT = "";

  /**
   * The cached value of the '{@link #getName() <em>Name</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getName()
   * @generated
   * @ordered
   */
  protected String name = NAME_EDEFAULT;

  /**
   * The default value of the '{@link #is_LuaBased() <em>Lua Based</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #is_LuaBased()
   * @generated
   * @ordered
   */
  protected static final boolean _LUA_BASED_EDEFAULT = false;

  /**
   * The cached value of the '{@link #is_LuaBased() <em>Lua Based</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #is_LuaBased()
   * @generated
   * @ordered
   */
  protected boolean _LuaBased = _LUA_BASED_EDEFAULT;

  /**
   * The default value of the '{@link #get_DefinitionLocation() <em>Definition Location</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #get_DefinitionLocation()
   * @generated
   * @ordered
   */
  protected static final String _DEFINITION_LOCATION_EDEFAULT = "";

  /**
   * The cached value of the '{@link #get_DefinitionLocation() <em>Definition Location</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #get_DefinitionLocation()
   * @generated
   * @ordered
   */
  protected String _DefinitionLocation = _DEFINITION_LOCATION_EDEFAULT;

  /**
   * The default value of the '{@link #get_DefinitionOffset() <em>Definition Offset</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #get_DefinitionOffset()
   * @generated
   * @ordered
   */
  protected static final int _DEFINITION_OFFSET_EDEFAULT = 0;

  /**
   * The cached value of the '{@link #get_DefinitionOffset() <em>Definition Offset</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #get_DefinitionOffset()
   * @generated
   * @ordered
   */
  protected int _DefinitionOffset = _DEFINITION_OFFSET_EDEFAULT;

  /**
   * The default value of the '{@link #get_Cardinality() <em>Cardinality</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #get_Cardinality()
   * @generated
   * @ordered
   */
  protected static final char _CARDINALITY_EDEFAULT = ' ';

  /**
   * The cached value of the '{@link #get_Cardinality() <em>Cardinality</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #get_Cardinality()
   * @generated
   * @ordered
   */
  protected char _Cardinality = _CARDINALITY_EDEFAULT;

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  protected WMLExpressionImpl()
  {
    super();
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  @Override
  protected EClass eStaticClass()
  {
    return WmlPackage.Literals.WML_EXPRESSION;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public String getName()
  {
    return name;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public void setName(String newName)
  {
    String oldName = name;
    name = newName;
    if (eNotificationRequired())
      eNotify(new ENotificationImpl(this, Notification.SET, WmlPackage.WML_EXPRESSION__NAME, oldName, name));
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public boolean is_LuaBased()
  {
    return _LuaBased;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public void set_LuaBased(boolean new_LuaBased)
  {
    boolean old_LuaBased = _LuaBased;
    _LuaBased = new_LuaBased;
    if (eNotificationRequired())
      eNotify(new ENotificationImpl(this, Notification.SET, WmlPackage.WML_EXPRESSION__LUA_BASED, old_LuaBased, _LuaBased));
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public String get_DefinitionLocation()
  {
    return _DefinitionLocation;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public void set_DefinitionLocation(String new_DefinitionLocation)
  {
    String old_DefinitionLocation = _DefinitionLocation;
    _DefinitionLocation = new_DefinitionLocation;
    if (eNotificationRequired())
      eNotify(new ENotificationImpl(this, Notification.SET, WmlPackage.WML_EXPRESSION__DEFINITION_LOCATION, old_DefinitionLocation, _DefinitionLocation));
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public int get_DefinitionOffset()
  {
    return _DefinitionOffset;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public void set_DefinitionOffset(int new_DefinitionOffset)
  {
    int old_DefinitionOffset = _DefinitionOffset;
    _DefinitionOffset = new_DefinitionOffset;
    if (eNotificationRequired())
      eNotify(new ENotificationImpl(this, Notification.SET, WmlPackage.WML_EXPRESSION__DEFINITION_OFFSET, old_DefinitionOffset, _DefinitionOffset));
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public char get_Cardinality()
  {
    return _Cardinality;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public void set_Cardinality(char new_Cardinality)
  {
    char old_Cardinality = _Cardinality;
    _Cardinality = new_Cardinality;
    if (eNotificationRequired())
      eNotify(new ENotificationImpl(this, Notification.SET, WmlPackage.WML_EXPRESSION__CARDINALITY, old_Cardinality, _Cardinality));
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public boolean is_Required()
  {
    return _Cardinality == '1';
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public boolean is_Forbidden()
  {
    return _Cardinality == '-';
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public boolean is_Optional()
  {
    return _Cardinality == '?';
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public boolean is_Repeatable()
  {
    return _Cardinality == '*';
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public int getAllowedCount()
  {
    switch( _Cardinality ) {
                    case '-': return 0;
                    case '?': case '1':  return 1;
                }
                // by default let it be infinite times
                return Integer.MAX_VALUE;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public boolean isWMLTag()
  {
    return ( this instanceof WMLTag );
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public WMLTag asWMLTag()
  {
    if ( !( this instanceof WMLTag ) ) return null; return ( WMLTag ) this;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public boolean isWMLKey()
  {
    return ( this instanceof WMLKey );
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public WMLKey asWMLKey()
  {
    if ( !( this instanceof WMLKey ) ) return null; return ( WMLKey ) this;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  @Override
  public Object eGet(int featureID, boolean resolve, boolean coreType)
  {
    switch (featureID)
    {
      case WmlPackage.WML_EXPRESSION__NAME:
        return getName();
      case WmlPackage.WML_EXPRESSION__LUA_BASED:
        return is_LuaBased();
      case WmlPackage.WML_EXPRESSION__DEFINITION_LOCATION:
        return get_DefinitionLocation();
      case WmlPackage.WML_EXPRESSION__DEFINITION_OFFSET:
        return get_DefinitionOffset();
      case WmlPackage.WML_EXPRESSION__CARDINALITY:
        return get_Cardinality();
    }
    return super.eGet(featureID, resolve, coreType);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  @Override
  public void eSet(int featureID, Object newValue)
  {
    switch (featureID)
    {
      case WmlPackage.WML_EXPRESSION__NAME:
        setName((String)newValue);
        return;
      case WmlPackage.WML_EXPRESSION__LUA_BASED:
        set_LuaBased((Boolean)newValue);
        return;
      case WmlPackage.WML_EXPRESSION__DEFINITION_LOCATION:
        set_DefinitionLocation((String)newValue);
        return;
      case WmlPackage.WML_EXPRESSION__DEFINITION_OFFSET:
        set_DefinitionOffset((Integer)newValue);
        return;
      case WmlPackage.WML_EXPRESSION__CARDINALITY:
        set_Cardinality((Character)newValue);
        return;
    }
    super.eSet(featureID, newValue);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  @Override
  public void eUnset(int featureID)
  {
    switch (featureID)
    {
      case WmlPackage.WML_EXPRESSION__NAME:
        setName(NAME_EDEFAULT);
        return;
      case WmlPackage.WML_EXPRESSION__LUA_BASED:
        set_LuaBased(_LUA_BASED_EDEFAULT);
        return;
      case WmlPackage.WML_EXPRESSION__DEFINITION_LOCATION:
        set_DefinitionLocation(_DEFINITION_LOCATION_EDEFAULT);
        return;
      case WmlPackage.WML_EXPRESSION__DEFINITION_OFFSET:
        set_DefinitionOffset(_DEFINITION_OFFSET_EDEFAULT);
        return;
      case WmlPackage.WML_EXPRESSION__CARDINALITY:
        set_Cardinality(_CARDINALITY_EDEFAULT);
        return;
    }
    super.eUnset(featureID);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  @Override
  public boolean eIsSet(int featureID)
  {
    switch (featureID)
    {
      case WmlPackage.WML_EXPRESSION__NAME:
        return NAME_EDEFAULT == null ? name != null : !NAME_EDEFAULT.equals(name);
      case WmlPackage.WML_EXPRESSION__LUA_BASED:
        return _LuaBased != _LUA_BASED_EDEFAULT;
      case WmlPackage.WML_EXPRESSION__DEFINITION_LOCATION:
        return _DEFINITION_LOCATION_EDEFAULT == null ? _DefinitionLocation != null : !_DEFINITION_LOCATION_EDEFAULT.equals(_DefinitionLocation);
      case WmlPackage.WML_EXPRESSION__DEFINITION_OFFSET:
        return _DefinitionOffset != _DEFINITION_OFFSET_EDEFAULT;
      case WmlPackage.WML_EXPRESSION__CARDINALITY:
        return _Cardinality != _CARDINALITY_EDEFAULT;
    }
    return super.eIsSet(featureID);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  @Override
  public String toString()
  {
    if (eIsProxy()) return super.toString();

    StringBuffer result = new StringBuffer(super.toString());
    result.append(" (name: ");
    result.append(name);
    result.append(", _LuaBased: ");
    result.append(_LuaBased);
    result.append(", _DefinitionLocation: ");
    result.append(_DefinitionLocation);
    result.append(", _DefinitionOffset: ");
    result.append(_DefinitionOffset);
    result.append(", _Cardinality: ");
    result.append(_Cardinality);
    result.append(')');
    return result.toString();
  }

} //WMLExpressionImpl
