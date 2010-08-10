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
   * The feature id for the '<em><b>Macros</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_ROOT__MACROS = 1;

  /**
   * The feature id for the '<em><b>Macros Defines</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_ROOT__MACROS_DEFINES = 2;

  /**
   * The number of structural features of the '<em>Root</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_ROOT_FEATURE_COUNT = 3;

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
   * The feature id for the '<em><b>Macros</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG__MACROS = 3;

  /**
   * The feature id for the '<em><b>Macros Defines</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG__MACROS_DEFINES = 4;

  /**
   * The feature id for the '<em><b>Keys</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG__KEYS = 5;

  /**
   * The feature id for the '<em><b>End Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG__END_NAME = 6;

  /**
   * The number of structural features of the '<em>Tag</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG_FEATURE_COUNT = 7;

  /**
   * The meta object id for the '{@link org.wesnoth.wML.impl.WMLAbstractMacroCallImpl <em>Abstract Macro Call</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wML.impl.WMLAbstractMacroCallImpl
   * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLAbstractMacroCall()
   * @generated
   */
  int WML_ABSTRACT_MACRO_CALL = 2;

  /**
   * The feature id for the '<em><b>Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_ABSTRACT_MACRO_CALL__NAME = 0;

  /**
   * The number of structural features of the '<em>Abstract Macro Call</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_ABSTRACT_MACRO_CALL_FEATURE_COUNT = 1;

  /**
   * The meta object id for the '{@link org.wesnoth.wML.impl.WMLMacroIncludeImpl <em>Macro Include</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wML.impl.WMLMacroIncludeImpl
   * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLMacroInclude()
   * @generated
   */
  int WML_MACRO_INCLUDE = 3;

  /**
   * The feature id for the '<em><b>Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_INCLUDE__NAME = WML_ABSTRACT_MACRO_CALL__NAME;

  /**
   * The number of structural features of the '<em>Macro Include</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_INCLUDE_FEATURE_COUNT = WML_ABSTRACT_MACRO_CALL_FEATURE_COUNT + 0;

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
  int WML_MACRO_CALL__NAME = WML_ABSTRACT_MACRO_CALL__NAME;

  /**
   * The feature id for the '<em><b>Args</b></em>' attribute list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_CALL__ARGS = WML_ABSTRACT_MACRO_CALL_FEATURE_COUNT + 0;

  /**
   * The feature id for the '<em><b>Params</b></em>' attribute list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_CALL__PARAMS = WML_ABSTRACT_MACRO_CALL_FEATURE_COUNT + 1;

  /**
   * The feature id for the '<em><b>Tags</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_CALL__TAGS = WML_ABSTRACT_MACRO_CALL_FEATURE_COUNT + 2;

  /**
   * The feature id for the '<em><b>Macros</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_CALL__MACROS = WML_ABSTRACT_MACRO_CALL_FEATURE_COUNT + 3;

  /**
   * The feature id for the '<em><b>Macros Defines</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_CALL__MACROS_DEFINES = WML_ABSTRACT_MACRO_CALL_FEATURE_COUNT + 4;

  /**
   * The feature id for the '<em><b>Keys</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_CALL__KEYS = WML_ABSTRACT_MACRO_CALL_FEATURE_COUNT + 5;

  /**
   * The number of structural features of the '<em>Macro Call</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_CALL_FEATURE_COUNT = WML_ABSTRACT_MACRO_CALL_FEATURE_COUNT + 6;

  /**
   * The meta object id for the '{@link org.wesnoth.wML.impl.WMLMacroDefineImpl <em>Macro Define</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wML.impl.WMLMacroDefineImpl
   * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLMacroDefine()
   * @generated
   */
  int WML_MACRO_DEFINE = 5;

  /**
   * The feature id for the '<em><b>Params</b></em>' attribute list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_DEFINE__PARAMS = 0;

  /**
   * The feature id for the '<em><b>Tags</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_DEFINE__TAGS = 1;

  /**
   * The feature id for the '<em><b>Macros</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_DEFINE__MACROS = 2;

  /**
   * The feature id for the '<em><b>Macros Defines</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_DEFINE__MACROS_DEFINES = 3;

  /**
   * The feature id for the '<em><b>Keys</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_DEFINE__KEYS = 4;

  /**
   * The number of structural features of the '<em>Macro Define</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_DEFINE_FEATURE_COUNT = 5;

  /**
   * The meta object id for the '{@link org.wesnoth.wML.impl.WMLTextdomainImpl <em>Textdomain</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wML.impl.WMLTextdomainImpl
   * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLTextdomain()
   * @generated
   */
  int WML_TEXTDOMAIN = 6;

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
   * The meta object id for the '{@link org.wesnoth.wML.impl.WMLKeyImpl <em>Key</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wML.impl.WMLKeyImpl
   * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLKey()
   * @generated
   */
  int WML_KEY = 7;

  /**
   * The feature id for the '<em><b>Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_KEY__NAME = 0;

  /**
   * The feature id for the '<em><b>Value</b></em>' containment reference.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_KEY__VALUE = 1;

  /**
   * The feature id for the '<em><b>Extra Args</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_KEY__EXTRA_ARGS = 2;

  /**
   * The number of structural features of the '<em>Key</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_KEY_FEATURE_COUNT = 3;

  /**
   * The meta object id for the '{@link org.wesnoth.wML.impl.WMLKeyExtraArgsImpl <em>Key Extra Args</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wML.impl.WMLKeyExtraArgsImpl
   * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLKeyExtraArgs()
   * @generated
   */
  int WML_KEY_EXTRA_ARGS = 8;

  /**
   * The number of structural features of the '<em>Key Extra Args</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_KEY_EXTRA_ARGS_FEATURE_COUNT = 0;

  /**
   * The meta object id for the '{@link org.wesnoth.wML.impl.WMLKeyValueImpl <em>Key Value</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wML.impl.WMLKeyValueImpl
   * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLKeyValue()
   * @generated
   */
  int WML_KEY_VALUE = 9;

  /**
   * The number of structural features of the '<em>Key Value</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_KEY_VALUE_FEATURE_COUNT = 0;


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
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.WMLRoot#getMacros <em>Macros</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Macros</em>'.
   * @see org.wesnoth.wML.WMLRoot#getMacros()
   * @see #getWMLRoot()
   * @generated
   */
  EReference getWMLRoot_Macros();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.WMLRoot#getMacrosDefines <em>Macros Defines</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Macros Defines</em>'.
   * @see org.wesnoth.wML.WMLRoot#getMacrosDefines()
   * @see #getWMLRoot()
   * @generated
   */
  EReference getWMLRoot_MacrosDefines();

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
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.WMLTag#getMacros <em>Macros</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Macros</em>'.
   * @see org.wesnoth.wML.WMLTag#getMacros()
   * @see #getWMLTag()
   * @generated
   */
  EReference getWMLTag_Macros();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.WMLTag#getMacrosDefines <em>Macros Defines</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Macros Defines</em>'.
   * @see org.wesnoth.wML.WMLTag#getMacrosDefines()
   * @see #getWMLTag()
   * @generated
   */
  EReference getWMLTag_MacrosDefines();

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
   * Returns the meta object for class '{@link org.wesnoth.wML.WMLAbstractMacroCall <em>Abstract Macro Call</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>Abstract Macro Call</em>'.
   * @see org.wesnoth.wML.WMLAbstractMacroCall
   * @generated
   */
  EClass getWMLAbstractMacroCall();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wML.WMLAbstractMacroCall#getName <em>Name</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Name</em>'.
   * @see org.wesnoth.wML.WMLAbstractMacroCall#getName()
   * @see #getWMLAbstractMacroCall()
   * @generated
   */
  EAttribute getWMLAbstractMacroCall_Name();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wML.WMLMacroInclude <em>Macro Include</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>Macro Include</em>'.
   * @see org.wesnoth.wML.WMLMacroInclude
   * @generated
   */
  EClass getWMLMacroInclude();

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
   * Returns the meta object for the attribute list '{@link org.wesnoth.wML.WMLMacroCall#getArgs <em>Args</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute list '<em>Args</em>'.
   * @see org.wesnoth.wML.WMLMacroCall#getArgs()
   * @see #getWMLMacroCall()
   * @generated
   */
  EAttribute getWMLMacroCall_Args();

  /**
   * Returns the meta object for the attribute list '{@link org.wesnoth.wML.WMLMacroCall#getParams <em>Params</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute list '<em>Params</em>'.
   * @see org.wesnoth.wML.WMLMacroCall#getParams()
   * @see #getWMLMacroCall()
   * @generated
   */
  EAttribute getWMLMacroCall_Params();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.WMLMacroCall#getTags <em>Tags</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Tags</em>'.
   * @see org.wesnoth.wML.WMLMacroCall#getTags()
   * @see #getWMLMacroCall()
   * @generated
   */
  EReference getWMLMacroCall_Tags();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.WMLMacroCall#getMacros <em>Macros</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Macros</em>'.
   * @see org.wesnoth.wML.WMLMacroCall#getMacros()
   * @see #getWMLMacroCall()
   * @generated
   */
  EReference getWMLMacroCall_Macros();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.WMLMacroCall#getMacrosDefines <em>Macros Defines</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Macros Defines</em>'.
   * @see org.wesnoth.wML.WMLMacroCall#getMacrosDefines()
   * @see #getWMLMacroCall()
   * @generated
   */
  EReference getWMLMacroCall_MacrosDefines();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.WMLMacroCall#getKeys <em>Keys</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Keys</em>'.
   * @see org.wesnoth.wML.WMLMacroCall#getKeys()
   * @see #getWMLMacroCall()
   * @generated
   */
  EReference getWMLMacroCall_Keys();

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
   * Returns the meta object for the attribute list '{@link org.wesnoth.wML.WMLMacroDefine#getParams <em>Params</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute list '<em>Params</em>'.
   * @see org.wesnoth.wML.WMLMacroDefine#getParams()
   * @see #getWMLMacroDefine()
   * @generated
   */
  EAttribute getWMLMacroDefine_Params();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.WMLMacroDefine#getTags <em>Tags</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Tags</em>'.
   * @see org.wesnoth.wML.WMLMacroDefine#getTags()
   * @see #getWMLMacroDefine()
   * @generated
   */
  EReference getWMLMacroDefine_Tags();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.WMLMacroDefine#getMacros <em>Macros</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Macros</em>'.
   * @see org.wesnoth.wML.WMLMacroDefine#getMacros()
   * @see #getWMLMacroDefine()
   * @generated
   */
  EReference getWMLMacroDefine_Macros();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.WMLMacroDefine#getMacrosDefines <em>Macros Defines</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Macros Defines</em>'.
   * @see org.wesnoth.wML.WMLMacroDefine#getMacrosDefines()
   * @see #getWMLMacroDefine()
   * @generated
   */
  EReference getWMLMacroDefine_MacrosDefines();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.WMLMacroDefine#getKeys <em>Keys</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Keys</em>'.
   * @see org.wesnoth.wML.WMLMacroDefine#getKeys()
   * @see #getWMLMacroDefine()
   * @generated
   */
  EReference getWMLMacroDefine_Keys();

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
   * Returns the meta object for the containment reference '{@link org.wesnoth.wML.WMLKey#getValue <em>Value</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference '<em>Value</em>'.
   * @see org.wesnoth.wML.WMLKey#getValue()
   * @see #getWMLKey()
   * @generated
   */
  EReference getWMLKey_Value();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wML.WMLKey#getExtraArgs <em>Extra Args</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Extra Args</em>'.
   * @see org.wesnoth.wML.WMLKey#getExtraArgs()
   * @see #getWMLKey()
   * @generated
   */
  EReference getWMLKey_ExtraArgs();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wML.WMLKeyExtraArgs <em>Key Extra Args</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>Key Extra Args</em>'.
   * @see org.wesnoth.wML.WMLKeyExtraArgs
   * @generated
   */
  EClass getWMLKeyExtraArgs();

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
     * The meta object literal for the '<em><b>Macros</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_ROOT__MACROS = eINSTANCE.getWMLRoot_Macros();

    /**
     * The meta object literal for the '<em><b>Macros Defines</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_ROOT__MACROS_DEFINES = eINSTANCE.getWMLRoot_MacrosDefines();

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
     * The meta object literal for the '<em><b>Macros</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_TAG__MACROS = eINSTANCE.getWMLTag_Macros();

    /**
     * The meta object literal for the '<em><b>Macros Defines</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_TAG__MACROS_DEFINES = eINSTANCE.getWMLTag_MacrosDefines();

    /**
     * The meta object literal for the '<em><b>Keys</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_TAG__KEYS = eINSTANCE.getWMLTag_Keys();

    /**
     * The meta object literal for the '<em><b>End Name</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_TAG__END_NAME = eINSTANCE.getWMLTag_EndName();

    /**
     * The meta object literal for the '{@link org.wesnoth.wML.impl.WMLAbstractMacroCallImpl <em>Abstract Macro Call</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wML.impl.WMLAbstractMacroCallImpl
     * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLAbstractMacroCall()
     * @generated
     */
    EClass WML_ABSTRACT_MACRO_CALL = eINSTANCE.getWMLAbstractMacroCall();

    /**
     * The meta object literal for the '<em><b>Name</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_ABSTRACT_MACRO_CALL__NAME = eINSTANCE.getWMLAbstractMacroCall_Name();

    /**
     * The meta object literal for the '{@link org.wesnoth.wML.impl.WMLMacroIncludeImpl <em>Macro Include</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wML.impl.WMLMacroIncludeImpl
     * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLMacroInclude()
     * @generated
     */
    EClass WML_MACRO_INCLUDE = eINSTANCE.getWMLMacroInclude();

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
     * The meta object literal for the '<em><b>Args</b></em>' attribute list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_MACRO_CALL__ARGS = eINSTANCE.getWMLMacroCall_Args();

    /**
     * The meta object literal for the '<em><b>Params</b></em>' attribute list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_MACRO_CALL__PARAMS = eINSTANCE.getWMLMacroCall_Params();

    /**
     * The meta object literal for the '<em><b>Tags</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_MACRO_CALL__TAGS = eINSTANCE.getWMLMacroCall_Tags();

    /**
     * The meta object literal for the '<em><b>Macros</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_MACRO_CALL__MACROS = eINSTANCE.getWMLMacroCall_Macros();

    /**
     * The meta object literal for the '<em><b>Macros Defines</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_MACRO_CALL__MACROS_DEFINES = eINSTANCE.getWMLMacroCall_MacrosDefines();

    /**
     * The meta object literal for the '<em><b>Keys</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_MACRO_CALL__KEYS = eINSTANCE.getWMLMacroCall_Keys();

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
     * The meta object literal for the '<em><b>Params</b></em>' attribute list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_MACRO_DEFINE__PARAMS = eINSTANCE.getWMLMacroDefine_Params();

    /**
     * The meta object literal for the '<em><b>Tags</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_MACRO_DEFINE__TAGS = eINSTANCE.getWMLMacroDefine_Tags();

    /**
     * The meta object literal for the '<em><b>Macros</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_MACRO_DEFINE__MACROS = eINSTANCE.getWMLMacroDefine_Macros();

    /**
     * The meta object literal for the '<em><b>Macros Defines</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_MACRO_DEFINE__MACROS_DEFINES = eINSTANCE.getWMLMacroDefine_MacrosDefines();

    /**
     * The meta object literal for the '<em><b>Keys</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_MACRO_DEFINE__KEYS = eINSTANCE.getWMLMacroDefine_Keys();

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
     * The meta object literal for the '<em><b>Value</b></em>' containment reference feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_KEY__VALUE = eINSTANCE.getWMLKey_Value();

    /**
     * The meta object literal for the '<em><b>Extra Args</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_KEY__EXTRA_ARGS = eINSTANCE.getWMLKey_ExtraArgs();

    /**
     * The meta object literal for the '{@link org.wesnoth.wML.impl.WMLKeyExtraArgsImpl <em>Key Extra Args</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wML.impl.WMLKeyExtraArgsImpl
     * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLKeyExtraArgs()
     * @generated
     */
    EClass WML_KEY_EXTRA_ARGS = eINSTANCE.getWMLKeyExtraArgs();

    /**
     * The meta object literal for the '{@link org.wesnoth.wML.impl.WMLKeyValueImpl <em>Key Value</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wML.impl.WMLKeyValueImpl
     * @see org.wesnoth.wML.impl.WMLPackageImpl#getWMLKeyValue()
     * @generated
     */
    EClass WML_KEY_VALUE = eINSTANCE.getWMLKeyValue();

  }

} //WMLPackage
