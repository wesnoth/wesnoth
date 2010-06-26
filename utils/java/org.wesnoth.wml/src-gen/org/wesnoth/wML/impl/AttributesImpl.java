/**
 * <copyright>
 * </copyright>
 *
 */
package org.wesnoth.wML.impl;

import org.eclipse.emf.common.notify.Notification;

import org.eclipse.emf.ecore.EClass;

import org.eclipse.emf.ecore.impl.ENotificationImpl;
import org.eclipse.emf.ecore.impl.MinimalEObjectImpl;

import org.wesnoth.wML.Attributes;
import org.wesnoth.wML.WMLPackage;

/**
 * <!-- begin-user-doc -->
 * An implementation of the model object '<em><b>Attributes</b></em>'.
 * <!-- end-user-doc -->
 * <p>
 * The following features are implemented:
 * <ul>
 *   <li>{@link org.wesnoth.wML.impl.AttributesImpl#getAttrName <em>Attr Name</em>}</li>
 *   <li>{@link org.wesnoth.wML.impl.AttributesImpl#getAttrValue <em>Attr Value</em>}</li>
 * </ul>
 * </p>
 *
 * @generated
 */
public class AttributesImpl extends MinimalEObjectImpl.Container implements Attributes
{
  /**
   * The default value of the '{@link #getAttrName() <em>Attr Name</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getAttrName()
   * @generated
   * @ordered
   */
  protected static final String ATTR_NAME_EDEFAULT = null;

  /**
   * The cached value of the '{@link #getAttrName() <em>Attr Name</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getAttrName()
   * @generated
   * @ordered
   */
  protected String attrName = ATTR_NAME_EDEFAULT;

  /**
   * The default value of the '{@link #getAttrValue() <em>Attr Value</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getAttrValue()
   * @generated
   * @ordered
   */
  protected static final String ATTR_VALUE_EDEFAULT = null;

  /**
   * The cached value of the '{@link #getAttrValue() <em>Attr Value</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getAttrValue()
   * @generated
   * @ordered
   */
  protected String attrValue = ATTR_VALUE_EDEFAULT;

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  protected AttributesImpl()
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
    return WMLPackage.Literals.ATTRIBUTES;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public String getAttrName()
  {
    return attrName;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public void setAttrName(String newAttrName)
  {
    String oldAttrName = attrName;
    attrName = newAttrName;
    if (eNotificationRequired())
      eNotify(new ENotificationImpl(this, Notification.SET, WMLPackage.ATTRIBUTES__ATTR_NAME, oldAttrName, attrName));
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public String getAttrValue()
  {
    return attrValue;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public void setAttrValue(String newAttrValue)
  {
    String oldAttrValue = attrValue;
    attrValue = newAttrValue;
    if (eNotificationRequired())
      eNotify(new ENotificationImpl(this, Notification.SET, WMLPackage.ATTRIBUTES__ATTR_VALUE, oldAttrValue, attrValue));
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
      case WMLPackage.ATTRIBUTES__ATTR_NAME:
        return getAttrName();
      case WMLPackage.ATTRIBUTES__ATTR_VALUE:
        return getAttrValue();
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
      case WMLPackage.ATTRIBUTES__ATTR_NAME:
        setAttrName((String)newValue);
        return;
      case WMLPackage.ATTRIBUTES__ATTR_VALUE:
        setAttrValue((String)newValue);
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
      case WMLPackage.ATTRIBUTES__ATTR_NAME:
        setAttrName(ATTR_NAME_EDEFAULT);
        return;
      case WMLPackage.ATTRIBUTES__ATTR_VALUE:
        setAttrValue(ATTR_VALUE_EDEFAULT);
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
      case WMLPackage.ATTRIBUTES__ATTR_NAME:
        return ATTR_NAME_EDEFAULT == null ? attrName != null : !ATTR_NAME_EDEFAULT.equals(attrName);
      case WMLPackage.ATTRIBUTES__ATTR_VALUE:
        return ATTR_VALUE_EDEFAULT == null ? attrValue != null : !ATTR_VALUE_EDEFAULT.equals(attrValue);
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
    result.append(" (attrName: ");
    result.append(attrName);
    result.append(", attrValue: ");
    result.append(attrValue);
    result.append(')');
    return result.toString();
  }

} //AttributesImpl
