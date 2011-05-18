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
import org.eclipse.emf.ecore.EObject;
import org.eclipse.emf.ecore.InternalEObject;

import org.eclipse.emf.ecore.impl.ENotificationImpl;

import org.eclipse.emf.ecore.util.EObjectContainmentEList;
import org.eclipse.emf.ecore.util.InternalEList;

import org.wesnoth.wml.WMLMacroCall;
import org.wesnoth.wml.WmlPackage;

/**
 * <!-- begin-user-doc -->
 * An implementation of the model object '<em><b>WML Macro Call</b></em>'.
 * <!-- end-user-doc -->
 * <p>
 * The following features are implemented:
 * <ul>
 *   <li>{@link org.wesnoth.wml.impl.WMLMacroCallImpl#isPoint <em>Point</em>}</li>
 *   <li>{@link org.wesnoth.wml.impl.WMLMacroCallImpl#isRelative <em>Relative</em>}</li>
 *   <li>{@link org.wesnoth.wml.impl.WMLMacroCallImpl#getName <em>Name</em>}</li>
 *   <li>{@link org.wesnoth.wml.impl.WMLMacroCallImpl#getParams <em>Params</em>}</li>
 *   <li>{@link org.wesnoth.wml.impl.WMLMacroCallImpl#getExtraMacros <em>Extra Macros</em>}</li>
 * </ul>
 * </p>
 *
 * @generated
 */
public class WMLMacroCallImpl extends WMLKeyValueImpl implements WMLMacroCall
{
  /**
   * The default value of the '{@link #isPoint() <em>Point</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #isPoint()
   * @generated
   * @ordered
   */
  protected static final boolean POINT_EDEFAULT = false;

  /**
   * The cached value of the '{@link #isPoint() <em>Point</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #isPoint()
   * @generated
   * @ordered
   */
  protected boolean point = POINT_EDEFAULT;

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
    return WmlPackage.Literals.WML_MACRO_CALL;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public boolean isPoint()
  {
    return point;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public void setPoint(boolean newPoint)
  {
    boolean oldPoint = point;
    point = newPoint;
    if (eNotificationRequired())
      eNotify(new ENotificationImpl(this, Notification.SET, WmlPackage.WML_MACRO_CALL__POINT, oldPoint, point));
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
      eNotify(new ENotificationImpl(this, Notification.SET, WmlPackage.WML_MACRO_CALL__RELATIVE, oldRelative, relative));
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
      eNotify(new ENotificationImpl(this, Notification.SET, WmlPackage.WML_MACRO_CALL__NAME, oldName, name));
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
      params = new EObjectContainmentEList<EObject>(EObject.class, this, WmlPackage.WML_MACRO_CALL__PARAMS);
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
      extraMacros = new EObjectContainmentEList<WMLMacroCall>(WMLMacroCall.class, this, WmlPackage.WML_MACRO_CALL__EXTRA_MACROS);
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
      case WmlPackage.WML_MACRO_CALL__PARAMS:
        return ((InternalEList<?>)getParams()).basicRemove(otherEnd, msgs);
      case WmlPackage.WML_MACRO_CALL__EXTRA_MACROS:
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
      case WmlPackage.WML_MACRO_CALL__POINT:
        return isPoint();
      case WmlPackage.WML_MACRO_CALL__RELATIVE:
        return isRelative();
      case WmlPackage.WML_MACRO_CALL__NAME:
        return getName();
      case WmlPackage.WML_MACRO_CALL__PARAMS:
        return getParams();
      case WmlPackage.WML_MACRO_CALL__EXTRA_MACROS:
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
      case WmlPackage.WML_MACRO_CALL__POINT:
        setPoint((Boolean)newValue);
        return;
      case WmlPackage.WML_MACRO_CALL__RELATIVE:
        setRelative((Boolean)newValue);
        return;
      case WmlPackage.WML_MACRO_CALL__NAME:
        setName((String)newValue);
        return;
      case WmlPackage.WML_MACRO_CALL__PARAMS:
        getParams().clear();
        getParams().addAll((Collection<? extends EObject>)newValue);
        return;
      case WmlPackage.WML_MACRO_CALL__EXTRA_MACROS:
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
      case WmlPackage.WML_MACRO_CALL__POINT:
        setPoint(POINT_EDEFAULT);
        return;
      case WmlPackage.WML_MACRO_CALL__RELATIVE:
        setRelative(RELATIVE_EDEFAULT);
        return;
      case WmlPackage.WML_MACRO_CALL__NAME:
        setName(NAME_EDEFAULT);
        return;
      case WmlPackage.WML_MACRO_CALL__PARAMS:
        getParams().clear();
        return;
      case WmlPackage.WML_MACRO_CALL__EXTRA_MACROS:
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
      case WmlPackage.WML_MACRO_CALL__POINT:
        return point != POINT_EDEFAULT;
      case WmlPackage.WML_MACRO_CALL__RELATIVE:
        return relative != RELATIVE_EDEFAULT;
      case WmlPackage.WML_MACRO_CALL__NAME:
        return NAME_EDEFAULT == null ? name != null : !NAME_EDEFAULT.equals(name);
      case WmlPackage.WML_MACRO_CALL__PARAMS:
        return params != null && !params.isEmpty();
      case WmlPackage.WML_MACRO_CALL__EXTRA_MACROS:
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
    result.append(" (point: ");
    result.append(point);
    result.append(", relative: ");
    result.append(relative);
    result.append(", name: ");
    result.append(name);
    result.append(')');
    return result.toString();
  }

} //WMLMacroCallImpl
