/**
 * <copyright>
 * </copyright>
 *

 */
package org.wesnoth.wML.impl;

import java.util.Collection;

import org.eclipse.emf.common.notify.Notification;
import org.eclipse.emf.common.notify.NotificationChain;

import org.eclipse.emf.common.util.EList;

import org.eclipse.emf.ecore.EClass;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.emf.ecore.InternalEObject;

import org.eclipse.emf.ecore.impl.ENotificationImpl;

import org.eclipse.emf.ecore.util.EObjectContainmentEList;
import org.eclipse.emf.ecore.util.InternalEList;

import org.wesnoth.wML.WMLMacroCall;
import org.wesnoth.wML.WMLPackage;

/**
 * <!-- begin-user-doc -->
 * An implementation of the model object '<em><b>Macro Call</b></em>'.
 * <!-- end-user-doc -->
 * <p>
 * The following features are implemented:
 * <ul>
 *   <li>{@link org.wesnoth.wML.impl.WMLMacroCallImpl#isRelative <em>Relative</em>}</li>
 *   <li>{@link org.wesnoth.wML.impl.WMLMacroCallImpl#getName <em>Name</em>}</li>
 *   <li>{@link org.wesnoth.wML.impl.WMLMacroCallImpl#getParams <em>Params</em>}</li>
 *   <li>{@link org.wesnoth.wML.impl.WMLMacroCallImpl#getExtraMacros <em>Extra Macros</em>}</li>
 * </ul>
 * </p>
 *
 * @generated
 */
public class WMLMacroCallImpl extends WMLKeyValueImpl implements WMLMacroCall
{
  /**
   * The default value of the '{@link #isRelative() <em>Relative</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #isRelative()
   * @generated
   * @ordered
   */
  protected static final boolean RELATIVE_EDEFAULT = false;

  /**
   * The cached value of the '{@link #isRelative() <em>Relative</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #isRelative()
   * @generated
   * @ordered
   */
  protected boolean relative = RELATIVE_EDEFAULT;

  /**
   * The default value of the '{@link #getName() <em>Name</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getName()
   * @generated
   * @ordered
   */
  protected static final String NAME_EDEFAULT = null;

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
   * The cached value of the '{@link #getParams() <em>Params</em>}' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getParams()
   * @generated
   * @ordered
   */
  protected EList<EObject> params;

  /**
   * The cached value of the '{@link #getExtraMacros() <em>Extra Macros</em>}' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getExtraMacros()
   * @generated
   * @ordered
   */
  protected EList<WMLMacroCall> extraMacros;

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  protected WMLMacroCallImpl()
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
    return WMLPackage.Literals.WML_MACRO_CALL;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public boolean isRelative()
  {
    return relative;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public void setRelative(boolean newRelative)
  {
    boolean oldRelative = relative;
    relative = newRelative;
    if (eNotificationRequired())
      eNotify(new ENotificationImpl(this, Notification.SET, WMLPackage.WML_MACRO_CALL__RELATIVE, oldRelative, relative));
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
      eNotify(new ENotificationImpl(this, Notification.SET, WMLPackage.WML_MACRO_CALL__NAME, oldName, name));
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EList<EObject> getParams()
  {
    if (params == null)
    {
      params = new EObjectContainmentEList<EObject>(EObject.class, this, WMLPackage.WML_MACRO_CALL__PARAMS);
    }
    return params;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EList<WMLMacroCall> getExtraMacros()
  {
    if (extraMacros == null)
    {
      extraMacros = new EObjectContainmentEList<WMLMacroCall>(WMLMacroCall.class, this, WMLPackage.WML_MACRO_CALL__EXTRA_MACROS);
    }
    return extraMacros;
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
      case WMLPackage.WML_MACRO_CALL__PARAMS:
        return ((InternalEList<?>)getParams()).basicRemove(otherEnd, msgs);
      case WMLPackage.WML_MACRO_CALL__EXTRA_MACROS:
        return ((InternalEList<?>)getExtraMacros()).basicRemove(otherEnd, msgs);
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
      case WMLPackage.WML_MACRO_CALL__RELATIVE:
        return isRelative();
      case WMLPackage.WML_MACRO_CALL__NAME:
        return getName();
      case WMLPackage.WML_MACRO_CALL__PARAMS:
        return getParams();
      case WMLPackage.WML_MACRO_CALL__EXTRA_MACROS:
        return getExtraMacros();
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
      case WMLPackage.WML_MACRO_CALL__RELATIVE:
        setRelative((Boolean)newValue);
        return;
      case WMLPackage.WML_MACRO_CALL__NAME:
        setName((String)newValue);
        return;
      case WMLPackage.WML_MACRO_CALL__PARAMS:
        getParams().clear();
        getParams().addAll((Collection<? extends EObject>)newValue);
        return;
      case WMLPackage.WML_MACRO_CALL__EXTRA_MACROS:
        getExtraMacros().clear();
        getExtraMacros().addAll((Collection<? extends WMLMacroCall>)newValue);
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
      case WMLPackage.WML_MACRO_CALL__RELATIVE:
        setRelative(RELATIVE_EDEFAULT);
        return;
      case WMLPackage.WML_MACRO_CALL__NAME:
        setName(NAME_EDEFAULT);
        return;
      case WMLPackage.WML_MACRO_CALL__PARAMS:
        getParams().clear();
        return;
      case WMLPackage.WML_MACRO_CALL__EXTRA_MACROS:
        getExtraMacros().clear();
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
      case WMLPackage.WML_MACRO_CALL__RELATIVE:
        return relative != RELATIVE_EDEFAULT;
      case WMLPackage.WML_MACRO_CALL__NAME:
        return NAME_EDEFAULT == null ? name != null : !NAME_EDEFAULT.equals(name);
      case WMLPackage.WML_MACRO_CALL__PARAMS:
        return params != null && !params.isEmpty();
      case WMLPackage.WML_MACRO_CALL__EXTRA_MACROS:
        return extraMacros != null && !extraMacros.isEmpty();
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
    result.append(" (relative: ");
    result.append(relative);
    result.append(", name: ");
    result.append(name);
    result.append(')');
    return result.toString();
  }

} //WMLMacroCallImpl
