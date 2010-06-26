/**
 * <copyright>
 * </copyright>
 *
 */
package org.wesnoth.wML.impl;

import org.eclipse.emf.common.notify.Notification;

import org.eclipse.emf.ecore.EClass;

import org.eclipse.emf.ecore.impl.ENotificationImpl;

import org.wesnoth.wML.Macro;
import org.wesnoth.wML.WMLPackage;

/**
 * <!-- begin-user-doc -->
 * An implementation of the model object '<em><b>Macro</b></em>'.
 * <!-- end-user-doc -->
 * <p>
 * The following features are implemented:
 * <ul>
 *   <li>{@link org.wesnoth.wML.impl.MacroImpl#getMacroName <em>Macro Name</em>}</li>
 * </ul>
 * </p>
 *
 * @generated
 */
public class MacroImpl extends PreprocessorImpl implements Macro
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
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  protected MacroImpl()
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
    return WMLPackage.Literals.MACRO;
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
      eNotify(new ENotificationImpl(this, Notification.SET, WMLPackage.MACRO__MACRO_NAME, oldMacroName, macroName));
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
      case WMLPackage.MACRO__MACRO_NAME:
        return getMacroName();
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
      case WMLPackage.MACRO__MACRO_NAME:
        setMacroName((String)newValue);
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
      case WMLPackage.MACRO__MACRO_NAME:
        setMacroName(MACRO_NAME_EDEFAULT);
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
      case WMLPackage.MACRO__MACRO_NAME:
        return MACRO_NAME_EDEFAULT == null ? macroName != null : !MACRO_NAME_EDEFAULT.equals(macroName);
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
    result.append(')');
    return result.toString();
  }

} //MacroImpl
