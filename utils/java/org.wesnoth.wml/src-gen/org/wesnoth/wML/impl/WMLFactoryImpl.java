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
      case WMLPackage.ROOT: return createRoot();
      case WMLPackage.TEXT_DOMAIN: return createTextDomain();
      case WMLPackage.PREPROCESSOR: return createPreprocessor();
      case WMLPackage.MACRO: return createMacro();
      case WMLPackage.PATH_INCLUDE: return createPathInclude();
      case WMLPackage.ROOT_TYPE: return createRootType();
      case WMLPackage.ROOT_TAG: return createRootTag();
      case WMLPackage.SIMPLE_TAG: return createSimpleTag();
      case WMLPackage.ADDED_TAG: return createAddedTag();
      case WMLPackage.ATTRIBUTES: return createAttributes();
      default:
        throw new IllegalArgumentException("The class '" + eClass.getName() + "' is not a valid classifier");
    }
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public Root createRoot()
  {
    RootImpl root = new RootImpl();
    return root;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public TextDomain createTextDomain()
  {
    TextDomainImpl textDomain = new TextDomainImpl();
    return textDomain;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public Preprocessor createPreprocessor()
  {
    PreprocessorImpl preprocessor = new PreprocessorImpl();
    return preprocessor;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public Macro createMacro()
  {
    MacroImpl macro = new MacroImpl();
    return macro;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public PathInclude createPathInclude()
  {
    PathIncludeImpl pathInclude = new PathIncludeImpl();
    return pathInclude;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public RootType createRootType()
  {
    RootTypeImpl rootType = new RootTypeImpl();
    return rootType;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public RootTag createRootTag()
  {
    RootTagImpl rootTag = new RootTagImpl();
    return rootTag;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public SimpleTag createSimpleTag()
  {
    SimpleTagImpl simpleTag = new SimpleTagImpl();
    return simpleTag;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public AddedTag createAddedTag()
  {
    AddedTagImpl addedTag = new AddedTagImpl();
    return addedTag;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public Attributes createAttributes()
  {
    AttributesImpl attributes = new AttributesImpl();
    return attributes;
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
