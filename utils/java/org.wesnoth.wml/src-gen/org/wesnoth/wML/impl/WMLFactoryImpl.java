/**
 * <copyright>
 * </copyright>
 *

 */
package org.wesnoth.wML.impl;

import org.eclipse.emf.ecore.EClass;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.emf.ecore.EPackage;

import org.eclipse.emf.ecore.impl.EFactoryImpl;

import org.eclipse.emf.ecore.plugin.EcorePlugin;

import org.wesnoth.wML.*;

/**
 * <!-- begin-user-doc -->
 * An implementation of the model <b>Factory</b>.
 * <!-- end-user-doc -->
 * @generated
 */
public class WMLFactoryImpl extends EFactoryImpl implements WMLFactory
{
  /**
   * Creates the default factory implementation.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public static WMLFactory init()
  {
    try
    {
      WMLFactory theWMLFactory = (WMLFactory)EPackage.Registry.INSTANCE.getEFactory("http://www.wesnoth.org/WML"); 
      if (theWMLFactory != null)
      {
        return theWMLFactory;
      }
    }
    catch (Exception exception)
    {
      EcorePlugin.INSTANCE.log(exception);
    }
    return new WMLFactoryImpl();
  }

  /**
   * Creates an instance of the factory.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public WMLFactoryImpl()
  {
    super();
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  @Override
  public EObject create(EClass eClass)
  {
    switch (eClass.getClassifierID())
    {
      case WMLPackage.WML_ROOT: return createWMLRoot();
      case WMLPackage.WML_TAG: return createWMLTag();
      case WMLPackage.WML_KEY: return createWMLKey();
      case WMLPackage.WML_ABSTRACT_MACRO_CALL: return createWMLAbstractMacroCall();
      case WMLPackage.WML_MACRO_INCLUDE: return createWMLMacroInclude();
      case WMLPackage.WML_MACRO_CALL: return createWMLMacroCall();
      case WMLPackage.WML_ABSTRACT_KEY_VALUE: return createWMLAbstractKeyValue();
      default:
        throw new IllegalArgumentException("The class '" + eClass.getName() + "' is not a valid classifier");
    }
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public WMLRoot createWMLRoot()
  {
    WMLRootImpl wmlRoot = new WMLRootImpl();
    return wmlRoot;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public WMLTag createWMLTag()
  {
    WMLTagImpl wmlTag = new WMLTagImpl();
    return wmlTag;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public WMLKey createWMLKey()
  {
    WMLKeyImpl wmlKey = new WMLKeyImpl();
    return wmlKey;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public WMLAbstractMacroCall createWMLAbstractMacroCall()
  {
    WMLAbstractMacroCallImpl wmlAbstractMacroCall = new WMLAbstractMacroCallImpl();
    return wmlAbstractMacroCall;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public WMLMacroInclude createWMLMacroInclude()
  {
    WMLMacroIncludeImpl wmlMacroInclude = new WMLMacroIncludeImpl();
    return wmlMacroInclude;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public WMLMacroCall createWMLMacroCall()
  {
    WMLMacroCallImpl wmlMacroCall = new WMLMacroCallImpl();
    return wmlMacroCall;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public WMLAbstractKeyValue createWMLAbstractKeyValue()
  {
    WMLAbstractKeyValueImpl wmlAbstractKeyValue = new WMLAbstractKeyValueImpl();
    return wmlAbstractKeyValue;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public WMLPackage getWMLPackage()
  {
    return (WMLPackage)getEPackage();
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @deprecated
   * @generated
   */
  @Deprecated
  public static WMLPackage getPackage()
  {
    return WMLPackage.eINSTANCE;
  }

} //WMLFactoryImpl
