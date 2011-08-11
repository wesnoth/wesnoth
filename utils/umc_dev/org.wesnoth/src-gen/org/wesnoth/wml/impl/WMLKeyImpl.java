/**
 * <copyright>
 * </copyright>
 *

 */
package org.wesnoth.wml.impl;

import java.util.Collection;

import org.eclipse.emf.common.notify.Notification;
import org.eclipse.emf.common.notify.NotificationChain;

import org.eclipse.emf.common.util.EList;

import org.eclipse.emf.ecore.EClass;
import org.eclipse.emf.ecore.InternalEObject;

import org.eclipse.emf.ecore.impl.ENotificationImpl;

import org.eclipse.emf.ecore.util.EDataTypeEList;
import org.eclipse.emf.ecore.util.EObjectContainmentEList;
import org.eclipse.emf.ecore.util.InternalEList;

import org.wesnoth.wml.WMLKey;
import org.wesnoth.wml.WMLKeyValue;
import org.wesnoth.wml.WmlPackage;

/**
 * <!-- begin-user-doc -->
 * An implementation of the model object '<em><b>WML Key</b></em>'.
 * <!-- end-user-doc -->
 * <p>
 * The following features are implemented:
 * <ul>
 *   <li>{@link org.wesnoth.wml.impl.WMLKeyImpl#getValues <em>Values</em>}</li>
 *   <li>{@link org.wesnoth.wml.impl.WMLKeyImpl#getEol <em>Eol</em>}</li>
 *   <li>{@link org.wesnoth.wml.impl.WMLKeyImpl#is_Enum <em>Enum</em>}</li>
 *   <li>{@link org.wesnoth.wml.impl.WMLKeyImpl#is_Translatable <em>Translatable</em>}</li>
 *   <li>{@link org.wesnoth.wml.impl.WMLKeyImpl#get_DataType <em>Data Type</em>}</li>
 * </ul>
 * </p>
 *
 * @generated
 */
public class WMLKeyImpl extends WMLExpressionImpl implements WMLKey
{
  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  private static final long serialVersionUID = 1L;

  /**
   * The cached value of the '{@link #getValues() <em>Values</em>}' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getValues()
   * @generated
   * @ordered
   */
  protected EList<WMLKeyValue> values;

  /**
   * The cached value of the '{@link #getEol() <em>Eol</em>}' attribute list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getEol()
   * @generated
   * @ordered
   */
  protected EList<String> eol;

  /**
   * The default value of the '{@link #is_Enum() <em>Enum</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #is_Enum()
   * @generated
   * @ordered
   */
  protected static final boolean _ENUM_EDEFAULT = false;

  /**
   * The cached value of the '{@link #is_Enum() <em>Enum</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #is_Enum()
   * @generated
   * @ordered
   */
  protected boolean _Enum = _ENUM_EDEFAULT;

  /**
   * The default value of the '{@link #is_Translatable() <em>Translatable</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #is_Translatable()
   * @generated
   * @ordered
   */
  protected static final boolean _TRANSLATABLE_EDEFAULT = false;

  /**
   * The cached value of the '{@link #is_Translatable() <em>Translatable</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #is_Translatable()
   * @generated
   * @ordered
   */
  protected boolean _Translatable = _TRANSLATABLE_EDEFAULT;

  /**
   * The default value of the '{@link #get_DataType() <em>Data Type</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #get_DataType()
   * @generated
   * @ordered
   */
  protected static final String _DATA_TYPE_EDEFAULT = "";

  /**
   * The cached value of the '{@link #get_DataType() <em>Data Type</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #get_DataType()
   * @generated
   * @ordered
   */
  protected String _DataType = _DATA_TYPE_EDEFAULT;

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  protected WMLKeyImpl()
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
    return WmlPackage.Literals.WML_KEY;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EList<WMLKeyValue> getValues()
  {
    if (values == null)
    {
      values = new EObjectContainmentEList<WMLKeyValue>(WMLKeyValue.class, this, WmlPackage.WML_KEY__VALUES);
    }
    return values;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EList<String> getEol()
  {
    if (eol == null)
    {
      eol = new EDataTypeEList<String>(String.class, this, WmlPackage.WML_KEY__EOL);
    }
    return eol;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public boolean is_Enum()
  {
    return _Enum;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public void set_Enum(boolean new_Enum)
  {
    boolean old_Enum = _Enum;
    _Enum = new_Enum;
    if (eNotificationRequired())
      eNotify(new ENotificationImpl(this, Notification.SET, WmlPackage.WML_KEY__ENUM, old_Enum, _Enum));
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public boolean is_Translatable()
  {
    return _Translatable;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public void set_Translatable(boolean new_Translatable)
  {
    boolean old_Translatable = _Translatable;
    _Translatable = new_Translatable;
    if (eNotificationRequired())
      eNotify(new ENotificationImpl(this, Notification.SET, WmlPackage.WML_KEY__TRANSLATABLE, old_Translatable, _Translatable));
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public String get_DataType()
  {
    return _DataType;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public void set_DataType(String new_DataType)
  {
    String old_DataType = _DataType;
    _DataType = new_DataType;
    if (eNotificationRequired())
      eNotify(new ENotificationImpl(this, Notification.SET, WmlPackage.WML_KEY__DATA_TYPE, old_DataType, _DataType));
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public String getValue()
  {
     return org.wesnoth.utils.WMLUtils.getKeyValue( getValues( ) );
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  @Override
  public NotificationChain eInverseRemove(InternalEObject otherEnd, int featureID, NotificationChain msgs)
  {
    switch (featureID)
    {
      case WmlPackage.WML_KEY__VALUES:
        return ((InternalEList<?>)getValues()).basicRemove(otherEnd, msgs);
    }
    return super.eInverseRemove(otherEnd, featureID, msgs);
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
      case WmlPackage.WML_KEY__VALUES:
        return getValues();
      case WmlPackage.WML_KEY__EOL:
        return getEol();
      case WmlPackage.WML_KEY__ENUM:
        return is_Enum();
      case WmlPackage.WML_KEY__TRANSLATABLE:
        return is_Translatable();
      case WmlPackage.WML_KEY__DATA_TYPE:
        return get_DataType();
    }
    return super.eGet(featureID, resolve, coreType);
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  @SuppressWarnings("unchecked")
  @Override
  public void eSet(int featureID, Object newValue)
  {
    switch (featureID)
    {
      case WmlPackage.WML_KEY__VALUES:
        getValues().clear();
        getValues().addAll((Collection<? extends WMLKeyValue>)newValue);
        return;
      case WmlPackage.WML_KEY__EOL:
        getEol().clear();
        getEol().addAll((Collection<? extends String>)newValue);
        return;
      case WmlPackage.WML_KEY__ENUM:
        set_Enum((Boolean)newValue);
        return;
      case WmlPackage.WML_KEY__TRANSLATABLE:
        set_Translatable((Boolean)newValue);
        return;
      case WmlPackage.WML_KEY__DATA_TYPE:
        set_DataType((String)newValue);
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
      case WmlPackage.WML_KEY__VALUES:
        getValues().clear();
        return;
      case WmlPackage.WML_KEY__EOL:
        getEol().clear();
        return;
      case WmlPackage.WML_KEY__ENUM:
        set_Enum(_ENUM_EDEFAULT);
        return;
      case WmlPackage.WML_KEY__TRANSLATABLE:
        set_Translatable(_TRANSLATABLE_EDEFAULT);
        return;
      case WmlPackage.WML_KEY__DATA_TYPE:
        set_DataType(_DATA_TYPE_EDEFAULT);
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
      case WmlPackage.WML_KEY__VALUES:
        return values != null && !values.isEmpty();
      case WmlPackage.WML_KEY__EOL:
        return eol != null && !eol.isEmpty();
      case WmlPackage.WML_KEY__ENUM:
        return _Enum != _ENUM_EDEFAULT;
      case WmlPackage.WML_KEY__TRANSLATABLE:
        return _Translatable != _TRANSLATABLE_EDEFAULT;
      case WmlPackage.WML_KEY__DATA_TYPE:
        return _DATA_TYPE_EDEFAULT == null ? _DataType != null : !_DATA_TYPE_EDEFAULT.equals(_DataType);
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
    result.append(" (eol: ");
    result.append(eol);
    result.append(", _Enum: ");
    result.append(_Enum);
    result.append(", _Translatable: ");
    result.append(_Translatable);
    result.append(", _DataType: ");
    result.append(_DataType);
    result.append(')');
    return result.toString();
  }

} //WMLKeyImpl
