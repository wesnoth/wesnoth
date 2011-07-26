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

import org.eclipse.emf.ecore.util.EObjectContainmentEList;
import org.eclipse.emf.ecore.util.InternalEList;

import org.wesnoth.wml.WMLExpression;
import org.wesnoth.wml.WMLTag;
import org.wesnoth.wml.WmlPackage;

/**
 * <!-- begin-user-doc -->
 * An implementation of the model object '<em><b>WML Tag</b></em>'.
 * <!-- end-user-doc -->
 * <p>
 * The following features are implemented:
 * <ul>
 *   <li>{@link org.wesnoth.wml.impl.WMLTagImpl#getPlus <em>Plus</em>}</li>
 *   <li>{@link org.wesnoth.wml.impl.WMLTagImpl#getExpressions <em>Expressions</em>}</li>
 *   <li>{@link org.wesnoth.wml.impl.WMLTagImpl#getEndName <em>End Name</em>}</li>
 *   <li>{@link org.wesnoth.wml.impl.WMLTagImpl#get_extendedTagName <em>extended Tag Name</em>}</li>
 *   <li>{@link org.wesnoth.wml.impl.WMLTagImpl#get_cardinality <em>cardinality</em>}</li>
 *   <li>{@link org.wesnoth.wml.impl.WMLTagImpl#is_needsExpanding <em>needs Expanding</em>}</li>
 * </ul>
 * </p>
 *
 * @generated
 */
public class WMLTagImpl extends WMLRootExpressionImpl implements WMLTag
{
  /**
   * The default value of the '{@link #getPlus() <em>Plus</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getPlus()
   * @generated
   * @ordered
   */
  protected static final String PLUS_EDEFAULT = "";

  /**
   * The cached value of the '{@link #getPlus() <em>Plus</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getPlus()
   * @generated
   * @ordered
   */
  protected String plus = PLUS_EDEFAULT;

  /**
   * The cached value of the '{@link #getExpressions() <em>Expressions</em>}' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getExpressions()
   * @generated
   * @ordered
   */
  protected EList<WMLExpression> expressions;

  /**
   * The default value of the '{@link #getEndName() <em>End Name</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getEndName()
   * @generated
   * @ordered
   */
  protected static final String END_NAME_EDEFAULT = "";

  /**
   * The cached value of the '{@link #getEndName() <em>End Name</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #getEndName()
   * @generated
   * @ordered
   */
  protected String endName = END_NAME_EDEFAULT;

  /**
   * The default value of the '{@link #get_extendedTagName() <em>extended Tag Name</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #get_extendedTagName()
   * @generated
   * @ordered
   */
  protected static final String _EXTENDED_TAG_NAME_EDEFAULT = "";

  /**
   * The cached value of the '{@link #get_extendedTagName() <em>extended Tag Name</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #get_extendedTagName()
   * @generated
   * @ordered
   */
  protected String _extendedTagName = _EXTENDED_TAG_NAME_EDEFAULT;

  /**
   * The default value of the '{@link #get_cardinality() <em>cardinality</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #get_cardinality()
   * @generated
   * @ordered
   */
  protected static final char _CARDINALITY_EDEFAULT = ' ';

  /**
   * The cached value of the '{@link #get_cardinality() <em>cardinality</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #get_cardinality()
   * @generated
   * @ordered
   */
  protected char _cardinality = _CARDINALITY_EDEFAULT;

  /**
   * The default value of the '{@link #is_needsExpanding() <em>needs Expanding</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #is_needsExpanding()
   * @generated
   * @ordered
   */
  protected static final boolean _NEEDS_EXPANDING_EDEFAULT = false;

  /**
   * The cached value of the '{@link #is_needsExpanding() <em>needs Expanding</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #is_needsExpanding()
   * @generated
   * @ordered
   */
  protected boolean _needsExpanding = _NEEDS_EXPANDING_EDEFAULT;

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
    return WmlPackage.Literals.WML_TAG;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public String getPlus()
  {
    return plus;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public void setPlus(String newPlus)
  {
    String oldPlus = plus;
    plus = newPlus;
    if (eNotificationRequired())
      eNotify(new ENotificationImpl(this, Notification.SET, WmlPackage.WML_TAG__PLUS, oldPlus, plus));
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EList<WMLExpression> getExpressions()
  {
    if (expressions == null)
    {
      expressions = new EObjectContainmentEList<WMLExpression>(WMLExpression.class, this, WmlPackage.WML_TAG__EXPRESSIONS);
    }
    return expressions;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public String getEndName()
  {
    return endName;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public void setEndName(String newEndName)
  {
    String oldEndName = endName;
    endName = newEndName;
    if (eNotificationRequired())
      eNotify(new ENotificationImpl(this, Notification.SET, WmlPackage.WML_TAG__END_NAME, oldEndName, endName));
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public String get_extendedTagName()
  {
    return _extendedTagName;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public void set_extendedTagName(String new_extendedTagName)
  {
    String old_extendedTagName = _extendedTagName;
    _extendedTagName = new_extendedTagName;
    if (eNotificationRequired())
      eNotify(new ENotificationImpl(this, Notification.SET, WmlPackage.WML_TAG__EXTENDED_TAG_NAME, old_extendedTagName, _extendedTagName));
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public char get_cardinality()
  {
    return _cardinality;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public void set_cardinality(char new_cardinality)
  {
    char old_cardinality = _cardinality;
    _cardinality = new_cardinality;
    if (eNotificationRequired())
      eNotify(new ENotificationImpl(this, Notification.SET, WmlPackage.WML_TAG__CARDINALITY, old_cardinality, _cardinality));
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public boolean is_needsExpanding()
  {
    return _needsExpanding;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public void set_needsExpanding(boolean new_needsExpanding)
  {
    boolean old_needsExpanding = _needsExpanding;
    _needsExpanding = new_needsExpanding;
    if (eNotificationRequired())
      eNotify(new ENotificationImpl(this, Notification.SET, WmlPackage.WML_TAG__NEEDS_EXPANDING, old_needsExpanding, _needsExpanding));
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
      case WmlPackage.WML_TAG__EXPRESSIONS:
        return ((InternalEList<?>)getExpressions()).basicRemove(otherEnd, msgs);
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
      case WmlPackage.WML_TAG__PLUS:
        return getPlus();
      case WmlPackage.WML_TAG__EXPRESSIONS:
        return getExpressions();
      case WmlPackage.WML_TAG__END_NAME:
        return getEndName();
      case WmlPackage.WML_TAG__EXTENDED_TAG_NAME:
        return get_extendedTagName();
      case WmlPackage.WML_TAG__CARDINALITY:
        return get_cardinality();
      case WmlPackage.WML_TAG__NEEDS_EXPANDING:
        return is_needsExpanding();
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
      case WmlPackage.WML_TAG__PLUS:
        setPlus((String)newValue);
        return;
      case WmlPackage.WML_TAG__EXPRESSIONS:
        getExpressions().clear();
        getExpressions().addAll((Collection<? extends WMLExpression>)newValue);
        return;
      case WmlPackage.WML_TAG__END_NAME:
        setEndName((String)newValue);
        return;
      case WmlPackage.WML_TAG__EXTENDED_TAG_NAME:
        set_extendedTagName((String)newValue);
        return;
      case WmlPackage.WML_TAG__CARDINALITY:
        set_cardinality((Character)newValue);
        return;
      case WmlPackage.WML_TAG__NEEDS_EXPANDING:
        set_needsExpanding((Boolean)newValue);
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
      case WmlPackage.WML_TAG__PLUS:
        setPlus(PLUS_EDEFAULT);
        return;
      case WmlPackage.WML_TAG__EXPRESSIONS:
        getExpressions().clear();
        return;
      case WmlPackage.WML_TAG__END_NAME:
        setEndName(END_NAME_EDEFAULT);
        return;
      case WmlPackage.WML_TAG__EXTENDED_TAG_NAME:
        set_extendedTagName(_EXTENDED_TAG_NAME_EDEFAULT);
        return;
      case WmlPackage.WML_TAG__CARDINALITY:
        set_cardinality(_CARDINALITY_EDEFAULT);
        return;
      case WmlPackage.WML_TAG__NEEDS_EXPANDING:
        set_needsExpanding(_NEEDS_EXPANDING_EDEFAULT);
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
      case WmlPackage.WML_TAG__PLUS:
        return PLUS_EDEFAULT == null ? plus != null : !PLUS_EDEFAULT.equals(plus);
      case WmlPackage.WML_TAG__EXPRESSIONS:
        return expressions != null && !expressions.isEmpty();
      case WmlPackage.WML_TAG__END_NAME:
        return END_NAME_EDEFAULT == null ? endName != null : !END_NAME_EDEFAULT.equals(endName);
      case WmlPackage.WML_TAG__EXTENDED_TAG_NAME:
        return _EXTENDED_TAG_NAME_EDEFAULT == null ? _extendedTagName != null : !_EXTENDED_TAG_NAME_EDEFAULT.equals(_extendedTagName);
      case WmlPackage.WML_TAG__CARDINALITY:
        return _cardinality != _CARDINALITY_EDEFAULT;
      case WmlPackage.WML_TAG__NEEDS_EXPANDING:
        return _needsExpanding != _NEEDS_EXPANDING_EDEFAULT;
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
    result.append(" (plus: ");
    result.append(plus);
    result.append(", endName: ");
    result.append(endName);
    result.append(", _extendedTagName: ");
    result.append(_extendedTagName);
    result.append(", _cardinality: ");
    result.append(_cardinality);
    result.append(", _needsExpanding: ");
    result.append(_needsExpanding);
    result.append(')');
    return result.toString();
  }

} //WMLTagImpl
