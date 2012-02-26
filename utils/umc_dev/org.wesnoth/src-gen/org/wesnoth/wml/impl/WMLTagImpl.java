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
import org.wesnoth.wml.WMLKey;
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
 *   <li>{@link org.wesnoth.wml.impl.WMLTagImpl#get_InhertedTagName <em>Inherted Tag Name</em>}</li>
 *   <li>{@link org.wesnoth.wml.impl.WMLTagImpl#is_NeedingExpansion <em>Needing Expansion</em>}</li>
 *   <li>{@link org.wesnoth.wml.impl.WMLTagImpl#get_Description <em>Description</em>}</li>
 * </ul>
 * </p>
 *
 * @generated
 */
public class WMLTagImpl extends WMLRootExpressionImpl implements WMLTag
{
  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  private static final long serialVersionUID = 1L;

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
   * The default value of the '{@link #get_InhertedTagName() <em>Inherted Tag Name</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #get_InhertedTagName()
   * @generated
   * @ordered
   */
  protected static final String _INHERTED_TAG_NAME_EDEFAULT = "";

  /**
   * The cached value of the '{@link #get_InhertedTagName() <em>Inherted Tag Name</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #get_InhertedTagName()
   * @generated
   * @ordered
   */
  protected String _InhertedTagName = _INHERTED_TAG_NAME_EDEFAULT;

  /**
   * The default value of the '{@link #is_NeedingExpansion() <em>Needing Expansion</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #is_NeedingExpansion()
   * @generated
   * @ordered
   */
  protected static final boolean _NEEDING_EXPANSION_EDEFAULT = false;

  /**
   * The cached value of the '{@link #is_NeedingExpansion() <em>Needing Expansion</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #is_NeedingExpansion()
   * @generated
   * @ordered
   */
  protected boolean _NeedingExpansion = _NEEDING_EXPANSION_EDEFAULT;

  /**
   * The default value of the '{@link #get_Description() <em>Description</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #get_Description()
   * @generated
   * @ordered
   */
  protected static final String _DESCRIPTION_EDEFAULT = "";

  /**
   * The cached value of the '{@link #get_Description() <em>Description</em>}' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see #get_Description()
   * @generated
   * @ordered
   */
  protected String _Description = _DESCRIPTION_EDEFAULT;

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
  public String get_InhertedTagName()
  {
    return _InhertedTagName;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public void set_InhertedTagName(String new_InhertedTagName)
  {
    String old_InhertedTagName = _InhertedTagName;
    _InhertedTagName = new_InhertedTagName;
    if (eNotificationRequired())
      eNotify(new ENotificationImpl(this, Notification.SET, WmlPackage.WML_TAG__INHERTED_TAG_NAME, old_InhertedTagName, _InhertedTagName));
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public boolean is_NeedingExpansion()
  {
    return _NeedingExpansion;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public void set_NeedingExpansion(boolean new_NeedingExpansion)
  {
    boolean old_NeedingExpansion = _NeedingExpansion;
    _NeedingExpansion = new_NeedingExpansion;
    if (eNotificationRequired())
      eNotify(new ENotificationImpl(this, Notification.SET, WmlPackage.WML_TAG__NEEDING_EXPANSION, old_NeedingExpansion, _NeedingExpansion));
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public String get_Description()
  {
    return _Description;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public void set_Description(String new_Description)
  {
    String old_Description = _Description;
    _Description = new_Description;
    if (eNotificationRequired())
      eNotify(new ENotificationImpl(this, Notification.SET, WmlPackage.WML_TAG__DESCRIPTION, old_Description, _Description));
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EList<WMLTag> getWMLTags()
  {
    EList<WMLTag> result = new org.eclipse.emf.common.util.BasicEList<WMLTag>();
            for ( WMLExpression expression : getExpressions( ) ) {
                if ( expression.isWMLTag( ) )
                    result.add( expression.asWMLTag( ) );
            }
    
            return result;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public EList<WMLKey> getWMLKeys()
  {
    EList<WMLKey> result = new org.eclipse.emf.common.util.BasicEList<WMLKey>();
            for ( WMLExpression expression : getExpressions( ) ) {
                if ( expression.isWMLKey( ) )
                    result.add( expression.asWMLKey( ) );
            }
    
            return result;
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
      case WmlPackage.WML_TAG__INHERTED_TAG_NAME:
        return get_InhertedTagName();
      case WmlPackage.WML_TAG__NEEDING_EXPANSION:
        return is_NeedingExpansion();
      case WmlPackage.WML_TAG__DESCRIPTION:
        return get_Description();
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
      case WmlPackage.WML_TAG__INHERTED_TAG_NAME:
        set_InhertedTagName((String)newValue);
        return;
      case WmlPackage.WML_TAG__NEEDING_EXPANSION:
        set_NeedingExpansion((Boolean)newValue);
        return;
      case WmlPackage.WML_TAG__DESCRIPTION:
        set_Description((String)newValue);
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
      case WmlPackage.WML_TAG__INHERTED_TAG_NAME:
        set_InhertedTagName(_INHERTED_TAG_NAME_EDEFAULT);
        return;
      case WmlPackage.WML_TAG__NEEDING_EXPANSION:
        set_NeedingExpansion(_NEEDING_EXPANSION_EDEFAULT);
        return;
      case WmlPackage.WML_TAG__DESCRIPTION:
        set_Description(_DESCRIPTION_EDEFAULT);
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
      case WmlPackage.WML_TAG__INHERTED_TAG_NAME:
        return _INHERTED_TAG_NAME_EDEFAULT == null ? _InhertedTagName != null : !_INHERTED_TAG_NAME_EDEFAULT.equals(_InhertedTagName);
      case WmlPackage.WML_TAG__NEEDING_EXPANSION:
        return _NeedingExpansion != _NEEDING_EXPANSION_EDEFAULT;
      case WmlPackage.WML_TAG__DESCRIPTION:
        return _DESCRIPTION_EDEFAULT == null ? _Description != null : !_DESCRIPTION_EDEFAULT.equals(_Description);
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
    result.append(", _InhertedTagName: ");
    result.append(_InhertedTagName);
    result.append(", _NeedingExpansion: ");
    result.append(_NeedingExpansion);
    result.append(", _Description: ");
    result.append(_Description);
    result.append(')');
    return result.toString();
  }

} //WMLTagImpl
