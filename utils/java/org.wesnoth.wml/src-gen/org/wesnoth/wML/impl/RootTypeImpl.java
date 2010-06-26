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

import org.wesnoth.wML.Attributes;
import org.wesnoth.wML.Preprocessor;
import org.wesnoth.wML.RootTag;
import org.wesnoth.wML.RootType;
import org.wesnoth.wML.WMLPackage;

/**
 * <!-- begin-user-doc -->
 * An implementation of the model object '<em><b>Root Type</b></em>'.
 * <!-- end-user-doc -->
 * <p>
 * The following features are implemented:
 * <ul>
 *   <li>{@link org.wesnoth.wML.impl.RootTypeImpl#getStartTag <em>Start Tag</em>}</li>
 *   <li>{@link org.wesnoth.wML.impl.RootTypeImpl#getSubTypes <em>Sub Types</em>}</li>
 *   <li>{@link org.wesnoth.wML.impl.RootTypeImpl#getAt <em>At</em>}</li>
 *   <li>{@link org.wesnoth.wML.impl.RootTypeImpl#getOkpreproc <em>Okpreproc</em>}</li>
 *   <li>{@link org.wesnoth.wML.impl.RootTypeImpl#getEndTag <em>End Tag</em>}</li>
 * </ul>
 * </p>
 *
 * @generated
 */
public class RootTypeImpl extends MinimalEObjectImpl.Container implements RootType
{
  /**
   * The cached value of the '{@link #getStartTag() <em>Start Tag</em>}' containment reference.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getStartTag()
   * @generated
   * @ordered
   */
  protected RootTag startTag;

  /**
   * The cached value of the '{@link #getSubTypes() <em>Sub Types</em>}' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getSubTypes()
   * @generated
   * @ordered
   */
  protected EList<RootType> subTypes;

  /**
   * The cached value of the '{@link #getAt() <em>At</em>}' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getAt()
   * @generated
   * @ordered
   */
  protected EList<Attributes> at;

  /**
   * The cached value of the '{@link #getOkpreproc() <em>Okpreproc</em>}' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getOkpreproc()
   * @generated
   * @ordered
   */
  protected EList<Preprocessor> okpreproc;

  /**
   * The cached value of the '{@link #getEndTag() <em>End Tag</em>}' containment reference.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getEndTag()
   * @generated
   * @ordered
   */
  protected RootTag endTag;

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  protected RootTypeImpl()
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
    return WMLPackage.Literals.ROOT_TYPE;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public RootTag getStartTag()
  {
    return startTag;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public NotificationChain basicSetStartTag(RootTag newStartTag, NotificationChain msgs)
  {
    RootTag oldStartTag = startTag;
    startTag = newStartTag;
    if (eNotificationRequired())
    {
      ENotificationImpl notification = new ENotificationImpl(this, Notification.SET, WMLPackage.ROOT_TYPE__START_TAG, oldStartTag, newStartTag);
      if (msgs == null) msgs = notification; else msgs.add(notification);
    }
    return msgs;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public void setStartTag(RootTag newStartTag)
  {
    if (newStartTag != startTag)
    {
      NotificationChain msgs = null;
      if (startTag != null)
        msgs = ((InternalEObject)startTag).eInverseRemove(this, EOPPOSITE_FEATURE_BASE - WMLPackage.ROOT_TYPE__START_TAG, null, msgs);
      if (newStartTag != null)
        msgs = ((InternalEObject)newStartTag).eInverseAdd(this, EOPPOSITE_FEATURE_BASE - WMLPackage.ROOT_TYPE__START_TAG, null, msgs);
      msgs = basicSetStartTag(newStartTag, msgs);
      if (msgs != null) msgs.dispatch();
    }
    else if (eNotificationRequired())
      eNotify(new ENotificationImpl(this, Notification.SET, WMLPackage.ROOT_TYPE__START_TAG, newStartTag, newStartTag));
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EList<RootType> getSubTypes()
  {
    if (subTypes == null)
    {
      subTypes = new EObjectContainmentEList<RootType>(RootType.class, this, WMLPackage.ROOT_TYPE__SUB_TYPES);
    }
    return subTypes;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EList<Attributes> getAt()
  {
    if (at == null)
    {
      at = new EObjectContainmentEList<Attributes>(Attributes.class, this, WMLPackage.ROOT_TYPE__AT);
    }
    return at;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EList<Preprocessor> getOkpreproc()
  {
    if (okpreproc == null)
    {
      okpreproc = new EObjectContainmentEList<Preprocessor>(Preprocessor.class, this, WMLPackage.ROOT_TYPE__OKPREPROC);
    }
    return okpreproc;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public RootTag getEndTag()
  {
    return endTag;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public NotificationChain basicSetEndTag(RootTag newEndTag, NotificationChain msgs)
  {
    RootTag oldEndTag = endTag;
    endTag = newEndTag;
    if (eNotificationRequired())
    {
      ENotificationImpl notification = new ENotificationImpl(this, Notification.SET, WMLPackage.ROOT_TYPE__END_TAG, oldEndTag, newEndTag);
      if (msgs == null) msgs = notification; else msgs.add(notification);
    }
    return msgs;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public void setEndTag(RootTag newEndTag)
  {
    if (newEndTag != endTag)
    {
      NotificationChain msgs = null;
      if (endTag != null)
        msgs = ((InternalEObject)endTag).eInverseRemove(this, EOPPOSITE_FEATURE_BASE - WMLPackage.ROOT_TYPE__END_TAG, null, msgs);
      if (newEndTag != null)
        msgs = ((InternalEObject)newEndTag).eInverseAdd(this, EOPPOSITE_FEATURE_BASE - WMLPackage.ROOT_TYPE__END_TAG, null, msgs);
      msgs = basicSetEndTag(newEndTag, msgs);
      if (msgs != null) msgs.dispatch();
    }
    else if (eNotificationRequired())
      eNotify(new ENotificationImpl(this, Notification.SET, WMLPackage.ROOT_TYPE__END_TAG, newEndTag, newEndTag));
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
      case WMLPackage.ROOT_TYPE__START_TAG:
        return basicSetStartTag(null, msgs);
      case WMLPackage.ROOT_TYPE__SUB_TYPES:
        return ((InternalEList<?>)getSubTypes()).basicRemove(otherEnd, msgs);
      case WMLPackage.ROOT_TYPE__AT:
        return ((InternalEList<?>)getAt()).basicRemove(otherEnd, msgs);
      case WMLPackage.ROOT_TYPE__OKPREPROC:
        return ((InternalEList<?>)getOkpreproc()).basicRemove(otherEnd, msgs);
      case WMLPackage.ROOT_TYPE__END_TAG:
        return basicSetEndTag(null, msgs);
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
      case WMLPackage.ROOT_TYPE__START_TAG:
        return getStartTag();
      case WMLPackage.ROOT_TYPE__SUB_TYPES:
        return getSubTypes();
      case WMLPackage.ROOT_TYPE__AT:
        return getAt();
      case WMLPackage.ROOT_TYPE__OKPREPROC:
        return getOkpreproc();
      case WMLPackage.ROOT_TYPE__END_TAG:
        return getEndTag();
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
      case WMLPackage.ROOT_TYPE__START_TAG:
        setStartTag((RootTag)newValue);
        return;
      case WMLPackage.ROOT_TYPE__SUB_TYPES:
        getSubTypes().clear();
        getSubTypes().addAll((Collection<? extends RootType>)newValue);
        return;
      case WMLPackage.ROOT_TYPE__AT:
        getAt().clear();
        getAt().addAll((Collection<? extends Attributes>)newValue);
        return;
      case WMLPackage.ROOT_TYPE__OKPREPROC:
        getOkpreproc().clear();
        getOkpreproc().addAll((Collection<? extends Preprocessor>)newValue);
        return;
      case WMLPackage.ROOT_TYPE__END_TAG:
        setEndTag((RootTag)newValue);
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
      case WMLPackage.ROOT_TYPE__START_TAG:
        setStartTag((RootTag)null);
        return;
      case WMLPackage.ROOT_TYPE__SUB_TYPES:
        getSubTypes().clear();
        return;
      case WMLPackage.ROOT_TYPE__AT:
        getAt().clear();
        return;
      case WMLPackage.ROOT_TYPE__OKPREPROC:
        getOkpreproc().clear();
        return;
      case WMLPackage.ROOT_TYPE__END_TAG:
        setEndTag((RootTag)null);
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
      case WMLPackage.ROOT_TYPE__START_TAG:
        return startTag != null;
      case WMLPackage.ROOT_TYPE__SUB_TYPES:
        return subTypes != null && !subTypes.isEmpty();
      case WMLPackage.ROOT_TYPE__AT:
        return at != null && !at.isEmpty();
      case WMLPackage.ROOT_TYPE__OKPREPROC:
        return okpreproc != null && !okpreproc.isEmpty();
      case WMLPackage.ROOT_TYPE__END_TAG:
        return endTag != null;
    }
    return super.eIsSet(featureID);
  }

} //RootTypeImpl
