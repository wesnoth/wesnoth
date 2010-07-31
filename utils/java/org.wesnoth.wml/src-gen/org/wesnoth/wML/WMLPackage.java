/**
 * <copyright>
 * </copyright>
 *

 */
package org.wesnoth.wML;

import org.eclipse.emf.ecore.EAttribute;
import org.eclipse.emf.ecore.EClass;
import org.eclipse.emf.ecore.EPackage;
import org.eclipse.emf.ecore.EReference;

/**
 * <!-- begin-user-doc -->
 * The <b>Package</b> for the model.
 * It contains accessors for the meta objects to represent
 * <ul>
 *   <li>each class,</li>
 *   <li>each feature of each class,</li>
 *   <li>each enum,</li>
 *   <li>and each data type</li>
 * </ul>
 * <!-- end-user-doc -->
 * @see org.wesnoth.wML.WMLFactory
 * @model kind="package"
 * @generated
 */
public interface WMLPackage extends EPackage
{
  /**
   * The package name.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  String eNAME = "wML";

  /**
   * The package namespace URI.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  String eNS_URI = "http://www.wesnoth.org/WML";

  /**
   * The package namespace name.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  String eNS_PREFIX = "wML";

  /**
   * The singleton instance of the package.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  WMLPackage eINSTANCE = org.wesnoth.wML.impl.WMLPackageImpl.init();

  /**
   * The meta object id for the '{@link org.wesnoth.wML.impl.WMLRootImpl <em>Root</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wML.impl.WMLRootImpl
   * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLRoot()
   * @generated
   */
  int WML_ROOT = 0;

  /**
   * The feature id for the '<em><b>Tags</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_ROOT__TAGS = 0;

  /**
   * The feature id for the '<em><b>Macro Calls</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_ROOT__MACRO_CALLS = 1;

  /**
   * The feature id for the '<em><b>Macro Defines</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_ROOT__MACRO_DEFINES = 2;

  /**
   * The feature id for the '<em><b>Textdomains</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_ROOT__TEXTDOMAINS = 3;

  /**
   * The number of structural features of the '<em>Root</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_ROOT_FEATURE_COUNT = 4;

  /**
   * The meta object id for the '{@link org.wesnoth.wML.impl.WMLTagImpl <em>Tag</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wML.impl.WMLTagImpl
   * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLTag()
   * @generated
   */
  int WML_TAG = 1;

  /**
   * The feature id for the '<em><b>Plus</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG__PLUS = 0;

  /**
   * The feature id for the '<em><b>Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG__NAME = 1;

  /**
   * The feature id for the '<em><b>Tags</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG__TAGS = 2;

  /**
   * The feature id for the '<em><b>Keys</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG__KEYS = 3;

  /**
   * The feature id for the '<em><b>Macro Calls</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG__MACRO_CALLS = 4;

  /**
   * The feature id for the '<em><b>Macro Defines</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG__MACRO_DEFINES = 5;

  /**
   * The feature id for the '<em><b>Textdomains</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG__TEXTDOMAINS = 6;

  /**
   * The feature id for the '<em><b>End Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG__END_NAME = 7;

  /**
   * The number of structural features of the '<em>Tag</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG_FEATURE_COUNT = 8;

  /**
   * The meta object id for the '{@link org.wesnoth.wML.impl.WMLKeyImpl <em>Key</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wML.impl.WMLKeyImpl
   * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLKey()
   * @generated
   */
  int WML_KEY = 2;

  /**
   * The feature id for the '<em><b>Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_KEY__NAME = 0;

  /**
   * The feature id for the '<em><b>Value</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_KEY__VALUE = 1;

  /**
   * The number of structural features of the '<em>Key</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_KEY_FEATURE_COUNT = 2;

  /**
   * The meta object id for the '{@link org.wesnoth.wML.impl.WMLKeyValueImpl <em>Key Value</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wML.impl.WMLKeyValueImpl
   * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLKeyValue()
   * @generated
   */
  int WML_KEY_VALUE = 3;

  /**
   * The number of structural features of the '<em>Key Value</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_KEY_VALUE_FEATURE_COUNT = 0;

  /**
   * The meta object id for the '{@link org.wesnoth.wML.impl.WMLMacroCallImpl <em>Macro Call</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wML.impl.WMLMacroCallImpl
   * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLMacroCall()
   * @generated
   */
  int WML_MACRO_CALL = 4;

  /**
   * The feature id for the '<em><b>Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_CALL__NAME = WML_KEY_VALUE_FEATURE_COUNT + 0;

  /**
   * The number of structural features of the '<em>Macro Call</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_CALL_FEATURE_COUNT = WML_KEY_VALUE_FEATURE_COUNT + 1;

  /**
   * The meta object id for the '{@link org.wesnoth.wML.impl.WMLLuaCodeImpl <em>Lua Code</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wML.impl.WMLLuaCodeImpl
   * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLLuaCode()
   * @generated
   */
  int WML_LUA_CODE = 5;

  /**
   * The feature id for the '<em><b>Value</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_LUA_CODE__VALUE = WML_KEY_VALUE_FEATURE_COUNT + 0;

  /**
   * The number of structural features of the '<em>Lua Code</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_LUA_CODE_FEATURE_COUNT = WML_KEY_VALUE_FEATURE_COUNT + 1;

  /**
   * The meta object id for the '{@link org.wesnoth.wML.impl.WMLArrayCallImpl <em>Array Call</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wML.impl.WMLArrayCallImpl
   * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLArrayCall()
   * @generated
   */
  int WML_ARRAY_CALL = 6;

  /**
   * The feature id for the '<em><b>Value</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_ARRAY_CALL__VALUE = WML_KEY_VALUE_FEATURE_COUNT + 0;

  /**
   * The number of structural features of the '<em>Array Call</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_ARRAY_CALL_FEATURE_COUNT = WML_KEY_VALUE_FEATURE_COUNT + 1;

  /**
   * The meta object id for the '{@link org.wesnoth.wML.impl.WMLMacroDefineImpl <em>Macro Define</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wML.impl.WMLMacroDefineImpl
   * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLMacroDefine()
   * @generated
   */
  int WML_MACRO_DEFINE = 7;

  /**
   * The feature id for the '<em><b>Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_DEFINE__NAME = 0;

  /**
   * The number of structural features of the '<em>Macro Define</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_DEFINE_FEATURE_COUNT = 1;

  /**
   * The meta object id for the '{@link org.wesnoth.wML.impl.WMLTextdomainImpl <em>Textdomain</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wML.impl.WMLTextdomainImpl
   * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLTextdomain()
   * @generated
   */
  int WML_TEXTDOMAIN = 8;

  /**
   * The feature id for the '<em><b>Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TEXTDOMAIN__NAME = 0;

  /**
   * The number of structural features of the '<em>Textdomain</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TEXTDOMAIN_FEATURE_COUNT = 1;

  /**
   * The meta object id for the '{@link org.wesnoth.wML.impl.WMLValueImpl <em>Value</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wML.impl.WMLValueImpl
   * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLValue()
   * @generated
   */
  int WML_VALUE = 9;

  /**
   * The feature id for the '<em><b>Value</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_VALUE__VALUE = WML_KEY_VALUE_FEATURE_COUNT + 0;

  /**
   * The number of structural features of the '<em>Value</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_VALUE_FEATURE_COUNT = WML_KEY_VALUE_FEATURE_COUNT + 1;


  /**
   * Returns the meta object for class '{@link org.wesnoth.wML.WMLRoot <em>Root</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>Root</em>'.
   * @see org.wesnoth.wML.WMLRoot
   * @generated
   */
  EClass getWMLRoot();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.WMLRoot#getTags <em>Tags</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Tags</em>'.
   * @see org.wesnoth.wML.WMLRoot#getTags()
   * @see #getWMLRoot()
   * @generated
   */
  EReference getWMLRoot_Tags();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.WMLRoot#getMacroCalls <em>Macro Calls</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Macro Calls</em>'.
   * @see org.wesnoth.wML.WMLRoot#getMacroCalls()
   * @see #getWMLRoot()
   * @generated
   */
  EReference getWMLRoot_MacroCalls();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.WMLRoot#getMacroDefines <em>Macro Defines</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Macro Defines</em>'.
   * @see org.wesnoth.wML.WMLRoot#getMacroDefines()
   * @see #getWMLRoot()
   * @generated
   */
  EReference getWMLRoot_MacroDefines();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.WMLRoot#getTextdomains <em>Textdomains</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Textdomains</em>'.
   * @see org.wesnoth.wML.WMLRoot#getTextdomains()
   * @see #getWMLRoot()
   * @generated
   */
  EReference getWMLRoot_Textdomains();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wML.WMLTag <em>Tag</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>Tag</em>'.
   * @see org.wesnoth.wML.WMLTag
   * @generated
   */
  EClass getWMLTag();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wML.WMLTag#isPlus <em>Plus</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Plus</em>'.
   * @see org.wesnoth.wML.WMLTag#isPlus()
   * @see #getWMLTag()
   * @generated
   */
  EAttribute getWMLTag_Plus();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wML.WMLTag#getName <em>Name</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Name</em>'.
   * @see org.wesnoth.wML.WMLTag#getName()
   * @see #getWMLTag()
   * @generated
   */
  EAttribute getWMLTag_Name();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.WMLTag#getTags <em>Tags</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Tags</em>'.
   * @see org.wesnoth.wML.WMLTag#getTags()
   * @see #getWMLTag()
   * @generated
   */
  EReference getWMLTag_Tags();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.WMLTag#getKeys <em>Keys</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Keys</em>'.
   * @see org.wesnoth.wML.WMLTag#getKeys()
   * @see #getWMLTag()
   * @generated
   */
  EReference getWMLTag_Keys();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.WMLTag#getMacroCalls <em>Macro Calls</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Macro Calls</em>'.
   * @see org.wesnoth.wML.WMLTag#getMacroCalls()
   * @see #getWMLTag()
   * @generated
   */
  EReference getWMLTag_MacroCalls();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.WMLTag#getMacroDefines <em>Macro Defines</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Macro Defines</em>'.
   * @see org.wesnoth.wML.WMLTag#getMacroDefines()
   * @see #getWMLTag()
   * @generated
   */
  EReference getWMLTag_MacroDefines();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.WMLTag#getTextdomains <em>Textdomains</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Textdomains</em>'.
   * @see org.wesnoth.wML.WMLTag#getTextdomains()
   * @see #getWMLTag()
   * @generated
   */
  EReference getWMLTag_Textdomains();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wML.WMLTag#getEndName <em>End Name</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>End Name</em>'.
   * @see org.wesnoth.wML.WMLTag#getEndName()
   * @see #getWMLTag()
   * @generated
   */
  EAttribute getWMLTag_EndName();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wML.WMLKey <em>Key</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>Key</em>'.
   * @see org.wesnoth.wML.WMLKey
   * @generated
   */
  EClass getWMLKey();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wML.WMLKey#getName <em>Name</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Name</em>'.
   * @see org.wesnoth.wML.WMLKey#getName()
   * @see #getWMLKey()
   * @generated
   */
  EAttribute getWMLKey_Name();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.WMLKey#getValue <em>Value</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Value</em>'.
   * @see org.wesnoth.wML.WMLKey#getValue()
   * @see #getWMLKey()
   * @generated
   */
  EReference getWMLKey_Value();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wML.WMLKeyValue <em>Key Value</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>Key Value</em>'.
   * @see org.wesnoth.wML.WMLKeyValue
   * @generated
   */
  EClass getWMLKeyValue();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wML.WMLMacroCall <em>Macro Call</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>Macro Call</em>'.
   * @see org.wesnoth.wML.WMLMacroCall
   * @generated
   */
  EClass getWMLMacroCall();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wML.WMLMacroCall#getName <em>Name</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Name</em>'.
   * @see org.wesnoth.wML.WMLMacroCall#getName()
   * @see #getWMLMacroCall()
   * @generated
   */
  EAttribute getWMLMacroCall_Name();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wML.WMLLuaCode <em>Lua Code</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>Lua Code</em>'.
   * @see org.wesnoth.wML.WMLLuaCode
   * @generated
   */
  EClass getWMLLuaCode();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wML.WMLLuaCode#getValue <em>Value</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Value</em>'.
   * @see org.wesnoth.wML.WMLLuaCode#getValue()
   * @see #getWMLLuaCode()
   * @generated
   */
  EAttribute getWMLLuaCode_Value();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wML.WMLArrayCall <em>Array Call</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>Array Call</em>'.
   * @see org.wesnoth.wML.WMLArrayCall
   * @generated
   */
  EClass getWMLArrayCall();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.WMLArrayCall#getValue <em>Value</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Value</em>'.
   * @see org.wesnoth.wML.WMLArrayCall#getValue()
   * @see #getWMLArrayCall()
   * @generated
   */
  EReference getWMLArrayCall_Value();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wML.WMLMacroDefine <em>Macro Define</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>Macro Define</em>'.
   * @see org.wesnoth.wML.WMLMacroDefine
   * @generated
   */
  EClass getWMLMacroDefine();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wML.WMLMacroDefine#getName <em>Name</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Name</em>'.
   * @see org.wesnoth.wML.WMLMacroDefine#getName()
   * @see #getWMLMacroDefine()
   * @generated
   */
  EAttribute getWMLMacroDefine_Name();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wML.WMLTextdomain <em>Textdomain</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>Textdomain</em>'.
   * @see org.wesnoth.wML.WMLTextdomain
   * @generated
   */
  EClass getWMLTextdomain();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wML.WMLTextdomain#getName <em>Name</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Name</em>'.
   * @see org.wesnoth.wML.WMLTextdomain#getName()
   * @see #getWMLTextdomain()
   * @generated
   */
  EAttribute getWMLTextdomain_Name();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wML.WMLValue <em>Value</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>Value</em>'.
   * @see org.wesnoth.wML.WMLValue
   * @generated
   */
  EClass getWMLValue();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wML.WMLValue#getValue <em>Value</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Value</em>'.
   * @see org.wesnoth.wML.WMLValue#getValue()
   * @see #getWMLValue()
   * @generated
   */
  EAttribute getWMLValue_Value();

  /**
   * Returns the factory that creates the instances of the model.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the factory that creates the instances of the model.
   * @generated
   */
  WMLFactory getWMLFactory();

  /**
   * <!-- begin-user-doc -->
   * Defines literals for the meta objects that represent
   * <ul>
   *   <li>each class,</li>
   *   <li>each feature of each class,</li>
   *   <li>each enum,</li>
   *   <li>and each data type</li>
   * </ul>
   * <!-- end-user-doc -->
   * @generated
   */
  interface Literals
  {
    /**
     * The meta object literal for the '{@link org.wesnoth.wML.impl.WMLRootImpl <em>Root</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wML.impl.WMLRootImpl
     * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLRoot()
     * @generated
     */
    EClass WML_ROOT = eINSTANCE.getWMLRoot();

    /**
     * The meta object literal for the '<em><b>Tags</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_ROOT__TAGS = eINSTANCE.getWMLRoot_Tags();

    /**
     * The meta object literal for the '<em><b>Macro Calls</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_ROOT__MACRO_CALLS = eINSTANCE.getWMLRoot_MacroCalls();

    /**
     * The meta object literal for the '<em><b>Macro Defines</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_ROOT__MACRO_DEFINES = eINSTANCE.getWMLRoot_MacroDefines();

    /**
     * The meta object literal for the '<em><b>Textdomains</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_ROOT__TEXTDOMAINS = eINSTANCE.getWMLRoot_Textdomains();

    /**
     * The meta object literal for the '{@link org.wesnoth.wML.impl.WMLTagImpl <em>Tag</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wML.impl.WMLTagImpl
     * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLTag()
     * @generated
     */
    EClass WML_TAG = eINSTANCE.getWMLTag();

    /**
     * The meta object literal for the '<em><b>Plus</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_TAG__PLUS = eINSTANCE.getWMLTag_Plus();

    /**
     * The meta object literal for the '<em><b>Name</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_TAG__NAME = eINSTANCE.getWMLTag_Name();

    /**
     * The meta object literal for the '<em><b>Tags</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_TAG__TAGS = eINSTANCE.getWMLTag_Tags();

    /**
     * The meta object literal for the '<em><b>Keys</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_TAG__KEYS = eINSTANCE.getWMLTag_Keys();

    /**
     * The meta object literal for the '<em><b>Macro Calls</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_TAG__MACRO_CALLS = eINSTANCE.getWMLTag_MacroCalls();

    /**
     * The meta object literal for the '<em><b>Macro Defines</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_TAG__MACRO_DEFINES = eINSTANCE.getWMLTag_MacroDefines();

    /**
     * The meta object literal for the '<em><b>Textdomains</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_TAG__TEXTDOMAINS = eINSTANCE.getWMLTag_Textdomains();

    /**
     * The meta object literal for the '<em><b>End Name</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_TAG__END_NAME = eINSTANCE.getWMLTag_EndName();

    /**
     * The meta object literal for the '{@link org.wesnoth.wML.impl.WMLKeyImpl <em>Key</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wML.impl.WMLKeyImpl
     * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLKey()
     * @generated
     */
    EClass WML_KEY = eINSTANCE.getWMLKey();

    /**
     * The meta object literal for the '<em><b>Name</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_KEY__NAME = eINSTANCE.getWMLKey_Name();

    /**
     * The meta object literal for the '<em><b>Value</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_KEY__VALUE = eINSTANCE.getWMLKey_Value();

    /**
     * The meta object literal for the '{@link org.wesnoth.wML.impl.WMLKeyValueImpl <em>Key Value</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wML.impl.WMLKeyValueImpl
     * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLKeyValue()
     * @generated
     */
    EClass WML_KEY_VALUE = eINSTANCE.getWMLKeyValue();

    /**
     * The meta object literal for the '{@link org.wesnoth.wML.impl.WMLMacroCallImpl <em>Macro Call</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wML.impl.WMLMacroCallImpl
     * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLMacroCall()
     * @generated
     */
    EClass WML_MACRO_CALL = eINSTANCE.getWMLMacroCall();

    /**
     * The meta object literal for the '<em><b>Name</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_MACRO_CALL__NAME = eINSTANCE.getWMLMacroCall_Name();

    /**
     * The meta object literal for the '{@link org.wesnoth.wML.impl.WMLLuaCodeImpl <em>Lua Code</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wML.impl.WMLLuaCodeImpl
     * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLLuaCode()
     * @generated
     */
    EClass WML_LUA_CODE = eINSTANCE.getWMLLuaCode();

    /**
     * The meta object literal for the '<em><b>Value</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_LUA_CODE__VALUE = eINSTANCE.getWMLLuaCode_Value();

    /**
     * The meta object literal for the '{@link org.wesnoth.wML.impl.WMLArrayCallImpl <em>Array Call</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wML.impl.WMLArrayCallImpl
     * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLArrayCall()
     * @generated
     */
    EClass WML_ARRAY_CALL = eINSTANCE.getWMLArrayCall();

    /**
     * The meta object literal for the '<em><b>Value</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_ARRAY_CALL__VALUE = eINSTANCE.getWMLArrayCall_Value();

    /**
     * The meta object literal for the '{@link org.wesnoth.wML.impl.WMLMacroDefineImpl <em>Macro Define</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wML.impl.WMLMacroDefineImpl
     * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLMacroDefine()
     * @generated
     */
    EClass WML_MACRO_DEFINE = eINSTANCE.getWMLMacroDefine();

    /**
     * The meta object literal for the '<em><b>Name</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_MACRO_DEFINE__NAME = eINSTANCE.getWMLMacroDefine_Name();

    /**
     * The meta object literal for the '{@link org.wesnoth.wML.impl.WMLTextdomainImpl <em>Textdomain</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wML.impl.WMLTextdomainImpl
     * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLTextdomain()
     * @generated
     */
    EClass WML_TEXTDOMAIN = eINSTANCE.getWMLTextdomain();

    /**
     * The meta object literal for the '<em><b>Name</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_TEXTDOMAIN__NAME = eINSTANCE.getWMLTextdomain_Name();

    /**
     * The meta object literal for the '{@link org.wesnoth.wML.impl.WMLValueImpl <em>Value</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wML.impl.WMLValueImpl
     * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLValue()
     * @generated
     */
    EClass WML_VALUE = eINSTANCE.getWMLValue();

    /**
     * The meta object literal for the '<em><b>Value</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_VALUE__VALUE = eINSTANCE.getWMLValue_Value();

  }

} //WMLPackage
