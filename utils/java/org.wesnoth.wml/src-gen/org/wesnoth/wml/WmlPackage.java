/**
 * <copyright>
 * </copyright>
 *

 */
package org.wesnoth.wml;

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
 * @see org.wesnoth.wml.WmlFactory
 * @model kind="package"
 * @generated
 */
public interface WmlPackage extends EPackage
{
  /**
   * The package name.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  String eNAME = "wml";

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
  String eNS_PREFIX = "wml";

  /**
   * The singleton instance of the package.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   */
  WmlPackage eINSTANCE = org.wesnoth.wml.impl.WmlPackageImpl.init();

  /**
   * The meta object id for the '{@link org.wesnoth.wml.impl.WMLRootImpl <em>WML Root</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wml.impl.WMLRootImpl
   * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLRoot()
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
   * The feature id for the '<em><b>If Defs</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_ROOT__IF_DEFS = 4;

  /**
   * The number of structural features of the '<em>WML Root</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_ROOT_FEATURE_COUNT = 5;

  /**
   * The meta object id for the '{@link org.wesnoth.wml.impl.WMLTagImpl <em>WML Tag</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wml.impl.WMLTagImpl
   * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLTag()
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
   * The feature id for the '<em><b>If Defs</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG__IF_DEFS = 7;

  /**
   * The feature id for the '<em><b>End Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG__END_NAME = 8;

  /**
   * The number of structural features of the '<em>WML Tag</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG_FEATURE_COUNT = 9;

  /**
   * The meta object id for the '{@link org.wesnoth.wml.impl.WMLKeyImpl <em>WML Key</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wml.impl.WMLKeyImpl
   * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLKey()
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
   * The feature id for the '<em><b>Eol</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_KEY__EOL = 2;

  /**
   * The number of structural features of the '<em>WML Key</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_KEY_FEATURE_COUNT = 3;

  /**
   * The meta object id for the '{@link org.wesnoth.wml.impl.WMLKeyValueImpl <em>WML Key Value</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wml.impl.WMLKeyValueImpl
   * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLKeyValue()
   * @generated
   */
  int WML_KEY_VALUE = 3;

  /**
   * The number of structural features of the '<em>WML Key Value</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_KEY_VALUE_FEATURE_COUNT = 0;

  /**
   * The meta object id for the '{@link org.wesnoth.wml.impl.WMLMacroCallImpl <em>WML Macro Call</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wml.impl.WMLMacroCallImpl
   * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLMacroCall()
   * @generated
   */
  int WML_MACRO_CALL = 4;

  /**
   * The feature id for the '<em><b>Point</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_CALL__POINT = WML_KEY_VALUE_FEATURE_COUNT + 0;

  /**
   * The feature id for the '<em><b>Relative</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_CALL__RELATIVE = WML_KEY_VALUE_FEATURE_COUNT + 1;

  /**
   * The feature id for the '<em><b>Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_CALL__NAME = WML_KEY_VALUE_FEATURE_COUNT + 2;

  /**
   * The feature id for the '<em><b>Params</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_CALL__PARAMS = WML_KEY_VALUE_FEATURE_COUNT + 3;

  /**
   * The feature id for the '<em><b>Extra Macros</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_CALL__EXTRA_MACROS = WML_KEY_VALUE_FEATURE_COUNT + 4;

  /**
   * The number of structural features of the '<em>WML Macro Call</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_CALL_FEATURE_COUNT = WML_KEY_VALUE_FEATURE_COUNT + 5;

  /**
   * The meta object id for the '{@link org.wesnoth.wml.impl.WMLLuaCodeImpl <em>WML Lua Code</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wml.impl.WMLLuaCodeImpl
   * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLLuaCode()
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
   * The number of structural features of the '<em>WML Lua Code</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_LUA_CODE_FEATURE_COUNT = WML_KEY_VALUE_FEATURE_COUNT + 1;

  /**
   * The meta object id for the '{@link org.wesnoth.wml.impl.WMLArrayCallImpl <em>WML Array Call</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wml.impl.WMLArrayCallImpl
   * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLArrayCall()
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
   * The number of structural features of the '<em>WML Array Call</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_ARRAY_CALL_FEATURE_COUNT = WML_KEY_VALUE_FEATURE_COUNT + 1;

  /**
   * The meta object id for the '{@link org.wesnoth.wml.impl.WMLMacroDefineImpl <em>WML Macro Define</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wml.impl.WMLMacroDefineImpl
   * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLMacroDefine()
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
   * The feature id for the '<em><b>Tags</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_DEFINE__TAGS = 1;

  /**
   * The feature id for the '<em><b>Keys</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_DEFINE__KEYS = 2;

  /**
   * The feature id for the '<em><b>Macro Calls</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_DEFINE__MACRO_CALLS = 3;

  /**
   * The feature id for the '<em><b>Macro Defines</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_DEFINE__MACRO_DEFINES = 4;

  /**
   * The feature id for the '<em><b>Textdomains</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_DEFINE__TEXTDOMAINS = 5;

  /**
   * The feature id for the '<em><b>Values</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_DEFINE__VALUES = 6;

  /**
   * The feature id for the '<em><b>If Defs</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_DEFINE__IF_DEFS = 7;

  /**
   * The feature id for the '<em><b>End Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_DEFINE__END_NAME = 8;

  /**
   * The number of structural features of the '<em>WML Macro Define</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_DEFINE_FEATURE_COUNT = 9;

  /**
   * The meta object id for the '{@link org.wesnoth.wml.impl.WMLPreprocIFImpl <em>WML Preproc IF</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wml.impl.WMLPreprocIFImpl
   * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLPreprocIF()
   * @generated
   */
  int WML_PREPROC_IF = 8;

  /**
   * The feature id for the '<em><b>Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_PREPROC_IF__NAME = 0;

  /**
   * The feature id for the '<em><b>Tags</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_PREPROC_IF__TAGS = 1;

  /**
   * The feature id for the '<em><b>Keys</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_PREPROC_IF__KEYS = 2;

  /**
   * The feature id for the '<em><b>Macro Calls</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_PREPROC_IF__MACRO_CALLS = 3;

  /**
   * The feature id for the '<em><b>Macro Defines</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_PREPROC_IF__MACRO_DEFINES = 4;

  /**
   * The feature id for the '<em><b>Textdomains</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_PREPROC_IF__TEXTDOMAINS = 5;

  /**
   * The feature id for the '<em><b>Values</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_PREPROC_IF__VALUES = 6;

  /**
   * The feature id for the '<em><b>If Defs</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_PREPROC_IF__IF_DEFS = 7;

  /**
   * The feature id for the '<em><b>Elses</b></em>' attribute list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_PREPROC_IF__ELSES = 8;

  /**
   * The feature id for the '<em><b>End Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_PREPROC_IF__END_NAME = 9;

  /**
   * The number of structural features of the '<em>WML Preproc IF</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_PREPROC_IF_FEATURE_COUNT = 10;

  /**
   * The meta object id for the '{@link org.wesnoth.wml.impl.WMLTextdomainImpl <em>WML Textdomain</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wml.impl.WMLTextdomainImpl
   * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLTextdomain()
   * @generated
   */
  int WML_TEXTDOMAIN = 9;

  /**
   * The feature id for the '<em><b>Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TEXTDOMAIN__NAME = 0;

  /**
   * The number of structural features of the '<em>WML Textdomain</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TEXTDOMAIN_FEATURE_COUNT = 1;

  /**
   * The meta object id for the '{@link org.wesnoth.wml.impl.WMLValueImpl <em>WML Value</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wml.impl.WMLValueImpl
   * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLValue()
   * @generated
   */
  int WML_VALUE = 10;

  /**
   * The feature id for the '<em><b>Value</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_VALUE__VALUE = WML_KEY_VALUE_FEATURE_COUNT + 0;

  /**
   * The number of structural features of the '<em>WML Value</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_VALUE_FEATURE_COUNT = WML_KEY_VALUE_FEATURE_COUNT + 1;

  /**
   * The meta object id for the '{@link org.wesnoth.wml.impl.MacroTokensImpl <em>Macro Tokens</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wml.impl.MacroTokensImpl
   * @see org.wesnoth.wml.impl.WmlPackageImpl#getMacroTokens()
   * @generated
   */
  int MACRO_TOKENS = 11;

  /**
   * The feature id for the '<em><b>Val</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int MACRO_TOKENS__VAL = 0;

  /**
   * The number of structural features of the '<em>Macro Tokens</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int MACRO_TOKENS_FEATURE_COUNT = 1;


  /**
   * Returns the meta object for class '{@link org.wesnoth.wml.WMLRoot <em>WML Root</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>WML Root</em>'.
   * @see org.wesnoth.wml.WMLRoot
   * @generated
   */
  EClass getWMLRoot();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wml.WMLRoot#getTags <em>Tags</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Tags</em>'.
   * @see org.wesnoth.wml.WMLRoot#getTags()
   * @see #getWMLRoot()
   * @generated
   */
  EReference getWMLRoot_Tags();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wml.WMLRoot#getMacroCalls <em>Macro Calls</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Macro Calls</em>'.
   * @see org.wesnoth.wml.WMLRoot#getMacroCalls()
   * @see #getWMLRoot()
   * @generated
   */
  EReference getWMLRoot_MacroCalls();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wml.WMLRoot#getMacroDefines <em>Macro Defines</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Macro Defines</em>'.
   * @see org.wesnoth.wml.WMLRoot#getMacroDefines()
   * @see #getWMLRoot()
   * @generated
   */
  EReference getWMLRoot_MacroDefines();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wml.WMLRoot#getTextdomains <em>Textdomains</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Textdomains</em>'.
   * @see org.wesnoth.wml.WMLRoot#getTextdomains()
   * @see #getWMLRoot()
   * @generated
   */
  EReference getWMLRoot_Textdomains();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wml.WMLRoot#getIfDefs <em>If Defs</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>If Defs</em>'.
   * @see org.wesnoth.wml.WMLRoot#getIfDefs()
   * @see #getWMLRoot()
   * @generated
   */
  EReference getWMLRoot_IfDefs();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wml.WMLTag <em>WML Tag</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>WML Tag</em>'.
   * @see org.wesnoth.wml.WMLTag
   * @generated
   */
  EClass getWMLTag();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wml.WMLTag#isPlus <em>Plus</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Plus</em>'.
   * @see org.wesnoth.wml.WMLTag#isPlus()
   * @see #getWMLTag()
   * @generated
   */
  EAttribute getWMLTag_Plus();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wml.WMLTag#getName <em>Name</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Name</em>'.
   * @see org.wesnoth.wml.WMLTag#getName()
   * @see #getWMLTag()
   * @generated
   */
  EAttribute getWMLTag_Name();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wml.WMLTag#getTags <em>Tags</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Tags</em>'.
   * @see org.wesnoth.wml.WMLTag#getTags()
   * @see #getWMLTag()
   * @generated
   */
  EReference getWMLTag_Tags();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wml.WMLTag#getKeys <em>Keys</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Keys</em>'.
   * @see org.wesnoth.wml.WMLTag#getKeys()
   * @see #getWMLTag()
   * @generated
   */
  EReference getWMLTag_Keys();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wml.WMLTag#getMacroCalls <em>Macro Calls</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Macro Calls</em>'.
   * @see org.wesnoth.wml.WMLTag#getMacroCalls()
   * @see #getWMLTag()
   * @generated
   */
  EReference getWMLTag_MacroCalls();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wml.WMLTag#getMacroDefines <em>Macro Defines</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Macro Defines</em>'.
   * @see org.wesnoth.wml.WMLTag#getMacroDefines()
   * @see #getWMLTag()
   * @generated
   */
  EReference getWMLTag_MacroDefines();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wml.WMLTag#getTextdomains <em>Textdomains</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Textdomains</em>'.
   * @see org.wesnoth.wml.WMLTag#getTextdomains()
   * @see #getWMLTag()
   * @generated
   */
  EReference getWMLTag_Textdomains();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wml.WMLTag#getIfDefs <em>If Defs</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>If Defs</em>'.
   * @see org.wesnoth.wml.WMLTag#getIfDefs()
   * @see #getWMLTag()
   * @generated
   */
  EReference getWMLTag_IfDefs();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wml.WMLTag#getEndName <em>End Name</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>End Name</em>'.
   * @see org.wesnoth.wml.WMLTag#getEndName()
   * @see #getWMLTag()
   * @generated
   */
  EAttribute getWMLTag_EndName();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wml.WMLKey <em>WML Key</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>WML Key</em>'.
   * @see org.wesnoth.wml.WMLKey
   * @generated
   */
  EClass getWMLKey();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wml.WMLKey#getName <em>Name</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Name</em>'.
   * @see org.wesnoth.wml.WMLKey#getName()
   * @see #getWMLKey()
   * @generated
   */
  EAttribute getWMLKey_Name();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wml.WMLKey#getValue <em>Value</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Value</em>'.
   * @see org.wesnoth.wml.WMLKey#getValue()
   * @see #getWMLKey()
   * @generated
   */
  EReference getWMLKey_Value();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wml.WMLKey#getEol <em>Eol</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Eol</em>'.
   * @see org.wesnoth.wml.WMLKey#getEol()
   * @see #getWMLKey()
   * @generated
   */
  EAttribute getWMLKey_Eol();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wml.WMLKeyValue <em>WML Key Value</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>WML Key Value</em>'.
   * @see org.wesnoth.wml.WMLKeyValue
   * @generated
   */
  EClass getWMLKeyValue();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wml.WMLMacroCall <em>WML Macro Call</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>WML Macro Call</em>'.
   * @see org.wesnoth.wml.WMLMacroCall
   * @generated
   */
  EClass getWMLMacroCall();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wml.WMLMacroCall#isPoint <em>Point</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Point</em>'.
   * @see org.wesnoth.wml.WMLMacroCall#isPoint()
   * @see #getWMLMacroCall()
   * @generated
   */
  EAttribute getWMLMacroCall_Point();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wml.WMLMacroCall#isRelative <em>Relative</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Relative</em>'.
   * @see org.wesnoth.wml.WMLMacroCall#isRelative()
   * @see #getWMLMacroCall()
   * @generated
   */
  EAttribute getWMLMacroCall_Relative();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wml.WMLMacroCall#getName <em>Name</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Name</em>'.
   * @see org.wesnoth.wml.WMLMacroCall#getName()
   * @see #getWMLMacroCall()
   * @generated
   */
  EAttribute getWMLMacroCall_Name();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wml.WMLMacroCall#getParams <em>Params</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Params</em>'.
   * @see org.wesnoth.wml.WMLMacroCall#getParams()
   * @see #getWMLMacroCall()
   * @generated
   */
  EReference getWMLMacroCall_Params();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wml.WMLMacroCall#getExtraMacros <em>Extra Macros</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Extra Macros</em>'.
   * @see org.wesnoth.wml.WMLMacroCall#getExtraMacros()
   * @see #getWMLMacroCall()
   * @generated
   */
  EReference getWMLMacroCall_ExtraMacros();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wml.WMLLuaCode <em>WML Lua Code</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>WML Lua Code</em>'.
   * @see org.wesnoth.wml.WMLLuaCode
   * @generated
   */
  EClass getWMLLuaCode();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wml.WMLLuaCode#getValue <em>Value</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Value</em>'.
   * @see org.wesnoth.wml.WMLLuaCode#getValue()
   * @see #getWMLLuaCode()
   * @generated
   */
  EAttribute getWMLLuaCode_Value();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wml.WMLArrayCall <em>WML Array Call</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>WML Array Call</em>'.
   * @see org.wesnoth.wml.WMLArrayCall
   * @generated
   */
  EClass getWMLArrayCall();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wml.WMLArrayCall#getValue <em>Value</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Value</em>'.
   * @see org.wesnoth.wml.WMLArrayCall#getValue()
   * @see #getWMLArrayCall()
   * @generated
   */
  EReference getWMLArrayCall_Value();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wml.WMLMacroDefine <em>WML Macro Define</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>WML Macro Define</em>'.
   * @see org.wesnoth.wml.WMLMacroDefine
   * @generated
   */
  EClass getWMLMacroDefine();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wml.WMLMacroDefine#getName <em>Name</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Name</em>'.
   * @see org.wesnoth.wml.WMLMacroDefine#getName()
   * @see #getWMLMacroDefine()
   * @generated
   */
  EAttribute getWMLMacroDefine_Name();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wml.WMLMacroDefine#getTags <em>Tags</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Tags</em>'.
   * @see org.wesnoth.wml.WMLMacroDefine#getTags()
   * @see #getWMLMacroDefine()
   * @generated
   */
  EReference getWMLMacroDefine_Tags();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wml.WMLMacroDefine#getKeys <em>Keys</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Keys</em>'.
   * @see org.wesnoth.wml.WMLMacroDefine#getKeys()
   * @see #getWMLMacroDefine()
   * @generated
   */
  EReference getWMLMacroDefine_Keys();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wml.WMLMacroDefine#getMacroCalls <em>Macro Calls</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Macro Calls</em>'.
   * @see org.wesnoth.wml.WMLMacroDefine#getMacroCalls()
   * @see #getWMLMacroDefine()
   * @generated
   */
  EReference getWMLMacroDefine_MacroCalls();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wml.WMLMacroDefine#getMacroDefines <em>Macro Defines</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Macro Defines</em>'.
   * @see org.wesnoth.wml.WMLMacroDefine#getMacroDefines()
   * @see #getWMLMacroDefine()
   * @generated
   */
  EReference getWMLMacroDefine_MacroDefines();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wml.WMLMacroDefine#getTextdomains <em>Textdomains</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Textdomains</em>'.
   * @see org.wesnoth.wml.WMLMacroDefine#getTextdomains()
   * @see #getWMLMacroDefine()
   * @generated
   */
  EReference getWMLMacroDefine_Textdomains();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wml.WMLMacroDefine#getValues <em>Values</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Values</em>'.
   * @see org.wesnoth.wml.WMLMacroDefine#getValues()
   * @see #getWMLMacroDefine()
   * @generated
   */
  EReference getWMLMacroDefine_Values();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wml.WMLMacroDefine#getIfDefs <em>If Defs</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>If Defs</em>'.
   * @see org.wesnoth.wml.WMLMacroDefine#getIfDefs()
   * @see #getWMLMacroDefine()
   * @generated
   */
  EReference getWMLMacroDefine_IfDefs();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wml.WMLMacroDefine#getEndName <em>End Name</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>End Name</em>'.
   * @see org.wesnoth.wml.WMLMacroDefine#getEndName()
   * @see #getWMLMacroDefine()
   * @generated
   */
  EAttribute getWMLMacroDefine_EndName();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wml.WMLPreprocIF <em>WML Preproc IF</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>WML Preproc IF</em>'.
   * @see org.wesnoth.wml.WMLPreprocIF
   * @generated
   */
  EClass getWMLPreprocIF();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wml.WMLPreprocIF#getName <em>Name</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Name</em>'.
   * @see org.wesnoth.wml.WMLPreprocIF#getName()
   * @see #getWMLPreprocIF()
   * @generated
   */
  EAttribute getWMLPreprocIF_Name();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wml.WMLPreprocIF#getTags <em>Tags</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Tags</em>'.
   * @see org.wesnoth.wml.WMLPreprocIF#getTags()
   * @see #getWMLPreprocIF()
   * @generated
   */
  EReference getWMLPreprocIF_Tags();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wml.WMLPreprocIF#getKeys <em>Keys</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Keys</em>'.
   * @see org.wesnoth.wml.WMLPreprocIF#getKeys()
   * @see #getWMLPreprocIF()
   * @generated
   */
  EReference getWMLPreprocIF_Keys();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wml.WMLPreprocIF#getMacroCalls <em>Macro Calls</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Macro Calls</em>'.
   * @see org.wesnoth.wml.WMLPreprocIF#getMacroCalls()
   * @see #getWMLPreprocIF()
   * @generated
   */
  EReference getWMLPreprocIF_MacroCalls();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wml.WMLPreprocIF#getMacroDefines <em>Macro Defines</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Macro Defines</em>'.
   * @see org.wesnoth.wml.WMLPreprocIF#getMacroDefines()
   * @see #getWMLPreprocIF()
   * @generated
   */
  EReference getWMLPreprocIF_MacroDefines();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wml.WMLPreprocIF#getTextdomains <em>Textdomains</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Textdomains</em>'.
   * @see org.wesnoth.wml.WMLPreprocIF#getTextdomains()
   * @see #getWMLPreprocIF()
   * @generated
   */
  EReference getWMLPreprocIF_Textdomains();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wml.WMLPreprocIF#getValues <em>Values</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Values</em>'.
   * @see org.wesnoth.wml.WMLPreprocIF#getValues()
   * @see #getWMLPreprocIF()
   * @generated
   */
  EReference getWMLPreprocIF_Values();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wml.WMLPreprocIF#getIfDefs <em>If Defs</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>If Defs</em>'.
   * @see org.wesnoth.wml.WMLPreprocIF#getIfDefs()
   * @see #getWMLPreprocIF()
   * @generated
   */
  EReference getWMLPreprocIF_IfDefs();

  /**
   * Returns the meta object for the attribute list '{@link org.wesnoth.wml.WMLPreprocIF#getElses <em>Elses</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute list '<em>Elses</em>'.
   * @see org.wesnoth.wml.WMLPreprocIF#getElses()
   * @see #getWMLPreprocIF()
   * @generated
   */
  EAttribute getWMLPreprocIF_Elses();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wml.WMLPreprocIF#getEndName <em>End Name</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>End Name</em>'.
   * @see org.wesnoth.wml.WMLPreprocIF#getEndName()
   * @see #getWMLPreprocIF()
   * @generated
   */
  EAttribute getWMLPreprocIF_EndName();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wml.WMLTextdomain <em>WML Textdomain</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>WML Textdomain</em>'.
   * @see org.wesnoth.wml.WMLTextdomain
   * @generated
   */
  EClass getWMLTextdomain();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wml.WMLTextdomain#getName <em>Name</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Name</em>'.
   * @see org.wesnoth.wml.WMLTextdomain#getName()
   * @see #getWMLTextdomain()
   * @generated
   */
  EAttribute getWMLTextdomain_Name();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wml.WMLValue <em>WML Value</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>WML Value</em>'.
   * @see org.wesnoth.wml.WMLValue
   * @generated
   */
  EClass getWMLValue();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wml.WMLValue#getValue <em>Value</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Value</em>'.
   * @see org.wesnoth.wml.WMLValue#getValue()
   * @see #getWMLValue()
   * @generated
   */
  EAttribute getWMLValue_Value();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wml.MacroTokens <em>Macro Tokens</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>Macro Tokens</em>'.
   * @see org.wesnoth.wml.MacroTokens
   * @generated
   */
  EClass getMacroTokens();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wml.MacroTokens#getVal <em>Val</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Val</em>'.
   * @see org.wesnoth.wml.MacroTokens#getVal()
   * @see #getMacroTokens()
   * @generated
   */
  EAttribute getMacroTokens_Val();

  /**
   * Returns the factory that creates the instances of the model.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the factory that creates the instances of the model.
   * @generated
   */
  WmlFactory getWmlFactory();

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
     * The meta object literal for the '{@link org.wesnoth.wml.impl.WMLRootImpl <em>WML Root</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wml.impl.WMLRootImpl
     * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLRoot()
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
     * The meta object literal for the '<em><b>If Defs</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_ROOT__IF_DEFS = eINSTANCE.getWMLRoot_IfDefs();

    /**
     * The meta object literal for the '{@link org.wesnoth.wml.impl.WMLTagImpl <em>WML Tag</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wml.impl.WMLTagImpl
     * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLTag()
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
     * The meta object literal for the '<em><b>If Defs</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_TAG__IF_DEFS = eINSTANCE.getWMLTag_IfDefs();

    /**
     * The meta object literal for the '<em><b>End Name</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_TAG__END_NAME = eINSTANCE.getWMLTag_EndName();

    /**
     * The meta object literal for the '{@link org.wesnoth.wml.impl.WMLKeyImpl <em>WML Key</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wml.impl.WMLKeyImpl
     * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLKey()
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
     * The meta object literal for the '<em><b>Eol</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_KEY__EOL = eINSTANCE.getWMLKey_Eol();

    /**
     * The meta object literal for the '{@link org.wesnoth.wml.impl.WMLKeyValueImpl <em>WML Key Value</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wml.impl.WMLKeyValueImpl
     * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLKeyValue()
     * @generated
     */
    EClass WML_KEY_VALUE = eINSTANCE.getWMLKeyValue();

    /**
     * The meta object literal for the '{@link org.wesnoth.wml.impl.WMLMacroCallImpl <em>WML Macro Call</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wml.impl.WMLMacroCallImpl
     * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLMacroCall()
     * @generated
     */
    EClass WML_MACRO_CALL = eINSTANCE.getWMLMacroCall();

    /**
     * The meta object literal for the '<em><b>Point</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_MACRO_CALL__POINT = eINSTANCE.getWMLMacroCall_Point();

    /**
     * The meta object literal for the '<em><b>Relative</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_MACRO_CALL__RELATIVE = eINSTANCE.getWMLMacroCall_Relative();

    /**
     * The meta object literal for the '<em><b>Name</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_MACRO_CALL__NAME = eINSTANCE.getWMLMacroCall_Name();

    /**
     * The meta object literal for the '<em><b>Params</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_MACRO_CALL__PARAMS = eINSTANCE.getWMLMacroCall_Params();

    /**
     * The meta object literal for the '<em><b>Extra Macros</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_MACRO_CALL__EXTRA_MACROS = eINSTANCE.getWMLMacroCall_ExtraMacros();

    /**
     * The meta object literal for the '{@link org.wesnoth.wml.impl.WMLLuaCodeImpl <em>WML Lua Code</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wml.impl.WMLLuaCodeImpl
     * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLLuaCode()
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
     * The meta object literal for the '{@link org.wesnoth.wml.impl.WMLArrayCallImpl <em>WML Array Call</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wml.impl.WMLArrayCallImpl
     * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLArrayCall()
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
     * The meta object literal for the '{@link org.wesnoth.wml.impl.WMLMacroDefineImpl <em>WML Macro Define</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wml.impl.WMLMacroDefineImpl
     * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLMacroDefine()
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
     * The meta object literal for the '<em><b>Tags</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_MACRO_DEFINE__TAGS = eINSTANCE.getWMLMacroDefine_Tags();

    /**
     * The meta object literal for the '<em><b>Keys</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_MACRO_DEFINE__KEYS = eINSTANCE.getWMLMacroDefine_Keys();

    /**
     * The meta object literal for the '<em><b>Macro Calls</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_MACRO_DEFINE__MACRO_CALLS = eINSTANCE.getWMLMacroDefine_MacroCalls();

    /**
     * The meta object literal for the '<em><b>Macro Defines</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_MACRO_DEFINE__MACRO_DEFINES = eINSTANCE.getWMLMacroDefine_MacroDefines();

    /**
     * The meta object literal for the '<em><b>Textdomains</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_MACRO_DEFINE__TEXTDOMAINS = eINSTANCE.getWMLMacroDefine_Textdomains();

    /**
     * The meta object literal for the '<em><b>Values</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_MACRO_DEFINE__VALUES = eINSTANCE.getWMLMacroDefine_Values();

    /**
     * The meta object literal for the '<em><b>If Defs</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_MACRO_DEFINE__IF_DEFS = eINSTANCE.getWMLMacroDefine_IfDefs();

    /**
     * The meta object literal for the '<em><b>End Name</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_MACRO_DEFINE__END_NAME = eINSTANCE.getWMLMacroDefine_EndName();

    /**
     * The meta object literal for the '{@link org.wesnoth.wml.impl.WMLPreprocIFImpl <em>WML Preproc IF</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wml.impl.WMLPreprocIFImpl
     * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLPreprocIF()
     * @generated
     */
    EClass WML_PREPROC_IF = eINSTANCE.getWMLPreprocIF();

    /**
     * The meta object literal for the '<em><b>Name</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_PREPROC_IF__NAME = eINSTANCE.getWMLPreprocIF_Name();

    /**
     * The meta object literal for the '<em><b>Tags</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_PREPROC_IF__TAGS = eINSTANCE.getWMLPreprocIF_Tags();

    /**
     * The meta object literal for the '<em><b>Keys</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_PREPROC_IF__KEYS = eINSTANCE.getWMLPreprocIF_Keys();

    /**
     * The meta object literal for the '<em><b>Macro Calls</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_PREPROC_IF__MACRO_CALLS = eINSTANCE.getWMLPreprocIF_MacroCalls();

    /**
     * The meta object literal for the '<em><b>Macro Defines</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_PREPROC_IF__MACRO_DEFINES = eINSTANCE.getWMLPreprocIF_MacroDefines();

    /**
     * The meta object literal for the '<em><b>Textdomains</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_PREPROC_IF__TEXTDOMAINS = eINSTANCE.getWMLPreprocIF_Textdomains();

    /**
     * The meta object literal for the '<em><b>Values</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_PREPROC_IF__VALUES = eINSTANCE.getWMLPreprocIF_Values();

    /**
     * The meta object literal for the '<em><b>If Defs</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_PREPROC_IF__IF_DEFS = eINSTANCE.getWMLPreprocIF_IfDefs();

    /**
     * The meta object literal for the '<em><b>Elses</b></em>' attribute list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_PREPROC_IF__ELSES = eINSTANCE.getWMLPreprocIF_Elses();

    /**
     * The meta object literal for the '<em><b>End Name</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_PREPROC_IF__END_NAME = eINSTANCE.getWMLPreprocIF_EndName();

    /**
     * The meta object literal for the '{@link org.wesnoth.wml.impl.WMLTextdomainImpl <em>WML Textdomain</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wml.impl.WMLTextdomainImpl
     * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLTextdomain()
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
     * The meta object literal for the '{@link org.wesnoth.wml.impl.WMLValueImpl <em>WML Value</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wml.impl.WMLValueImpl
     * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLValue()
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

    /**
     * The meta object literal for the '{@link org.wesnoth.wml.impl.MacroTokensImpl <em>Macro Tokens</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wml.impl.MacroTokensImpl
     * @see org.wesnoth.wml.impl.WmlPackageImpl#getMacroTokens()
     * @generated
     */
    EClass MACRO_TOKENS = eINSTANCE.getMacroTokens();

    /**
     * The meta object literal for the '<em><b>Val</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute MACRO_TOKENS__VAL = eINSTANCE.getMacroTokens_Val();

  }

} //WmlPackage
