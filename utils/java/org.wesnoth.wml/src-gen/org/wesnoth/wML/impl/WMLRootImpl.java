/**
 * <copyright>
 * </copyright>
 *

 */
package org.wesnoth.wML.impl;

import java.util.Collection;

import org.eclipse.emf.common.notify.NotificationChain;

import org.eclipse.emf.common.util.EList;

import org.eclipse.emf.ecore.EClass;
import org.eclipse.emf.ecore.InternalEObject;

import org.eclipse.emf.ecore.impl.MinimalEObjectImpl;

import org.eclipse.emf.ecore.util.EObjectContainmentEList;
import org.eclipse.emf.ecore.util.InternalEList;

import org.wesnoth.wML.WMLMacro;
import org.wesnoth.wML.WMLPackage;
import org.wesnoth.wML.WMLRoot;
import org.wesnoth.wML.WMLTag;

/**
 * <!-- begin-user-doc -->
 * An implementation of the model object '<em><b>Root</b></em>'.
 * <!-- end-user-doc -->
 * <p>
 * The following features are implemented:
 * <ul>
 *   <li>{@link org.wesnoth.wML.impl.WMLRootImpl#getRtags <em>Rtags</em>}</li>
 *   <li>{@link org.wesnoth.wML.impl.WMLRootImpl#getRmacros <em>Rmacros</em>}</li>
 * </ul>
 * </p>
 *
 * @generated
 */
public class WMLRootImpl extends MinimalEObjectImpl.Container implements WMLRoot
{
  /**
   * The cached value of the '{@link #getRtags() <em>Rtags</em>}' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getRtags()
   * @generated
   * @ordered
   */
  protected EList<WMLTag> rtags;

  /**
   * The cached value of the '{@link #getRmacros() <em>Rmacros</em>}' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getRmacros()
   * @generated
   * @ordered
   */
  protected EList<WMLMacro> rmacros;

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  protected WMLRootImpl()
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
    return WMLPackage.Literals.WML_ROOT;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EList<WMLTag> getRtags()
  {
    if (rtags == null)
    {
      rtags = new EObjectContainmentEList<WMLTag>(WMLTag.class, this, WMLPackage.WML_ROOT__RTAGS);
    }
    return rtags;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EList<WMLMacro> getRmacros()
  {
    if (rmacros == null)
    {
      rmacros = new EObjectContainmentEList<WMLMacro>(WMLMacro.class, this, WMLPackage.WML_ROOT__RMACROS);
    }
    return rmacros;
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
      case WMLPackage.WML_ROOT__RTAGS:
        return ((InternalEList<?>)getRtags()).basicRemove(otherEnd, msgs);
      case WMLPackage.WML_ROOT__RMACROS:
        return ((InternalEList<?>)getRmacros()).basicRemove(otherEnd, msgs);
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
      case WMLPackage.WML_ROOT__RTAGS:
        return getRtags();
      case WMLPackage.WML_ROOT__RMACROS:
        return getRmacros();
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
      case WMLPackage.WML_ROOT__RTAGS:
        getRtags().clear();
        getRtags().addAll((Collection<? extends WMLTag>)newValue);
        return;
      case WMLPackage.WML_ROOT__RMACROS:
        getRmacros().clear();
        getRmacros().addAll((Collection<? extends WMLMacro>)newValue);
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
      case WMLPackage.WML_ROOT__RTAGS:
        getRtags().clear();
        return;
      case WMLPackage.WML_ROOT__RMACROS:
        getRmacros().clear();
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
      case WMLPackage.WML_ROOT__RTAGS:
        return rtags != null && !rtags.isEmpty();
      case WMLPackage.WML_ROOT__RMACROS:
        return rmacros != null && !rmacros.isEmpty();
    }
    return super.eIsSet(featureID);
  }

} //WMLRootImpl
