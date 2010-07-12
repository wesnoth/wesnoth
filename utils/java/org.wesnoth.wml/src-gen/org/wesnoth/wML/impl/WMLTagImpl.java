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
import org.eclipse.emf.ecore.InternalEObject;

import org.eclipse.emf.ecore.impl.ENotificationImpl;
import org.eclipse.emf.ecore.impl.MinimalEObjectImpl;

import org.eclipse.emf.ecore.util.EObjectContainmentEList;
import org.eclipse.emf.ecore.util.InternalEList;

import org.wesnoth.wML.WMLEndTag;
import org.wesnoth.wML.WMLKey;
import org.wesnoth.wML.WMLMacro;
import org.wesnoth.wML.WMLPackage;
import org.wesnoth.wML.WMLTag;

/**
 * <!-- begin-user-doc -->
 * An implementation of the model object '<em><b>Tag</b></em>'.
 * <!-- end-user-doc -->
 * <p>
 * The following features are implemented:
 * <ul>
 *   <li>{@link org.wesnoth.wML.impl.WMLTagImpl#getName <em>Name</em>}</li>
 *   <li>{@link org.wesnoth.wML.impl.WMLTagImpl#getTtags <em>Ttags</em>}</li>
 *   <li>{@link org.wesnoth.wML.impl.WMLTagImpl#getTkeys <em>Tkeys</em>}</li>
 *   <li>{@link org.wesnoth.wML.impl.WMLTagImpl#getTmacros <em>Tmacros</em>}</li>
 *   <li>{@link org.wesnoth.wML.impl.WMLTagImpl#getEnd <em>End</em>}</li>
 * </ul>
 * </p>
 *
 * @generated
 */
public class WMLTagImpl extends MinimalEObjectImpl.Container implements WMLTag
{
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
   * The cached value of the '{@link #getTtags() <em>Ttags</em>}' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getTtags()
   * @generated
   * @ordered
   */
  protected EList<WMLTag> ttags;

  /**
   * The cached value of the '{@link #getTkeys() <em>Tkeys</em>}' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getTkeys()
   * @generated
   * @ordered
   */
  protected EList<WMLKey> tkeys;

  /**
   * The cached value of the '{@link #getTmacros() <em>Tmacros</em>}' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getTmacros()
   * @generated
   * @ordered
   */
  protected EList<WMLMacro> tmacros;

  /**
   * The cached value of the '{@link #getEnd() <em>End</em>}' containment reference.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getEnd()
   * @generated
   * @ordered
   */
  protected WMLEndTag end;

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  protected WMLTagImpl()
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
    return WMLPackage.Literals.WML_TAG;
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
      eNotify(new ENotificationImpl(this, Notification.SET, WMLPackage.WML_TAG__NAME, oldName, name));
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EList<WMLTag> getTtags()
  {
    if (ttags == null)
    {
      ttags = new EObjectContainmentEList<WMLTag>(WMLTag.class, this, WMLPackage.WML_TAG__TTAGS);
    }
    return ttags;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EList<WMLKey> getTkeys()
  {
    if (tkeys == null)
    {
      tkeys = new EObjectContainmentEList<WMLKey>(WMLKey.class, this, WMLPackage.WML_TAG__TKEYS);
    }
    return tkeys;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EList<WMLMacro> getTmacros()
  {
    if (tmacros == null)
    {
      tmacros = new EObjectContainmentEList<WMLMacro>(WMLMacro.class, this, WMLPackage.WML_TAG__TMACROS);
    }
    return tmacros;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public WMLEndTag getEnd()
  {
    return end;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public NotificationChain basicSetEnd(WMLEndTag newEnd, NotificationChain msgs)
  {
    WMLEndTag oldEnd = end;
    end = newEnd;
    if (eNotificationRequired())
    {
      ENotificationImpl notification = new ENotificationImpl(this, Notification.SET, WMLPackage.WML_TAG__END, oldEnd, newEnd);
      if (msgs == null) msgs = notification; else msgs.add(notification);
    }
    return msgs;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public void setEnd(WMLEndTag newEnd)
  {
    if (newEnd != end)
    {
      NotificationChain msgs = null;
      if (end != null)
        msgs = ((InternalEObject)end).eInverseRemove(this, EOPPOSITE_FEATURE_BASE - WMLPackage.WML_TAG__END, null, msgs);
      if (newEnd != null)
        msgs = ((InternalEObject)newEnd).eInverseAdd(this, EOPPOSITE_FEATURE_BASE - WMLPackage.WML_TAG__END, null, msgs);
      msgs = basicSetEnd(newEnd, msgs);
      if (msgs != null) msgs.dispatch();
    }
    else if (eNotificationRequired())
      eNotify(new ENotificationImpl(this, Notification.SET, WMLPackage.WML_TAG__END, newEnd, newEnd));
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
      case WMLPackage.WML_TAG__TTAGS:
        return ((InternalEList<?>)getTtags()).basicRemove(otherEnd, msgs);
      case WMLPackage.WML_TAG__TKEYS:
        return ((InternalEList<?>)getTkeys()).basicRemove(otherEnd, msgs);
      case WMLPackage.WML_TAG__TMACROS:
        return ((InternalEList<?>)getTmacros()).basicRemove(otherEnd, msgs);
      case WMLPackage.WML_TAG__END:
        return basicSetEnd(null, msgs);
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
      case WMLPackage.WML_TAG__NAME:
        return getName();
      case WMLPackage.WML_TAG__TTAGS:
        return getTtags();
      case WMLPackage.WML_TAG__TKEYS:
        return getTkeys();
      case WMLPackage.WML_TAG__TMACROS:
        return getTmacros();
      case WMLPackage.WML_TAG__END:
        return getEnd();
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
      case WMLPackage.WML_TAG__NAME:
        setName((String)newValue);
        return;
      case WMLPackage.WML_TAG__TTAGS:
        getTtags().clear();
        getTtags().addAll((Collection<? extends WMLTag>)newValue);
        return;
      case WMLPackage.WML_TAG__TKEYS:
        getTkeys().clear();
        getTkeys().addAll((Collection<? extends WMLKey>)newValue);
        return;
      case WMLPackage.WML_TAG__TMACROS:
        getTmacros().clear();
        getTmacros().addAll((Collection<? extends WMLMacro>)newValue);
        return;
      case WMLPackage.WML_TAG__END:
        setEnd((WMLEndTag)newValue);
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
      case WMLPackage.WML_TAG__NAME:
        setName(NAME_EDEFAULT);
        return;
      case WMLPackage.WML_TAG__TTAGS:
        getTtags().clear();
        return;
      case WMLPackage.WML_TAG__TKEYS:
        getTkeys().clear();
        return;
      case WMLPackage.WML_TAG__TMACROS:
        getTmacros().clear();
        return;
      case WMLPackage.WML_TAG__END:
        setEnd((WMLEndTag)null);
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
      case WMLPackage.WML_TAG__NAME:
        return NAME_EDEFAULT == null ? name != null : !NAME_EDEFAULT.equals(name);
      case WMLPackage.WML_TAG__TTAGS:
        return ttags != null && !ttags.isEmpty();
      case WMLPackage.WML_TAG__TKEYS:
        return tkeys != null && !tkeys.isEmpty();
      case WMLPackage.WML_TAG__TMACROS:
        return tmacros != null && !tmacros.isEmpty();
      case WMLPackage.WML_TAG__END:
        return end != null;
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
    result.append(')');
    return result.toString();
  }

} //WMLTagImpl
