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
   * The feature id for the '<em><b>Expressions</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_ROOT__EXPRESSIONS = 0;

  /**
   * The number of structural features of the '<em>WML Root</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_ROOT_FEATURE_COUNT = 1;

  /**
   * The meta object id for the '{@link org.wesnoth.wml.impl.WMLValuedExpressionImpl <em>WML Valued Expression</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wml.impl.WMLValuedExpressionImpl
   * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLValuedExpression()
   * @generated
   */
  int WML_VALUED_EXPRESSION = 10;

  /**
   * The number of structural features of the '<em>WML Valued Expression</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_VALUED_EXPRESSION_FEATURE_COUNT = 0;

  /**
   * The meta object id for the '{@link org.wesnoth.wml.impl.WMLExpressionImpl <em>WML Expression</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wml.impl.WMLExpressionImpl
   * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLExpression()
   * @generated
   */
  int WML_EXPRESSION = 9;

  /**
   * The feature id for the '<em><b>Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_EXPRESSION__NAME = WML_VALUED_EXPRESSION_FEATURE_COUNT + 0;

  /**
   * The number of structural features of the '<em>WML Expression</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_EXPRESSION_FEATURE_COUNT = WML_VALUED_EXPRESSION_FEATURE_COUNT + 1;

  /**
   * The meta object id for the '{@link org.wesnoth.wml.impl.WMLRootExpressionImpl <em>WML Root Expression</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wml.impl.WMLRootExpressionImpl
   * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLRootExpression()
   * @generated
   */
  int WML_ROOT_EXPRESSION = 8;

  /**
   * The feature id for the '<em><b>Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_ROOT_EXPRESSION__NAME = WML_EXPRESSION__NAME;

  /**
   * The number of structural features of the '<em>WML Root Expression</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_ROOT_EXPRESSION_FEATURE_COUNT = WML_EXPRESSION_FEATURE_COUNT + 0;

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
   * The feature id for the '<em><b>Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG__NAME = WML_ROOT_EXPRESSION__NAME;

  /**
   * The feature id for the '<em><b>Plus</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG__PLUS = WML_ROOT_EXPRESSION_FEATURE_COUNT + 0;

  /**
   * The feature id for the '<em><b>Expressions</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG__EXPRESSIONS = WML_ROOT_EXPRESSION_FEATURE_COUNT + 1;

  /**
   * The feature id for the '<em><b>End Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG__END_NAME = WML_ROOT_EXPRESSION_FEATURE_COUNT + 2;

  /**
   * The number of structural features of the '<em>WML Tag</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TAG_FEATURE_COUNT = WML_ROOT_EXPRESSION_FEATURE_COUNT + 3;

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
  int WML_KEY__NAME = WML_EXPRESSION__NAME;

  /**
   * The feature id for the '<em><b>Value</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_KEY__VALUE = WML_EXPRESSION_FEATURE_COUNT + 0;

  /**
   * The feature id for the '<em><b>Eol</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_KEY__EOL = WML_EXPRESSION_FEATURE_COUNT + 1;

  /**
   * The number of structural features of the '<em>WML Key</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_KEY_FEATURE_COUNT = WML_EXPRESSION_FEATURE_COUNT + 2;

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
   * The feature id for the '<em><b>Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_CALL__NAME = WML_KEY_VALUE_FEATURE_COUNT + 0;

  /**
   * The feature id for the '<em><b>Point</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_CALL__POINT = WML_KEY_VALUE_FEATURE_COUNT + 1;

  /**
   * The feature id for the '<em><b>Relative</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_CALL__RELATIVE = WML_KEY_VALUE_FEATURE_COUNT + 2;

  /**
   * The feature id for the '<em><b>Params</b></em>' attribute list.
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
   * The meta object id for the '{@link org.wesnoth.wml.impl.WMLArrayCallImpl <em>WML Array Call</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wml.impl.WMLArrayCallImpl
   * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLArrayCall()
   * @generated
   */
  int WML_ARRAY_CALL = 5;

  /**
   * The feature id for the '<em><b>Value</b></em>' attribute list.
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
  int WML_MACRO_DEFINE = 6;

  /**
   * The feature id for the '<em><b>Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_DEFINE__NAME = WML_ROOT_EXPRESSION__NAME;

  /**
   * The feature id for the '<em><b>Expressions</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_DEFINE__EXPRESSIONS = WML_ROOT_EXPRESSION_FEATURE_COUNT + 0;

  /**
   * The feature id for the '<em><b>End Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_DEFINE__END_NAME = WML_ROOT_EXPRESSION_FEATURE_COUNT + 1;

  /**
   * The number of structural features of the '<em>WML Macro Define</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_MACRO_DEFINE_FEATURE_COUNT = WML_ROOT_EXPRESSION_FEATURE_COUNT + 2;

  /**
   * The meta object id for the '{@link org.wesnoth.wml.impl.WMLPreprocIFImpl <em>WML Preproc IF</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wml.impl.WMLPreprocIFImpl
   * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLPreprocIF()
   * @generated
   */
  int WML_PREPROC_IF = 7;

  /**
   * The feature id for the '<em><b>Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_PREPROC_IF__NAME = WML_ROOT_EXPRESSION__NAME;

  /**
   * The feature id for the '<em><b>Expressions</b></em>' containment reference list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_PREPROC_IF__EXPRESSIONS = WML_ROOT_EXPRESSION_FEATURE_COUNT + 0;

  /**
   * The feature id for the '<em><b>Elses</b></em>' attribute list.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_PREPROC_IF__ELSES = WML_ROOT_EXPRESSION_FEATURE_COUNT + 1;

  /**
   * The feature id for the '<em><b>End Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_PREPROC_IF__END_NAME = WML_ROOT_EXPRESSION_FEATURE_COUNT + 2;

  /**
   * The number of structural features of the '<em>WML Preproc IF</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_PREPROC_IF_FEATURE_COUNT = WML_ROOT_EXPRESSION_FEATURE_COUNT + 3;

  /**
   * The meta object id for the '{@link org.wesnoth.wml.impl.WMLTextdomainImpl <em>WML Textdomain</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wml.impl.WMLTextdomainImpl
   * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLTextdomain()
   * @generated
   */
  int WML_TEXTDOMAIN = 11;

  /**
   * The feature id for the '<em><b>Name</b></em>' attribute.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TEXTDOMAIN__NAME = WML_ROOT_EXPRESSION__NAME;

  /**
   * The number of structural features of the '<em>WML Textdomain</em>' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @generated
   * @ordered
   */
  int WML_TEXTDOMAIN_FEATURE_COUNT = WML_ROOT_EXPRESSION_FEATURE_COUNT + 0;

  /**
   * The meta object id for the '{@link org.wesnoth.wml.impl.WMLLuaCodeImpl <em>WML Lua Code</em>}' class.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @see org.wesnoth.wml.impl.WMLLuaCodeImpl
   * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLLuaCode()
   * @generated
   */
  int WML_LUA_CODE = 12;

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
   * Returns the meta object for class '{@link org.wesnoth.wml.WMLRoot <em>WML Root</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>WML Root</em>'.
   * @see org.wesnoth.wml.WMLRoot
   * @generated
   */
  EClass getWMLRoot();

  /**
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wml.WMLRoot#getExpressions <em>Expressions</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Expressions</em>'.
   * @see org.wesnoth.wml.WMLRoot#getExpressions()
   * @see #getWMLRoot()
   * @generated
   */
  EReference getWMLRoot_Expressions();

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
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wml.WMLTag#getExpressions <em>Expressions</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Expressions</em>'.
   * @see org.wesnoth.wml.WMLTag#getExpressions()
   * @see #getWMLTag()
   * @generated
   */
  EReference getWMLTag_Expressions();

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
   * Returns the meta object for the attribute list '{@link org.wesnoth.wml.WMLMacroCall#getParams <em>Params</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute list '<em>Params</em>'.
   * @see org.wesnoth.wml.WMLMacroCall#getParams()
   * @see #getWMLMacroCall()
   * @generated
   */
  EAttribute getWMLMacroCall_Params();

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
   * Returns the meta object for class '{@link org.wesnoth.wml.WMLArrayCall <em>WML Array Call</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>WML Array Call</em>'.
   * @see org.wesnoth.wml.WMLArrayCall
   * @generated
   */
  EClass getWMLArrayCall();

  /**
   * Returns the meta object for the attribute list '{@link org.wesnoth.wml.WMLArrayCall#getValue <em>Value</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute list '<em>Value</em>'.
   * @see org.wesnoth.wml.WMLArrayCall#getValue()
   * @see #getWMLArrayCall()
   * @generated
   */
  EAttribute getWMLArrayCall_Value();

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
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wml.WMLMacroDefine#getExpressions <em>Expressions</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Expressions</em>'.
   * @see org.wesnoth.wml.WMLMacroDefine#getExpressions()
   * @see #getWMLMacroDefine()
   * @generated
   */
  EReference getWMLMacroDefine_Expressions();

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
   * Returns the meta object for the containment reference list '{@link org.wesnoth.wml.WMLPreprocIF#getExpressions <em>Expressions</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the containment reference list '<em>Expressions</em>'.
   * @see org.wesnoth.wml.WMLPreprocIF#getExpressions()
   * @see #getWMLPreprocIF()
   * @generated
   */
  EReference getWMLPreprocIF_Expressions();

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
   * Returns the meta object for class '{@link org.wesnoth.wml.WMLRootExpression <em>WML Root Expression</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>WML Root Expression</em>'.
   * @see org.wesnoth.wml.WMLRootExpression
   * @generated
   */
  EClass getWMLRootExpression();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wml.WMLExpression <em>WML Expression</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>WML Expression</em>'.
   * @see org.wesnoth.wml.WMLExpression
   * @generated
   */
  EClass getWMLExpression();

  /**
   * Returns the meta object for the attribute '{@link org.wesnoth.wml.WMLExpression#getName <em>Name</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for the attribute '<em>Name</em>'.
   * @see org.wesnoth.wml.WMLExpression#getName()
   * @see #getWMLExpression()
   * @generated
   */
  EAttribute getWMLExpression_Name();

  /**
   * Returns the meta object for class '{@link org.wesnoth.wml.WMLValuedExpression <em>WML Valued Expression</em>}'.
   * <!-- begin-user-doc -->
   * <!-- end-user-doc -->
   * @return the meta object for class '<em>WML Valued Expression</em>'.
   * @see org.wesnoth.wml.WMLValuedExpression
   * @generated
   */
  EClass getWMLValuedExpression();

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
     * The meta object literal for the '<em><b>Expressions</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_ROOT__EXPRESSIONS = eINSTANCE.getWMLRoot_Expressions();

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
     * The meta object literal for the '<em><b>Expressions</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_TAG__EXPRESSIONS = eINSTANCE.getWMLTag_Expressions();

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
     * The meta object literal for the '<em><b>Params</b></em>' attribute list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_MACRO_CALL__PARAMS = eINSTANCE.getWMLMacroCall_Params();

    /**
     * The meta object literal for the '<em><b>Extra Macros</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_MACRO_CALL__EXTRA_MACROS = eINSTANCE.getWMLMacroCall_ExtraMacros();

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
     * The meta object literal for the '<em><b>Value</b></em>' attribute list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_ARRAY_CALL__VALUE = eINSTANCE.getWMLArrayCall_Value();

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
     * The meta object literal for the '<em><b>Expressions</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_MACRO_DEFINE__EXPRESSIONS = eINSTANCE.getWMLMacroDefine_Expressions();

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
     * The meta object literal for the '<em><b>Expressions</b></em>' containment reference list feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EReference WML_PREPROC_IF__EXPRESSIONS = eINSTANCE.getWMLPreprocIF_Expressions();

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
     * The meta object literal for the '{@link org.wesnoth.wml.impl.WMLRootExpressionImpl <em>WML Root Expression</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wml.impl.WMLRootExpressionImpl
     * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLRootExpression()
     * @generated
     */
    EClass WML_ROOT_EXPRESSION = eINSTANCE.getWMLRootExpression();

    /**
     * The meta object literal for the '{@link org.wesnoth.wml.impl.WMLExpressionImpl <em>WML Expression</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wml.impl.WMLExpressionImpl
     * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLExpression()
     * @generated
     */
    EClass WML_EXPRESSION = eINSTANCE.getWMLExpression();

    /**
     * The meta object literal for the '<em><b>Name</b></em>' attribute feature.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    EAttribute WML_EXPRESSION__NAME = eINSTANCE.getWMLExpression_Name();

    /**
     * The meta object literal for the '{@link org.wesnoth.wml.impl.WMLValuedExpressionImpl <em>WML Valued Expression</em>}' class.
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @see org.wesnoth.wml.impl.WMLValuedExpressionImpl
     * @see org.wesnoth.wml.impl.WmlPackageImpl#getWMLValuedExpression()
     * @generated
     */
    EClass WML_VALUED_EXPRESSION = eINSTANCE.getWMLValuedExpression();

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

  }

} //WmlPackage
