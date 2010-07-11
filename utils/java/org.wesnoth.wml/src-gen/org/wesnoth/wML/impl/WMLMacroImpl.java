/**
 * <copyright>
 * </copyright>
 *

 */
package org.wesnoth.wML.impl;

import java.util.Collection;

import org.eclipse.emf.common.notify.Notification;

import org.eclipse.emf.common.util.EList;

import org.eclipse.emf.ecore.EClass;

import org.eclipse.emf.ecore.impl.ENotificationImpl;
import org.eclipse.emf.ecore.impl.MinimalEObjectImpl;

import org.eclipse.emf.ecore.util.EDataTypeEList;

import org.wesnoth.wML.WMLMacro;
import org.wesnoth.wML.WMLPackage;

/**
 * <!-- begin-user-doc -->
 * An implementation of the model object '<em><b>Macro</b></em>'.
 * <!-- end-user-doc -->
 * <p>
 * The following features are implemented:
 * <ul>
 *   <li>{@link org.wesnoth.wML.impl.WMLMacroImpl#getMacroName <em>Macro Name</em>}</li>
 *   <li>{@link org.wesnoth.wML.impl.WMLMacroImpl#getTagcontent <em>Tagcontent</em>}</li>
 * </ul>
 * </p>
 *
 * @generated
 */
public class WMLMacroImpl extends MinimalEObjectImpl.Container implements WMLMacro
{
  /**
   * The default value of the '{@link #getMacroName() <em>Macro Name</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getMacroName()
   * @generated
   * @ordered
   */
  protected static final String MACRO_NAME_EDEFAULT = null;

  /**
   * The cached value of the '{@link #getMacroName() <em>Macro Name</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getMacroName()
   * @generated
   * @ordered
   */
  protected String macroName = MACRO_NAME_EDEFAULT;

  /**
   * The cached value of the '{@link #getTagcontent() <em>Tagcontent</em>}' attribute list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getTagcontent()
   * @generated
   * @ordered
   */
  protected EList<String> tagcontent;

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  protected WMLMacroImpl()
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
    return WMLPackage.Literals.WML_MACRO;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public String getMacroName()
  {
    return macroName;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public void setMacroName(String newMacroName)
  {
    String oldMacroName = macroName;
    macroName = newMacroName;
    if (eNotificationRequired())
      eNotify(new ENotificationImpl(this, Notification.SET, WMLPackage.WML_MACRO__MACRO_NAME, oldMacroName, macroName));
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EList<String> getTagcontent()
  {
    if (tagcontent == null)
    {
      tagcontent = new EDataTypeEList<String>(String.class, this, WMLPackage.WML_MACRO__TAGCONTENT);
    }
    return tagcontent;
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
      case WMLPackage.WML_MACRO__MACRO_NAME:
        return getMacroName();
      case WMLPackage.WML_MACRO__TAGCONTENT:
        return getTagcontent();
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
      case WMLPackage.WML_MACRO__MACRO_NAME:
        setMacroName((String)newValue);
        return;
      case WMLPackage.WML_MACRO__TAGCONTENT:
        getTagcontent().clear();
        getTagcontent().addAll((Collection<? extends String>)newValue);
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
      case WMLPackage.WML_MACRO__MACRO_NAME:
        setMacroName(MACRO_NAME_EDEFAULT);
        return;
      case WMLPackage.WML_MACRO__TAGCONTENT:
        getTagcontent().clear();
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
      case WMLPackage.WML_MACRO__MACRO_NAME:
        return MACRO_NAME_EDEFAULT == null ? macroName != null : !MACRO_NAME_EDEFAULT.equals(macroName);
      case WMLPackage.WML_MACRO__TAGCONTENT:
        return tagcontent != null && !tagcontent.isEmpty();
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
    result.append(" (macroName: ");
    result.append(macroName);
    result.append(", tagcontent: ");
    result.append(tagcontent);
    result.append(')');
    return result.toString();
  }

} //WMLMacroImpl
