/**
 * <copyright>
 * </copyright>
 *

 */
package org.wesnoth.wml.impl;

import org.eclipse.emf.ecore.EClass;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.emf.ecore.EPackage;

import org.eclipse.emf.ecore.impl.EFactoryImpl;

import org.eclipse.emf.ecore.plugin.EcorePlugin;

import org.wesnoth.wml.*;

/**
 * <!-- begin-user-doc -->
 * An implementation of the model <b>Factory</b>.
 * <!-- end-user-doc -->
 * @generated
 */
public class WmlFactoryImpl extends EFactoryImpl implements WmlFactory
{
  /**
   * Creates the default factory implementation.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public static WmlFactory init()
  {
    try
    {
      WmlFactory theWmlFactory = (WmlFactory)EPackage.Registry.INSTANCE.getEFactory("http://www.wesnoth.org/WML"); 
      if (theWmlFactory != null)
      {
        return theWmlFactory;
      }
    }
    catch (Exception exception)
    {
      EcorePlugin.INSTANCE.log(exception);
    }
    return new WmlFactoryImpl();
  }

  /**
   * Creates an instance of the factory.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public WmlFactoryImpl()
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
      case WmlPackage.WML_ROOT: return createWMLRoot();
      case WmlPackage.WML_GRAMMAR_ELEMENT: return createWMLGrammarElement();
      case WmlPackage.WML_TAG: return createWMLTag();
      case WmlPackage.WML_KEY: return createWMLKey();
      case WmlPackage.WML_KEY_VALUE: return createWMLKeyValue();
      case WmlPackage.WML_MACRO_CALL: return createWMLMacroCall();
      case WmlPackage.WML_MACRO_CALL_PARAMETER: return createWMLMacroCallParameter();
      case WmlPackage.WML_ARRAY_CALL: return createWMLArrayCall();
      case WmlPackage.WML_MACRO_DEFINE: return createWMLMacroDefine();
      case WmlPackage.WML_PREPROC_IF: return createWMLPreprocIF();
      case WmlPackage.WML_ROOT_EXPRESSION: return createWMLRootExpression();
      case WmlPackage.WML_EXPRESSION: return createWMLExpression();
      case WmlPackage.WML_VALUED_EXPRESSION: return createWMLValuedExpression();
      case WmlPackage.WML_TEXTDOMAIN: return createWMLTextdomain();
      case WmlPackage.WML_LUA_CODE: return createWMLLuaCode();
      case WmlPackage.WML_MACRO_PARAMETER: return createWMLMacroParameter();
      case WmlPackage.WML_VALUE: return createWMLValue();
      case WmlPackage.MACRO_TOKENS: return createMacroTokens();
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
  public WMLGrammarElement createWMLGrammarElement()
  {
    WMLGrammarElementImpl wmlGrammarElement = new WMLGrammarElementImpl();
    return wmlGrammarElement;
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
  public WMLKeyValue createWMLKeyValue()
  {
    WMLKeyValueImpl wmlKeyValue = new WMLKeyValueImpl();
    return wmlKeyValue;
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
  public WMLMacroCallParameter createWMLMacroCallParameter()
  {
    WMLMacroCallParameterImpl wmlMacroCallParameter = new WMLMacroCallParameterImpl();
    return wmlMacroCallParameter;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public WMLArrayCall createWMLArrayCall()
  {
    WMLArrayCallImpl wmlArrayCall = new WMLArrayCallImpl();
    return wmlArrayCall;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public WMLMacroDefine createWMLMacroDefine()
  {
    WMLMacroDefineImpl wmlMacroDefine = new WMLMacroDefineImpl();
    return wmlMacroDefine;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public WMLPreprocIF createWMLPreprocIF()
  {
    WMLPreprocIFImpl wmlPreprocIF = new WMLPreprocIFImpl();
    return wmlPreprocIF;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public WMLRootExpression createWMLRootExpression()
  {
    WMLRootExpressionImpl wmlRootExpression = new WMLRootExpressionImpl();
    return wmlRootExpression;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public WMLExpression createWMLExpression()
  {
    WMLExpressionImpl wmlExpression = new WMLExpressionImpl();
    return wmlExpression;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public WMLValuedExpression createWMLValuedExpression()
  {
    WMLValuedExpressionImpl wmlValuedExpression = new WMLValuedExpressionImpl();
    return wmlValuedExpression;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public WMLTextdomain createWMLTextdomain()
  {
    WMLTextdomainImpl wmlTextdomain = new WMLTextdomainImpl();
    return wmlTextdomain;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public WMLLuaCode createWMLLuaCode()
  {
    WMLLuaCodeImpl wmlLuaCode = new WMLLuaCodeImpl();
    return wmlLuaCode;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public WMLMacroParameter createWMLMacroParameter()
  {
    WMLMacroParameterImpl wmlMacroParameter = new WMLMacroParameterImpl();
    return wmlMacroParameter;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public WMLValue createWMLValue()
  {
    WMLValueImpl wmlValue = new WMLValueImpl();
    return wmlValue;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public MacroTokens createMacroTokens()
  {
    MacroTokensImpl macroTokens = new MacroTokensImpl();
    return macroTokens;
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  public WmlPackage getWmlPackage()
  {
    return (WmlPackage)getEPackage();
  }

  /**
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @deprecated
   * @generated
   */
  @Deprecated
  public static WmlPackage getPackage()
  {
    return WmlPackage.eINSTANCE;
  }

} //WmlFactoryImpl
