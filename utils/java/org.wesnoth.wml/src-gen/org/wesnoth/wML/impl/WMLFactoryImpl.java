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
      case WMLPackage.WML_MACRO: return createWMLMacro();
      case WMLPackage.WML_TAG: return createWMLTag();
      case WMLPackage.WML_START_TAG: return createWMLStartTag();
      case WMLPackage.WML_END_TAG: return createWMLEndTag();
      case WMLPackage.WML_KEY: return createWMLKey();
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
  public WMLMacro createWMLMacro()
  {
    WMLMacroImpl wmlMacro = new WMLMacroImpl();
    return wmlMacro;
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
  public WMLStartTag createWMLStartTag()
  {
    WMLStartTagImpl wmlStartTag = new WMLStartTagImpl();
    return wmlStartTag;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public WMLEndTag createWMLEndTag()
  {
    WMLEndTagImpl wmlEndTag = new WMLEndTagImpl();
    return wmlEndTag;
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
